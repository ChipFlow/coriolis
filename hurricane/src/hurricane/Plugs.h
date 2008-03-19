// ****************************************************************************************************
// File: Plugs.h
// Authors: R. Escassut
// Copyright (c) BULL S.A. 2000-2004, All Rights Reserved
// ****************************************************************************************************

#ifndef HURRICANE_PLUGS
#define HURRICANE_PLUGS

#include "Collection.h"

namespace Hurricane {

class Plug;



// ****************************************************************************************************
// Plugs declaration
// ****************************************************************************************************

typedef GenericCollection<Plug*> Plugs;



// ****************************************************************************************************
// PlugLocator declaration
// ****************************************************************************************************

typedef GenericLocator<Plug*> PlugLocator;



// ****************************************************************************************************
// PlugFilter declaration
// ****************************************************************************************************

typedef GenericFilter<Plug*> PlugFilter;



// ****************************************************************************************************
// for_each_plug declaration
// ****************************************************************************************************

#define for_each_plug(plug, plugs)\
/*********************************/\
{\
    PlugLocator _locator = plugs.getLocator();\
    while (_locator.isValid()) {\
        Plug* plug = _locator.getElement();\
        _locator.progress();



} // End of Hurricane namespace.

#endif // HURRICANE_PLUGS

// ****************************************************************************************************
// Copyright (c) BULL S.A. 2000-2004, All Rights Reserved
// ****************************************************************************************************
