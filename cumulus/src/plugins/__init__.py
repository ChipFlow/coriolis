
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
# |  Python      :       "./plugins/__init__.py"                    |
# +-----------------------------------------------------------------+


import os
import sys
import traceback
import Cfg
import helpers
from   helpers.io import vprint
from   helpers.io import ErrorMessage
from   helpers.io import WarningMessage
from   Hurricane  import Contact
from   Hurricane  import Path
from   Hurricane  import Occurrence
from   Hurricane  import Instance
import Viewer
import CRL
from   CRL        import RoutingLayerGauge


NoFlags           = 0
ShowWarnings      = 1
WarningsAreErrors = 2
loaded            = False


def kwParseMain ( **kw ):
    cell = None
    if ('cell' in kw) and kw['cell']:
        cell = kw['cell']
    editor = None
    if ('editor' in kw) and kw['editor']:
        editor = kw['editor']
        if cell == None: cell = editor.getCell()
   #if cell == None:
   #  raise ErrorMessage( 3, 'Chip: No cell loaded yet.' )
    return cell, editor


def kwAddMenu ( menuPath, menuName, **kw ):
    editor = kw['editor']
    if menuPath.find('.') >= 0: flags = Viewer.CellViewer.NoFlags
    else:                       flags = Viewer.CellViewer.TopMenu
    if not editor.hasMenu(menuPath):
        editor.addMenu( menuPath, menuName, flags )
    return


def kwUnicornHook ( menuPath, menuName, menuTip, moduleFile, **kw ):
    editor = kw['editor']
    if moduleFile.endswith('.pyc') or moduleFile.endswith('.pyo'):
        moduleFile = moduleFile[:-1]
    if 'beforeAction' in kw:
        editor.addToMenu( menuPath, menuName, menuTip, moduleFile, kw['beforeAction'] )
    else:  
        editor.addToMenu( menuPath, menuName, menuTip, moduleFile )
    return


def getParameter ( pluginName, key ):
    if not Cfg.hasParameter(key):
        raise ErrorMessage( 1, 'Mandatory parameter <%s> for for plugin <%s> is missing in configuration.'
                               % (key,pluginName) )
    return Cfg.Configuration.get().getParameter(key)
    

class CheckUnplaced ( object ):
  
    def __init__ ( self, cell, flags=NoFlags ):
        self.cell      = cell
        self.flags     = flags
        self.unplaceds = []
        return
  
    def _rcheckUnplaced ( self, path, cell ):
        for instance in cell.getInstances():
            if instance.getPlacementStatus() == Instance.PlacementStatus.UNPLACED:
                self.unplaceds.append( Occurrence(instance,path) )
            
            rpath = Path( path, instance )
            self._rcheckUnplaced( rpath, instance.getMasterCell() )
        return


    def check ( self ):
        self._rcheckUnplaced( Path(), self.cell )
          
        if self.unplaceds:
            if self.flags & (ShowWarnings | WarningsAreErrors):
                message = [ 'Some instances are unplaceds:' ]
                for occurrence in self.unplaceds:
                    message += [ '%s' % (occurrence.getCompactString()) ]
                error = ErrorMessage( 3, message )
                
                if self.flags & WarningsAreErrors: raise error
                else:                              print( error )
        return self.unplaceds


class StackedVia ( object ):

    def __init__ ( self, net, depth, x, y, width, height ):
        self._hasLayout   = False
        self._net         = net
        self._bottomDepth = depth
        self._topDepth    = depth
        self._x           = x
        self._y           = y
        self._width       = width
        self._height      = height
        self._vias        = []
        return

    @property
    def topDepth    ( self ): return self._topDepth
    @property
    def bottomDepth ( self ): return self._bottomDepth

    def getNet ( self ): return self._net

    def mergeDepth ( self, depth ):
        if self._hasLayout:
            print( WarningMessage( 'StackedVia.mergeDepth(): Cannot be called *after* StackVia.doLayout()' ))
            return
        if depth < self._bottomDepth: self._bottomDepth = depth
        if depth > self._topDepth:    self._topDepth    = depth
        return

    def getLayer ( self, fromTop ):
        if self._topDepth-fromTop >= self._bottomDepth:
            routingGauge = CRL.AllianceFramework.get().getRoutingGauge()
            return routingGauge.getRoutingLayer(self._topDepth-fromTop)
        return None

    def getBlockageLayer ( self, fromTop ):
        if self._topDepth-fromTop >= self._bottomDepth:
            routingGauge = CRL.AllianceFramework.get().getRoutingGauge()
            return routingGauge.getRoutingLayer(self._topDepth-fromTop).getBlockageLayer()
        return None

    def getVia ( self, metal ):
        if not self._hasLayout: return None
        for via in self._vias:
            if via.getLayer().contains(metal): return via
        return None

    def doLayout ( self ):
        if self._hasLayout: return
        self._hasLayout = True

        routingGauge = CRL.AllianceFramework.get().getRoutingGauge()
        
        if self._bottomDepth == self._topDepth:
            self._vias.append( Contact.create( self._net
                                             , routingGauge.getRoutingLayer(self._bottomDepth)
                                             , self._x    , self._y
                                             , self._width, self._height
                                             ) )
        else:
            for depth in range(self._bottomDepth,self._topDepth):
                if depth == self._bottomDepth:
                    self._vias.append( Contact.create( self._net
                                                     , routingGauge.getContactLayer(depth)
                                                     , self._x    , self._y
                                                     , self._width, self._height
                                                     ) )
                else:
                    self._vias.append( Contact.create( self._vias[-1]
                                                     , routingGauge.getContactLayer(depth)
                                                     , 0          , 0
                                                     , self._width, self._height
                                                     ) )
                   #print( '    Sub-via: ', self._vias[-1] )
        return


def loadPlugins ( pluginsDir ):
    """
    Forced import of all the modules that resides in the directory ``pluginsDir``.
    Works in three stages:
   
    1. Build a list of all the ``.py`` files in the ``pluginsDir``, in case of
       directories, import the whole package (it is assumed it *is* a Python
       package directory).

    2. Sort the list of modules to be loaded (alphabetical order).
       This is an attempt to get the initialization done in deterministic order. 

    3. Import each module in order. 

    .. note:: Those modules will be searched later (in ``unicornInit.py``) for any
              potential ``unicornHook()`` function.
    """
    sys.path.append( pluginsDir )
    sys.modules['plugins'].__path__.append( pluginsDir )
    if not os.path.isdir(pluginsDir):
        print( ErrorMessage( 3, 'cumulus.__init__.py: Cannot find <cumulus/plugins> directory:' \
                              , '"{}"'.format(pluginsDir) ))
        return

    moduleNames = []
    for entry in os.listdir( pluginsDir ):
        if entry == "__init__.py": continue
        if not entry.endswith('.py'):
            path = os.path.join(pluginsDir,entry)
            if os.path.isdir(path):
                packageName = "plugins." + entry
                if not packageName in sys.modules:
                    module = __import__( packageName, globals(), locals() )
                else:
                    module = sys.modules[packageName]
                module.__path__.append( path )
            continue
        moduleNames.append( entry[:-3] )

    moduleNames.sort()
    for moduleName in moduleNames:
        try:
            vprint( 2, '     - "{}"'.format(moduleName) )
            module = __import__( moduleName, globals(), locals() )
        except ErrorMessage as e:
            print( e )
            helpers.showStackTrace( e.trace )
        except Exception as e:
            print( e )
            helpers.showPythonTrace( __file__, e )

    return


def staticInitialization ():
    global loaded
    if loaded: return
    try:
        vprint( 1, '  o  Preload standard plugins.' )
        pluginsDir = os.path.dirname(__file__)
        loadPlugins( pluginsDir )
        
        if helpers.ndaTopDir:
            vprint( 1, '  o  Preload NDA protected plugins.' )
            pythonDir  =os.path.join( helpers.ndaTopDir, 'python{}.{}'.format( sys.version_info.major
                                                                             , sys.version_info.minor ))
            if os.path.isdir(pythonDir):
                pluginsDir = os.path.join( pythonDir, 'site-packages/cumulus/plugins' )
                loadPlugins( pluginsDir )
            else:
                vprint( 1, '     - No NDA protected plugins directory.' )
        else:
            vprint( 1, '     - No NDA protected plugins.' )
    except Exception as e:
        helpers.showPythonTrace( __file__, e )
    loaded = True
    return


staticInitialization()
