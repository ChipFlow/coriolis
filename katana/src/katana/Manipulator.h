// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) UPMC 2008-2016, All Rights Reserved
//
// +-----------------------------------------------------------------+
// |                   C O R I O L I S                               |
// |      K i t e  -  D e t a i l e d   R o u t e r                  |
// |                                                                 |
// |  Author      :                    Jean-Paul CHAPUT              |
// |  E-mail      :       Jean-Paul.Chaput@asim.lip6.fr              |
// | =============================================================== |
// |  C++ Header  :   "./katana/Manipulator.h"                       |
// +-----------------------------------------------------------------+


#ifndef  KATANA_MANIPULATOR_H
#define  KATANA_MANIPULATOR_H

#include "hurricane/DbU.h"
#include "anabatic/Constants.h"


namespace Katana {

  using Hurricane::DbU;

  class TrackElement;
  class DataNegociate;
  class RoutingEvent;
  class SegmentFsm;


// -------------------------------------------------------------------
// Class Declaration  :  "::Manipulator".

  class Manipulator {
    public:
      enum FunctionFlag { ToRipupLimit        = (1 <<  0)
                        , AllowExpand         = (1 <<  1)
                        , NoExpand            = (1 <<  2)
                        , PerpandicularsFirst = (1 <<  3)
                        , ToMoveUp            = (1 <<  4)
                        , AllowLocalMoveUp    = (1 <<  5)
                        , AllowTerminalMoveUp = (1 <<  6)
                        , AllowShortPivotUp   = (1 <<  7)
                        , NoDoglegReuse       = (1 <<  8)
                        , LeftAxisHint        = (1 <<  9)
                        , RightAxisHint       = (1 << 10) 
                        , NotOnLastRipup      = (1 << 11) 
                        , IgnoreContacts      = (1 << 12) 
                        };
    public:
                                  Manipulator             ( TrackElement*, SegmentFsm& );
                                 ~Manipulator             ();
      inline TrackElement*        getSegment              () const;
      inline DataNegociate*       getData                 () const;
      inline RoutingEvent*        getEvent                () const;
      inline const Layer*         getLayer                () const;
      inline DbU::Unit            getPitch                () const;
      inline DbU::Unit            getPPitch               () const;
             bool                 canRipup                ( uint32_t flags=0 ) const;
             bool                 isCaged                 ( DbU::Unit ) const;
             bool                 ripup                   ( uint32_t type, DbU::Unit axisHint=0 );
             bool                 ripupPerpandiculars     ( uint32_t flags=0 );
             void                 repackPerpandiculars    ();
             void                 reprocessPerpandiculars ();
             bool                 ripple                  ();
             bool                 minimize                ();
             bool                 slacken                 ( Flags flags=Flags::NoFlags );
             bool                 pivotUp                 ();
             bool                 pivotDown               ();
             bool                 moveUp                  ( uint32_t flags=0 );
             bool                 makeDogleg              ();
             bool                 makeDogleg              ( DbU::Unit );
             bool                 makeDogleg              ( Interval );
             bool                 relax                   ( Interval, uint32_t flags=AllowExpand );
             bool                 insertInTrack           ( size_t );
             bool                 shrinkToTrack           ( size_t
                                                          , uint32_t   flags=0
                                                          , DbU::Unit  leftAxisHint=0
                                                          , DbU::Unit  rightAxisHint=0
                                                          );
             bool                 forceToTrack            ( size_t );
             bool                 forceOverLocals         ();
    private:
      TrackElement*  _segment;
      DataNegociate* _data;
      RoutingEvent*  _event;
      SegmentFsm&    _fsm;
  };


  inline TrackElement*  Manipulator::getSegment () const { return _segment; }
  inline DataNegociate* Manipulator::getData    () const { return _data; }
  inline RoutingEvent*  Manipulator::getEvent   () const { return _event; }
  inline const Layer*   Manipulator::getLayer   () const { return _segment->getLayer(); }
  inline DbU::Unit      Manipulator::getPitch   () const { return _segment->getPitch(); }
  inline DbU::Unit      Manipulator::getPPitch  () const { return _segment->getPPitch(); }


}  // Katana namespace.

#endif  // KATANA_MANIPULATOR_H
