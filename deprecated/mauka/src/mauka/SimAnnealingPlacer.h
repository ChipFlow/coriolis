
// This file is part of the Coriolis Project.
// Copyright (C) Laboratoire LIP6 - Departement ASIM
// Universite Pierre et Marie Curie
//
// Main contributors :
//        Christophe Alexandre   <Christophe.Alexandre@lip6.fr>
//        Sophie Belloeil             <Sophie.Belloeil@lip6.fr>
//        Hugo Cl�ment                   <Hugo.Clement@lip6.fr>
//        Jean-Paul Chaput           <Jean-Paul.Chaput@lip6.fr>
//        Damien Dupuis                 <Damien.Dupuis@lip6.fr>
//        Christian Masson           <Christian.Masson@lip6.fr>
//        Marek Sroka                     <Marek.Sroka@lip6.fr>
// 
// The Coriolis Project  is free software;  you can  redistribute it and/or
// modify  it  under the  terms  of  the  GNU  General  Public License  as
// published by  the Free  Software Foundation; either  version 2  of  the
// License, or (at your option) any later version.
// 
// The Coriolis Project  is distributed in the hope that it will be useful,
// but  WITHOUT  ANY  WARRANTY;  without  even  the  implied  warranty  of
// MERCHANTABILITY  or  FITNESS  FOR A  PARTICULAR PURPOSE.   See  the GNU
// General Public License for more details.
// 
// You should have received a copy  of  the  GNU  General  Public  License
// along with  the Coriolis Project;  if  not,  write to the  Free Software
// Foundation, inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
//
// License-Tag
//
// Date   : 19/07/2006
// Author : Christophe Alexandre  <Christophe.Alexandre@lip6.fr>
//
// Authors-Tag 
#ifndef __SIMANNEALINGPLACER_H
#define __SIMANNEALINGPLACER_H

#include "hurricane/Instance.h"
#include "crlcore/Utilities.h"
#include "mauka/MaukaEngine.h"

namespace Mauka {

class MaukaEngine;
class Bin;

class SimAnnealingPlacer {
// ***********************
    friend class MaukaEngine;
    friend class Move;
    friend class Bin;
    friend class SubRow;
    
// Types
// *****
   public: typedef std::vector<Bin*> InstanceBins;
    
// Attributes
// **********
    private: MaukaEngine*               _mauka;
    private: InstanceBins               _instanceBins;
    private: MaukaEngine::BBoxes        _netBBoxes;
    private: MaukaEngine::Costs         _netCosts;
    private: MaukaEngine::UVector       _netFlags;
    private: double                     _netCost;
    private: double                     _binCost;
    private: double                     _rowCost;
    private: double                     _initNetCost;
    private: double                     _initBinCost;
    private: double                     _initRowCost;
    private: double                     _netMult;
    private: double                     _binMult;
    private: double                     _rowMult;
    private: double                     _temperature;
    private: double                     _distance;
    private: unsigned                   _loop;
    private: unsigned                   _iterationsFactor;
    private: unsigned                   _iterations;
    private: unsigned                   _moves;
    private: unsigned                   _sourceEqualTargetMovementNumber;
    private: unsigned                   _surOccupationTargetMovementNumber;
    private: unsigned                   _impossibleExchangeMovementNumber;
             

// Constructor
// ***********
    public: SimAnnealingPlacer(MaukaEngine* mauka); 
            
// Accessors
// *********
    public: double getNetCost();
    public: double getCost() const;
    public: double& _getNetIdCost(unsigned netid) { return _netCosts[netid][_netFlags[netid]]; }
    public: double& _getNetIdTmpCost(unsigned netid) { return _netCosts[netid][!_netFlags[netid]]; }
    public: Box& _getNetIdBBox(unsigned netid) { return _netBBoxes[netid][_netFlags[netid]]; }
    public: Box& _getNetTmpBBox(unsigned netid) { return _netBBoxes[netid][!_netFlags[netid]]; }
    public: MaukaEngine* getMauka() { return _mauka; }
    public: unsigned getMoves() const { return _moves; }
    public: unsigned getRandInstance();

// Others
// ******
    public: bool Iterate();
    private: bool accept(double deltacost) const;
    public: double DebugNetCost();
    public: void DisplayResults() const;
    public: void _setInstanceIdBin(unsigned instanceid, Bin* bin) { _instanceBins[instanceid] = bin; }
    public: void _InvertNetIdFlag(unsigned netid) { _netFlags[netid] = !_netFlags[netid]; }
    public: double computeCost(double rowcost, double bincost, double netcost) const;
    public: void incrImpossibleExchangeMovementNumber() { ++_impossibleExchangeMovementNumber; }
    public: void incrSourceEqualTargetMovementNumber() { ++_sourceEqualTargetMovementNumber; }
    public: void incrSurOccupationTargetMovementNumber() { ++_surOccupationTargetMovementNumber; }
    public: void init();
    public: void Save() const;
    private: void Plot(std::ofstream& out) const;

};

}

#endif /* __SIMANNEALINGPLACER_H */
