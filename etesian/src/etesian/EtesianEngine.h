// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) SU 2014-2020, All Rights Reserved
//
// +-----------------------------------------------------------------+ 
// |                   C O R I O L I S                               |
// |   E t e s i a n  -  A n a l y t i c   P l a c e r               |
// |                                                                 |
// |  Author      :                    Jean-Paul CHAPUT              |
// |  E-mail      :            Jean-Paul.Chaput@lip6.fr              |
// | =============================================================== |
// |  C++ Header  :  "./etesian/EtesianEngine.h"                     |
// +-----------------------------------------------------------------+


#pragma once
#include <iostream>
#include <unordered_map>
#include "coloquinte/circuit.hxx"

#include "hurricane/Timer.h"
#include "hurricane/Name.h"
namespace Hurricane {
  class Layer;
  class Net;
  class Cell;
  class CellWidget;
  class CellViewer;
  class Instance;
}

#include "crlcore/ToolEngine.h"
#include "etesian/Configuration.h"
#include "etesian/FeedCells.h"
#include "etesian/BloatCells.h"


namespace Etesian {

  using Hurricane::Timer;
  using Hurricane::Name;
  using Hurricane::Layer;
  using Hurricane::Net;
  using Hurricane::Cell;
  using Hurricane::Record;
  using Hurricane::Instance;


// -------------------------------------------------------------------
// Class  :  "Etesian::EtesianEngine".

  class EtesianEngine : public CRL::ToolEngine {
    public:
      typedef ToolEngine  Super;
    public:
      static  const Name&            staticGetName       ();
      static  EtesianEngine*         create              ( Cell* );
      static  EtesianEngine*         get                 ( const Cell* );
    public:                                              
      virtual Configuration*         getConfiguration    ();
      virtual const Configuration*   getConfiguration    () const;
      virtual const Name&            getName             () const;
      inline  RoutingGauge*          getGauge            () const;
      inline  CellGauge*             getCellGauge        () const;
      inline  DbU::Unit              getHorizontalPitch  () const;
      inline  DbU::Unit              getVerticalPitch    () const;
      inline  DbU::Unit              getSliceHeight      () const;
      inline  DbU::Unit              getSliceStep        () const;
      inline  DbU::Unit              getFixedAbHeight    () const;
      inline  DbU::Unit              getFixedAbWidth     () const;
      inline  Effort                 getPlaceEffort      () const;
      inline  GraphicUpdate          getUpdateConf       () const;
      inline  Density                getSpreadingConf    () const;
      inline  double                 getSpaceMargin      () const;
      inline  double                 getAspectRatio      () const;
      inline  const FeedCells&       getFeedCells        () const;
      inline  Hurricane::CellViewer* getViewer           () const;
      inline  void                   setViewer           ( Hurricane::CellViewer* );
      inline  Cell*                  getBlockCell        () const;
      inline  Instance*              getBlockInstance    () const;
      inline  void                   setBlock            ( Instance* );
      inline  void                   setFixedAbHeight    ( DbU::Unit );
      inline  void                   setFixedAbWidth     ( DbU::Unit );
      inline  void                   setSpaceMargin      ( double );
      inline  void                   setAspectRatio      ( double );
              void                   setDefaultAb        ();
              void                   adjustSliceHeight   ();
              void                   resetPlacement      ();
              void                   loadLeafCellLayouts ();
              size_t                 toColoquinte        ();
              void                   preplace            ();
              void                   roughLegalize       ( float minDisruption, unsigned options );
              void                   globalPlace         ( float initPenalty, float minDisruption, float targetImprovement, float minInc, float maxInc, unsigned options=0 );
              void                   detailedPlace       ( int iterations, int effort, unsigned options=0 );
              void                   place               ();
      inline  void                   useFeed             ( Cell* );
              size_t                 findYSpin           ();
              void                   addFeeds            ();
              void                   readSlices          ();
      inline  void                   selectBloat         ( std::string );
      virtual Record*                _getRecord          () const;
      virtual std::string            _getString          () const;
      virtual std::string            _getTypeName        () const;
    private:
    // Attributes.
      static Name                                     _toolName;
    protected:
             Configuration*                           _configuration;
             Instance*                                _block;
             bool                                     _placed;
             bool                                     _ySpinSet;
             bool                                     _flatDesign;
             coloquinte::box<coloquinte::int_t>       _surface;
             coloquinte::netlist                      _circuit;
             coloquinte::placement_t                  _placementLB;
             coloquinte::placement_t                  _placementUB;
             coloquinte::density_restrictions         _densityLimits;
             std::unordered_map<string,unsigned int>  _cellsToIds;
             std::vector<Instance*>                   _idsToInsts;
             Hurricane::CellViewer*                   _viewer;
             FeedCells                                _feedCells;
             BloatCells                               _bloatCells;
             size_t                                   _yspinSlice0;
             DbU::Unit                                _sliceHeight;
             DbU::Unit                                _fixedAbHeight;
             DbU::Unit                                _fixedAbWidth;

    protected:
    // Constructors & Destructors.
                             EtesianEngine    ( Cell* );
      virtual               ~EtesianEngine    ();
      virtual void           _postCreate      ();
      virtual void           _preDestroy      ();
    private:                                  
                             EtesianEngine    ( const EtesianEngine& );
              EtesianEngine& operator=        ( const EtesianEngine& );
    private:
              void           _updatePlacement ( const coloquinte::placement_t& );
              void           _progressReport1 ( string label ) const;
              void           _progressReport2 ( string label ) const;
  };


// Inline Functions.
  inline  void                   EtesianEngine::setViewer          ( Hurricane::CellViewer* viewer ) { _viewer = viewer; }
  inline  Hurricane::CellViewer* EtesianEngine::getViewer          () const { return _viewer; }
  inline  RoutingGauge*          EtesianEngine::getGauge           () const { return getConfiguration()->getGauge(); }
  inline  CellGauge*             EtesianEngine::getCellGauge       () const { return getConfiguration()->getCellGauge(); }
  inline  DbU::Unit              EtesianEngine::getHorizontalPitch () const { return getGauge()->getHorizontalPitch(); }
  inline  DbU::Unit              EtesianEngine::getVerticalPitch   () const { return getGauge()->getVerticalPitch(); }
  inline  DbU::Unit              EtesianEngine::getSliceHeight     () const { return _sliceHeight; }
  inline  DbU::Unit              EtesianEngine::getSliceStep       () const { return getCellGauge()->getSliceStep(); }
  inline  DbU::Unit              EtesianEngine::getFixedAbHeight   () const { return _fixedAbHeight; }
  inline  DbU::Unit              EtesianEngine::getFixedAbWidth    () const { return _fixedAbWidth; }
  inline  Effort                 EtesianEngine::getPlaceEffort     () const { return getConfiguration()->getPlaceEffort(); }
  inline  GraphicUpdate          EtesianEngine::getUpdateConf      () const { return getConfiguration()->getUpdateConf(); }
  inline  Density                EtesianEngine::getSpreadingConf   () const { return getConfiguration()->getSpreadingConf(); }
  inline  double                 EtesianEngine::getSpaceMargin     () const { return getConfiguration()->getSpaceMargin(); }
  inline  double                 EtesianEngine::getAspectRatio     () const { return getConfiguration()->getAspectRatio(); }
  inline  void                   EtesianEngine::useFeed            ( Cell* cell ) { _feedCells.useFeed(cell); }
  inline  const FeedCells&       EtesianEngine::getFeedCells       () const { return _feedCells; }
  inline  void                   EtesianEngine::selectBloat        ( std::string profile ) { _bloatCells.select(profile); }

  inline  Cell*                  EtesianEngine::getBlockCell       () const { return (_block) ? _block->getMasterCell() : getCell(); }
  inline  Instance*              EtesianEngine::getBlockInstance   () const { return  _block; }
  inline  void                   EtesianEngine::setBlock           ( Instance* block ) { _block = block; _placed = _block->getMasterCell()->isPlaced(); }
  inline  void                   EtesianEngine::setFixedAbHeight   ( DbU::Unit abHeight ) { _fixedAbHeight = abHeight; }
  inline  void                   EtesianEngine::setFixedAbWidth    ( DbU::Unit abWidth  ) { _fixedAbWidth  = abWidth; }
  inline  void                   EtesianEngine::setSpaceMargin     ( double margin ) { getConfiguration()->setSpaceMargin(margin); }
  inline  void                   EtesianEngine::setAspectRatio     ( double ratio  ) { getConfiguration()->setAspectRatio(ratio); }

// Variables.
  extern const char* missingEtesian;


}  // Etesian namespace.


INSPECTOR_P_SUPPORT(Etesian::EtesianEngine);
