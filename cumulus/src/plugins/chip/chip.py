
# This file is part of the Coriolis Software.
# Copyright (c) Sorbonne Université 2014-2021, All Rights Reserved
#
# +-----------------------------------------------------------------+
# |                   C O R I O L I S                               |
# |      C u m u l u s  -  P y t h o n   T o o l s                  |
# |                                                                 |
# |  Author      :                    Jean-Paul CHAPUT              |
# |  E-mail      :            Jean-Paul.Chaput@lip6.fr              |
# | =============================================================== |
# |  Python      :       "./plugins/chip/chip.py"                   |
# +-----------------------------------------------------------------+


import sys
import traceback
import os.path
import optparse
import math
import cProfile
import pstats
import Cfg
import Hurricane
from   Hurricane  import DataBase
from   Hurricane  import DbU
from   Hurricane  import Point
from   Hurricane  import Transformation
from   Hurricane  import Box
from   Hurricane  import Path
from   Hurricane  import Occurrence
from   Hurricane  import UpdateSession
from   Hurricane  import Breakpoint
from   Hurricane  import Net
from   Hurricane  import RoutingPad
from   Hurricane  import Contact
from   Hurricane  import Horizontal
from   Hurricane  import Vertical
from   Hurricane  import Instance
from   Hurricane  import HyperNet
from   Hurricane  import Query
import Viewer
import CRL
from   CRL        import RoutingLayerGauge
import helpers
from   helpers.io import ErrorMessage
from   helpers.io import WarningMessage
import Etesian
import Anabatic
import Katana
import Unicorn
import plugins
import plugins.cts.clocktree
import plugins.chip
import plugins.chip.padscorona
import plugins.chip.blockpower
import plugins.chip.blockcorona


# --------------------------------------------------------------------
# PlaceRoute class


class PlaceRoute ( object ):

  def __init__ ( self, conf ):
    self.conf      = conf
    self.validated = True
    return


  def _refresh ( self ):
    if self.conf.viewer: self.conf.viewer.fit()
    return


  def validate ( self ):
    self.validated = True
    if len(self.conf.cores) < 1: self.validated = False

    coreAb = self.conf.core.getAbutmentBox()
    if (not coreAb.isEmpty()):
      if     coreAb.getWidth () <= self.conf.coreSize.getWidth() \
         and coreAb.getHeight() <= self.conf.coreSize.getHeight():
        self.conf.coreSize = coreAb
      else:
        raise ErrorMessage( 1, [ 'Core %s already have an abutment box, bigger than the requested one:'
                                 % self.conf.cores[0].getName()
                              , "       Cell abutment box: %s" % str(coreAb)
                              , "    Maximum abutment box: %s" % str(self.conf.coreSize) ] )
        self.validated = False

    return self.validated


  def doCoronaFloorplan ( self ):
    if not self.validated:
      raise ErrorMessage( 1, 'chip.doCoronaFloorplan(): Chip is not valid, aborting.' )
      return

    self.railsNb    = Cfg.getParamInt('chip.block.rails.count'   ).asInt()
    self.hRailWidth = Cfg.getParamInt('chip.block.rails.hWidth'  ).asInt()
    self.vRailWidth = Cfg.getParamInt('chip.block.rails.vWidth'  ).asInt()
    self.hRailSpace = Cfg.getParamInt('chip.block.rails.hSpacing').asInt()
    self.vRailSpace = Cfg.getParamInt('chip.block.rails.vSpacing').asInt()

    if not self.conf.useClockTree: self.railsNb -= 1

    innerBb = Box( self.conf.coreSize )
    innerBb.inflate( self.railsNb * self.vRailWidth + (self.railsNb+1) * self.vRailSpace
                   , self.railsNb * self.hRailWidth + (self.railsNb+1) * self.hRailSpace )
        
    coronaAb = self.conf.corona.getAbutmentBox()
    if innerBb.getWidth() > coronaAb.getWidth():
      raise ErrorMessage( 1, 'Core is too wide to fit into the corona, needs %s but only has %s.'
                          % ( DbU.getValueString(innerBb .getWidth())
                            , DbU.getValueString(coronaAb.getWidth()) ) )

    if innerBb.getHeight() > coronaAb.getHeight():
      raise ErrorMessage( 1, 'Core is too tall to fit into the corona, needs %s but only has %s.'
                          % ( DbU.getValueString(innerBb .getHeight())
                            , DbU.getValueString(coronaAb.getHeight()) ) )

    UpdateSession.open()
    self.conf.core.setAbutmentBox( self.conf.coreSize )
    x = (coronaAb.getWidth () - self.conf.coreSize.getWidth ()) // 2
    y = (coronaAb.getHeight() - self.conf.coreSize.getHeight()) // 2
    x = x - (x % self.conf.getSliceHeight())
    y = y - (y % self.conf.getSliceHeight())
    self.conf.icore.setTransformation ( Transformation(x,y,Transformation.Orientation.ID) )
    self.conf.icore.setPlacementStatus( Instance.PlacementStatus.FIXED )
    UpdateSession.close()
    return


  def doCorePlacement ( self ):
    if not self.validated:
      raise ErrorMessage( 1, 'chip.doCorePlacement(): Chip is not valid, aborting.' )
      return

    coreCell = self.conf.core

    checkUnplaced = plugins.CheckUnplaced( coreCell, plugins.NoFlags )
    if not checkUnplaced.check(): return

    coreCk = None
    if self.conf.coronaCk:
      for plug in self.conf.coronaCk.getPlugs():
        if plug.getInstance() == self.conf.icore:
          coreCk = plug.getMasterNet()
      if not coreCk:
        print( WarningMessage( 'Core "{}" is not connected to chip clock.'.format(self.conf.icore.getName()) ))

    if self.conf.useClockTree and coreCk:
      ht = plugins.cts.clocktree.HTree.create( self.conf, coreCell, coreCk, coreCell.getAbutmentBox() )
      ht.addCloned( self.conf.cell )
      ht.addCloned( self.conf.corona )
      etesian = Etesian.EtesianEngine.create( self.conf.corona )
      etesian.setBlock(  self.conf.icore )
      etesian.setViewer( self.conf.viewer )
      etesian.place()
      etesian.toHurricane()
      etesian.flattenPower()
      etesian.destroy()

      ht.connectLeaf()
      ht.route()
      ht.save( self.conf.cell )
    else:
      etesian = Etesian.EtesianEngine.create( self.conf.corona )
      etesian.setBlock( self.conf.icore )
      etesian.place()
      etesian.toHurricane()
      etesian.flattenPower()
      etesian.destroy()
    return


  def doChipPlacement ( self ):
    if not self.validated:
      raise ErrorMessage( 1, 'chip.doChipPlacement(): Chip is not valid, aborting.' )
      return

    padsCorona = plugins.chip.padscorona.Corona( self.conf )
    self.validated = padsCorona.validate()
    if not self.validated: return False

    padsCorona.doLayout()
    self.validate()
    self.doCoronaFloorplan()
    self._refresh()

    self.doCorePlacement()
    self._refresh()

    coreBlock = plugins.chip.blockpower.Block( self.conf )
    coreBlock.connectPower()
    coreBlock.connectClock()
    coreBlock.doLayout()
    self._refresh()

    coreCorona = plugins.chip.blockcorona.Corona( coreBlock )
    coreCorona.connectPads ( padsCorona )
    coreCorona.connectBlock()
    coreCorona.doLayout()
    self._refresh()

    return


  def doChipRouting ( self ):
    if not self.validated:
      raise ErrorMessage( 1, 'chip.doChipRouting(): Chip is not valid, aborting.' )
      return

    self.conf.corona.setName( self.conf.corona.getName()+"_r" )
    katana = Katana.KatanaEngine.create( self.conf.corona )
   #katana.printConfiguration   ()
    katana.digitalInit          ()
   #katana.runNegociatePreRouted()
    katana.runGlobalRouter      ( Katana.Flags.NoFlags )
    katana.loadGlobalRouting    ( Anabatic.EngineLoadGrByNet )
    katana.layerAssign          ( Anabatic.EngineNoNetLayerAssign )
    katana.runNegociate         ( Katana.Flags.NoFlags )
    success = katana.getSuccessState()
    katana.finalizeLayout()
    katana.destroy()

    return


  def save ( self ):
    if not self.validated:
      raise ErrorMessage( 1, 'chip.save(): Chip is not valid, aborting.' )
      return

    af = CRL.AllianceFramework.get()
    af.saveCell( self.conf.cell  , CRL.Catalog.State.Views )
    af.saveCell( self.conf.corona, CRL.Catalog.State.Views )
    return
