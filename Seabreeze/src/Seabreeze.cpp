#include "Seabreeze/Seabreeze.h"
#include "hurricane/Net.h"
#include "hurricane/Segment.h"

namespace Seabreeze {

  using namespace std;
  using Hurricane::DBo;
  using Hurricane::Net;
  using Hurricane::Cell;
  using Hurricane::Instance;
  using Hurricane::PrivateProperty;
  using Hurricane::Component;
  using Hurricane::Segment;

//---------------------------------------------------------
// Class : "Elmore"

  Elmore::Elmore ( Net* net )
    : _conts()
    , checker()
    , _tree(new Tree())
  {}

  Elmore::~Elmore ()
  {
    delete _tree;
  }

  void Elmore::contFromNet ( Net* net ) {
    for ( RoutingPad* rp : net->getRoutingPads() ) {

      for ( Component* comp : rp->getSlaveComponents() ) {

/*        Segment* seg = dynamic_cast<Segment*>(comp);
        if ( not seg ) continue;
      
        for( Component* c : seg->getAnchors() ){
          Contact* ct = dynamic_cast<Contact*>(c);
          if( not ct )  continue;
          
          _conts.insert(ct);
        }
        Contact* ct = dynamic_cast<Contact*>(seg->getOppositeAnchor(s->_contact));*/
        
        Contact* ct = dynamic_cast<Contact*>(comp);
        if ( not ct )   continue;

        _conts.insert(ct);
      }
    }
  }

  void Elmore::buildTree ( RoutingPad* rp )
  {
    if ( rp == nullptr ) {
      cerr << "Input RoutingPad is NULL. Please select a RoutingPad !" << endl;
      return;
    }
    Contact* ct = nullptr;
    for ( Component* c : rp->getSlaveComponents() ) {
      Contact* cont = dynamic_cast<Contact*>(c);
        
      if ( cont ) {//&& find(_conts.begin(), _conts.end(), cont) != _conts.end() ) {
        ct = cont;
        break;
      }
    }

    if ( ct == nullptr ) {
      cerr << "No contact found" << endl;
      return;
    }
    cerr << "Root contact : " << ct << endl;
    cerr << "Start building tree..." << endl << endl;;
    Node* s = new Node(nullptr, ct);
    build_from_node(s);
    cerr << endl << "Finished building tree !" << endl;
    _tree->print(cerr);
  }

  void Elmore::build_from_node ( Node* s  )
  {
//-----------------------------------------------------------------------
    cerr << "Elmore::build_from_node" << endl;
    cerr << "From contact : " << s->_contact << endl;
//-----------------------------------------------------------------------
    if ( s->_contact == nullptr ) {
      cerr << "No contact found" << s->_contact << endl;
      return;
    }

    _tree->add_node(s);
  
    // To check for circle
    checker.insert(s->_contact);

    for ( Component* comp : (s->_contact)->getSlaveComponents() ) {

      Segment* seg = dynamic_cast<Segment*>(comp);

      if ( not seg ) continue;

//-----------------------------------------
      cerr << "Segment : " << seg << endl;
//-----------------------------------------

      Contact* ccont = dynamic_cast<Contact*>(seg->getOppositeAnchor(s->_contact));

//----------------------------------------------------------------
      cerr << "Target contc : " << ccont << endl;
//---------------------------------------------------------------- 

      if ( not ccont || (s->Np && ccont == (s->Np)->_contact) )
        continue;
      else{               
        if ( find( checker.begin(), checker.end(), ccont) != checker.end() ) {
          cerr << "Net contains a circle. Cannot apply Elmore's delay here !" << endl;
          return;
        }
        else {
//-----------------------------------------------------------------------
          cerr << "Contact found in the net" << endl;
//-----------------------------------------------------------------------
          if ( not s->Np || ccont != (s->Np)->_contact ) {
            Node* node = new Node( s, ccont );
//-----------------------------------------------------------------------
            cerr << "Parent node : " << node->Np->_contact << endl;
            cerr << endl;
            cerr << endl;
//-----------------------------------------------------------------------
            build_from_node(node);
          }
        }
      } 
    }
  }

  void Elmore::clearTree ()
  {
    _tree->clear();
    checker.clear();
  }

  Tree* Elmore::getTree ()
  {
    return _tree;
  }

  int Elmore::delayElmore ( RoutingPad* rp )
  {
    return _tree->Delay_Elmore(rp);
  }
  
  void Elmore::toTREE ( ostream& out ) const
  {
    _tree->print( out );
  }

//---------------------------------------------------------
// Class : "ElmoreProperty"

  Name ElmoreProperty::_name = "Seabreeze::Elmore";
  
  ElmoreProperty* ElmoreProperty::create ( Net* net )
  {
    ElmoreProperty* property = new ElmoreProperty(net);
    //ElmoreProperty* property(net);
 
    property->_postCreate();
    return property;
  }

  Name ElmoreProperty::getName () const
  {
    return _name;
  }

  string ElmoreProperty::_getTypeName () const
  {
    return "ElmoreProperty";
  }

//---------------------------------------------------------
// Class : "ElmoreExtension"

  Elmore* ElmoreExtension::create ( Net* net )
  {
    return ElmoreProperty::create(net)->getElmore();
  }

  void ElmoreExtension::destroyAll ()
  {
    
  }

  void ElmoreExtension::destroy ( Net* net )
  {

  }
}
