// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) UPMC/LIP6 2012-2016, All Rights Reserved
//
// +-----------------------------------------------------------------+
// |                   C O R I O L I S                               |
// |         A n a b a t i c  -  Routing Toolbox                     |
// |                                                                 |
// |  Author      :                    Jean-Paul CHAPUT              |
// |  E-mail      :            Jean-Paul.Chaput@lip6.fr              |
// | =============================================================== |
// |  C++ Header  :  "./anabatic/AutoContactTerminal.h"              |
// +-----------------------------------------------------------------+


#ifndef  ANABATIC_AUTOCONTACT_TERMINAL_H
#define  ANABATIC_AUTOCONTACT_TERMINAL_H

#include  "anabatic/AutoContact.h"


namespace Anabatic {


// -------------------------------------------------------------------
// Class  :  "Anabatic::AutoContactTerminal".


  class AutoContactTerminal : public AutoContact {
      friend class AutoContact;
    public:
      static  AutoContactTerminal* create                 ( GCell*       gcell
                                                          , Component*   anchor
                                                          , const Layer* layer
                                                          , Point        point
                                                          , DbU::Unit    width
                                                          , DbU::Unit    height
                                                          );
      static  AutoContactTerminal* create                 (       GCell*     gcell
                                                          ,       Component* anchor
                                                          , const Layer*     layer
                                                          , const DbU::Unit  dx
                                                          , const DbU::Unit  dy
                                                          , const DbU::Unit  width
                                                          , const DbU::Unit  height
                                                          );
    protected:
    // Constructors & Destructors.
                                   AutoContactTerminal    ( GCell*, Contact* );
      virtual                     ~AutoContactTerminal    ();
      virtual void                 _invalidate            ( unsigned int flags );
    public:
      virtual Box                  getNativeConstraintBox () const;
      inline  AutoSegment*         getSegment             () const;
      virtual AutoSegment*         getSegment             ( unsigned int ) const;
      virtual AutoSegment*         getOpposite            ( const AutoSegment* ) const;
      virtual AutoSegment*         getPerpandicular       ( const AutoSegment* ) const;
      virtual void                 updateGeometry         ();
      virtual void                 updateTopology         ();
      virtual void                 cacheDetach            ( AutoSegment* );
      virtual void                 cacheAttach            ( AutoSegment* );
      virtual void                 updateCache            ();
      virtual string               _getTypeName           () const;
    private:                               
                                   AutoContactTerminal    ( const AutoContactTerminal& );
              AutoContactTerminal& operator=              ( const AutoContactTerminal& );
    protected:
      AutoSegment* _segment;
  };


  inline AutoSegment* AutoContactTerminal::getSegment () const { return _segment; }
  

} // Anabatic namespace.


INSPECTOR_P_SUPPORT(Anabatic::AutoContactTerminal);


#endif  // ANABATIC_AUTOCONTACT_TERMINAL_H
