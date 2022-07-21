// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) SU 2022-2022, All Rights Reserved
//
// +-----------------------------------------------------------------+
// |                   C O R I O L I S                               |
// |        S e a b r e e z e  -  Timing Analysis                    |
// |                                                                 |
// |  Author      :                   Vu Hoang Anh PHAM              |
// |  E-mail      :            Jean-Paul.Chaput@lip6.fr              |
// | =============================================================== |
// |  C++ Header  :  "./seabreeze/Delay.h"                            |
// +-----------------------------------------------------------------+


#pragma once
#include <map>
#include <iostream>
#include "hurricane/RoutingPad.h"

using namespace std;

namespace Seabreeze {

  using Hurricane::RoutingPad; 

//---------------------------------------------------------
// Class : Seabreeze::Delay

  class Delay {
    public:
                                                               Delay       ();
                                                              ~Delay       ();
      inline const map<RoutingPad*, map<RoutingPad*, double>>& getValues   () const ;
                   void                                        addPair     ( RoutingPad*, RoutingPad*, double );
                   void                                        addValue    ( RoutingPad*, RoutingPad*, double );
                   void                                        printDelays ();
      
    private:
      map<RoutingPad*, map<RoutingPad*, double>> _values;
  };
  
    inline const map<RoutingPad*, map<RoutingPad*, double>>& Delay::getValues () const { return _values; }
}
