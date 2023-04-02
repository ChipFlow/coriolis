// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) Sorbonne Université 2007-2023, All Rights Reserved
//
// +-----------------------------------------------------------------+ 
// |                   C O R I O L I S                               |
// |       T r a m o n t a n a  -  Extractor & LVX                   |
// |                                                                 |
// |  Algorithm    :                   Christian MASSON              |
// |  First impl.  :                           Yifei WU              |
// |  Second impl. :                   Jean-Paul CHAPUT              |
// |  E-mail       :           Jean-Paul.Chaput@lip6.fr              |
// | =============================================================== |
// |  C++ Module   :  "./Equipotential.cpp"                          |
// +-----------------------------------------------------------------+


#include <iomanip>
#include <set>
#include "hurricane/utilities/Path.h"
#include "hurricane/DebugSession.h"
#include "hurricane/UpdateSession.h"
#include "hurricane/Bug.h"
#include "hurricane/Error.h"
#include "hurricane/Warning.h"
#include "hurricane/Breakpoint.h"
#include "hurricane/Timer.h"
#include "hurricane/Layer.h"
#include "hurricane/Net.h"
#include "hurricane/Pad.h"
#include "hurricane/Plug.h"
#include "hurricane/Cell.h"
#include "hurricane/Instance.h"
#include "hurricane/Vertical.h"
#include "hurricane/Horizontal.h"
#include "hurricane/RoutingPad.h"
#include "crlcore/Utilities.h"
#include "tramontana/Equipotential.h"
#include "tramontana/EquipotentialRelation.h"
#include "tramontana/TramontanaEngine.h"


namespace {

  using Hurricane::Net;
  using Hurricane::Occurrence;


  class NetCompareByName {
    public:
      bool operator() ( const Net* lhs, const Net* rhs ) const;
  };


  bool NetCompareByName::operator() ( const Net* lhs, const Net* rhs ) const
  {
    if (lhs->isFused    () != rhs->isFused    ()) return rhs->isFused();
    if (lhs->isAutomatic() != rhs->isAutomatic()) return rhs->isAutomatic();
    if (lhs->isGlobal   () != rhs->isGlobal   ()) return rhs->isGlobal();

    if (lhs->getName().size() != rhs->getName().size())
       return lhs->getName().size() < rhs->getName().size();
    return lhs->getName() < rhs->getName();
  }


  class OccNetCompareByName {
    public:
      bool operator() ( const Occurrence& lhs, const Occurrence& rhs ) const;
  };


  bool OccNetCompareByName::operator() ( const Occurrence& lhs, const Occurrence& rhs ) const
  {
    static NetCompareByName compareByName;
    
    size_t lhsLength = lhs.getPath().getInstances().getSize();
    size_t rhsLength = rhs.getPath().getInstances().getSize();

    if (lhsLength != rhsLength) return lhsLength < rhsLength;
    return compareByName( static_cast<Net*>(lhs.getEntity()), static_cast<Net*>(rhs.getEntity()) );
  }


}  // Anonymous namespace.


namespace Tramontana {

  using std::cout;
  using std::cerr;
  using std::endl;
  using std::dec;
  using std::setw;
  using std::setfill;
  using std::left;
  using std::string;
  using std::ostream;
  using std::ofstream;
  using std::ostringstream;
  using std::setprecision;
  using std::vector;
  using std::set;
  using std::make_pair;
  using Hurricane::dbo_ptr;
  using Hurricane::UpdateSession;
  using Hurricane::DebugSession;
  using Hurricane::tab;
  using Hurricane::Bug;
  using Hurricane::Error;
  using Hurricane::Warning;
  using Hurricane::Breakpoint;
  using Hurricane::Box;
  using Hurricane::Layer;
  using Hurricane::Entity;
  using Hurricane::Net;
  using Hurricane::Horizontal;
  using Hurricane::Vertical;
  using Hurricane::RoutingPad;
  using Hurricane::Cell;
  using Hurricane::Instance;


// -------------------------------------------------------------------
// Class  :  "Tramontana::Equipotential".


  const char* defaultName = "Unamed (please call consolidate())";


  Equipotential::Equipotential ( Cell* owner )
    : _owner      (owner)
    , _boundingBox()
    , _components ()
    , _childs     ()
    , _name       (defaultName)
    , _type       (Net::Type::UNDEFINED)
    , _direction  (Net::Direction::DirUndefined)
    , _netCount   (0)
    , _isExternal (false)
    , _isGlobal   (false)
    , _isAutomatic(false)
    , _hasFused   (false)
  { }


  void  Equipotential::_postCreate ()
  {
    Super::_postCreate();
    TramontanaEngine* tramontana = TramontanaEngine::get( _owner );
    tramontana->add( this );
  }


  Equipotential* Equipotential::create ( Cell* owner )
  {
    Equipotential* equi = new Equipotential ( owner );
    equi->_postCreate();
    return equi;
  }


  void  Equipotential::_preDestroy ()
  {
    Super::_preDestroy();
  }


  Equipotential::~Equipotential ()
  { }


  Cell* Equipotential::getCell () const
  { return _owner; }


  Box  Equipotential::getBoundingBox () const
  { return _boundingBox; }

  
  void  Equipotential::add ( Occurrence component, const Box& boundingBox )
  {
    _components.insert( component );
    _boundingBox.merge( boundingBox );
  }

  
  void  Equipotential::merge ( Equipotential* other )
  {
    if (this == other) {
      cerr << Warning( "Equipotential::merge(): Attempt to merge itself (ignored).\n"
                       "        (on: %s)"
                     , getString(this).c_str()
                     ) << endl;
      return;
    }
    
    for ( const Occurrence& component : other->getComponents() ) add( component );
    for ( const Occurrence& child     : other->getChilds    () ) add( child );
    _boundingBox.merge( other->_boundingBox );
    other->clear();
  }

  
  void  Equipotential::consolidate ()
  {
    EquipotentialRelation*              relation = EquipotentialRelation::create( this );
    set<Net*,NetCompareByName>          nets;
    set<Occurrence,OccNetCompareByName> deepNets;
    for ( const Occurrence& occurrence : getComponents() ) {
      Component* component = dynamic_cast<Component*>( occurrence.getEntity() );
      if (not component) continue;
      if (not occurrence.getPath().isEmpty()) {
        deepNets.insert( Occurrence( component->getNet(), occurrence.getPath() ));
        continue;
      }
      component->put( relation );
      Net* net = component->getNet();
      if (net->isFused()) _hasFused = true;
      else {
        if (net->isExternal ()) _isExternal  = true;
        if (net->isGlobal   ()) _isGlobal    = true;
        if (net->isAutomatic()) _isAutomatic = true;
        _type       = net->getType();
        _direction |= net->getDirection();
      }
      nets.insert( component->getNet() );
    }
    if (not nets.empty()) {
      _name = getString( (*nets.begin())->getName() );
    } else {
      if (not deepNets.empty()) {
        _name = (*deepNets.begin()).getCompactString();
      }
    }
    _netCount = nets.size();

    // if (_name == "abc_11873_auto_rtlil_cc_2560_muxgate_11612")
    //   show();
  }

  
  void  Equipotential::clear ()
  {
    _components.clear();
    _childs    .clear();
  }


  void  Equipotential::show () const
  {
    cerr << this << endl;
    cerr << "+ Components:" << endl;
    for ( const Occurrence& component : _components ) {
      cerr << "| " << component << endl;
    }
    cerr << "+ Occurrences:" << endl;
    for ( Occurrence occ : _childs ) {
      cerr << "| " << occ << endl;
    }
  }


  string  Equipotential::getFlagsAsString () const
  {
    string sflags;
    sflags += ((_isExternal ) ? "e" : "-");
    sflags += ((_isGlobal   ) ? "g" : "-");
    sflags += ((_isAutomatic) ? "a" : "-");
    sflags += " [" + getString( _netCount - ((_hasFused) ? 1 : 0) );
    if (_hasFused)
      sflags += "+fused";
    sflags += "] ";
    return sflags;
  }


  string  Equipotential::_getTypeName () const
  { return "Tramontana::Equipotential"; }


  string  Equipotential::_getString () const
  {
    ostringstream  os;
    os << "<Equipotential id:" << getId() << " "
       << getFlagsAsString()
       << " " << getName()
       << " " << getType()
       << " " << getDirection()
       << ">";
    return os.str();
  }


  Record* Equipotential::_getRecord () const
  {
    Record* record = new Record ( _getString() );
    if (record) {
      record->add( getSlot( "_owner"      , &_owner       ) );
      record->add( getSlot( "_boundingBox", &_boundingBox ) );
      record->add( getSlot( "_components" , &_components  ) );
      record->add( getSlot( "_childs"     , &_childs      ) );
    //record->add( getSlot( "_name"       , &_name        ) );
    }
    return record;
  }


} // Tramontana namespace.
