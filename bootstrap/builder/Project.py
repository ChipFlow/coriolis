
# -*- mode:Python -*-
#
# This file is part of the Coriolis Software.
# Copyright (c) Sorbonne Université 2012-2021, All Rights Reserved
#
# +-----------------------------------------------------------------+ 
# |                   C O R I O L I S                               |
# |          T o o l c h a i n   B u i l d e r                      |
# |                                                                 |
# |  Author      :                    Jean-Paul Chaput              |
# |  E-mail      :            Jean-Paul.Chaput@lip6.fr              |
# | =============================================================== |
# |  Python      :   "./builder/Project.py"                         |
# +-----------------------------------------------------------------+


class Tool ( object ):

    def __init__ ( self, project, name ):
        self.project = project
        self.name    = name
        self.active  = False
        return

    def getToolDir ( self ): return self.project.getName()+'/'+self.name


class Project ( object ):

    def __init__ ( self, name, toolNames, repository ):
        self._name       = name
        self._tools      = []
        self._repository = repository

        for toolName in toolNames:
            self._tools.append( Tool(self,toolName) )
        return

    def getName       ( self ): return self._name
    def getTools      ( self ): return self._tools
    def getRepository ( self ): return self._repository

    def hasTool ( self, toolName ):
        for tool in self._tools:
            if tool.name == toolName: return True
        return False

    def getActives ( self ):
        actives = []
        for tool in self._tools:
            if tool.active: actives.append( tool )
        return actives

    def desactivate ( self ):
        self._active = []
        return

    def activateAll ( self ):
        for tool in self._tools:
            tool.active = True
        return
    
    def activate ( self, toolNames ):
        for tool in self._tools:
            if tool.name in toolNames:
                tool.active = True

        rejecteds = []
        for tool in toolNames:
            if not self.hasTool(tool):
                rejecteds.append( tool )
        return rejecteds
