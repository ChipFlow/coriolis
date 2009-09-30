// x-----------------------------------------------------------------x 
// |                                                                 |
// |                   C O R I O L I S                               |
// |    I s o b a r  -  Hurricane / Python Interface                 |
// |                                                                 |
// |  Author      :                    Sophie BELLOEIL               |
// |  E-mail      :       Sophie.Belloeil@asim.lip6.fr               |
// | =============================================================== |
// |  C++ Module  :       "./PyNet.cpp"                              |
// | *************************************************************** |
// |  U p d a t e s                                                  |
// |                                                                 |
// x-----------------------------------------------------------------x




#include "hurricane/isobar/PyNet.h"
#include "hurricane/isobar/PyCell.h" 
#include "hurricane/isobar/PyPoint.h" 
#include "hurricane/isobar/PyPlugCollection.h" 
#include "hurricane/isobar/PySegmentCollection.h" 
#include "hurricane/isobar/PyComponentCollection.h" 
#include "hurricane/isobar/PyPinCollection.h" 

#include "hurricane/Cell.h"
#include "hurricane/NetExternalComponents.h"
using namespace Hurricane;



namespace  Isobar {


extern "C" {


#undef  ACCESS_OBJECT
#undef  ACCESS_CLASS
#define ACCESS_OBJECT           _baseObject._object
#define ACCESS_CLASS(_pyObject)  &(_pyObject->_baseObject)
#define METHOD_HEAD(function)   GENERIC_METHOD_HEAD(Net,net,function)

#define  LOAD_CONSTANT(CONSTANT_VALUE,CONSTANT_NAME)             \
 constant = PyInt_FromLong ( (long)CONSTANT_VALUE );             \
 PyDict_SetItemString ( dictionnary, CONSTANT_NAME, constant );  \
 Py_DECREF ( constant );


// x=================================================================x
// |                 "PyNet" Python Module Code Part                 |
// x=================================================================x

#if defined(__PYTHON_MODULE__)


  // x-------------------------------------------------------------x
  // |                 "PyNet" Local Functions                     |
  // x-------------------------------------------------------------x

  
  // ---------------------------------------------------------------
  // Local Function  :  "PyInt_AsType ()"
  
  static Net::Type  PyInt_AsType ( PyObject* object ) {
    switch ( PyInt_AsLong(object) ) {
      case Net::Type::UNDEFINED : return ( Net::Type(Net::Type::UNDEFINED) );
      case Net::Type::LOGICAL   : return ( Net::Type(Net::Type::LOGICAL) );
      case Net::Type::CLOCK     : return ( Net::Type(Net::Type::CLOCK) );
      case Net::Type::POWER     : return ( Net::Type(Net::Type::POWER) );
      case Net::Type::GROUND    : return ( Net::Type(Net::Type::GROUND) );
    }

    return ( Net::Type(Net::Type::UNDEFINED) );
  }



  // ---------------------------------------------------------------
  // Local Function  :  "PyInt_AsDirection ()"

  static Net::Direction  PyInt_AsDirection ( PyObject* object )
  {
    switch ( PyInt_AsLong(object) ) {
      case Net::Direction::UNDEFINED      : return ( Net::Direction(Net::Direction::UNDEFINED) );
      case Net::Direction::IN             : return ( Net::Direction(Net::Direction::IN) );
      case Net::Direction::OUT            : return ( Net::Direction(Net::Direction::OUT) );
      case Net::Direction::INOUT          : return ( Net::Direction(Net::Direction::INOUT) );
      case Net::Direction::TRISTATE       : return ( Net::Direction(Net::Direction::TRISTATE) );
    }

    return ( Net::Direction(Net::Direction::UNDEFINED) );
  }




  // x-------------------------------------------------------------x
  // |                  Global Constants Loading                   |
  // x-------------------------------------------------------------x

  extern void  NetLoadConstants ( PyObject* dictionnary ) {
    PyObject* constant;

    LOAD_CONSTANT ( Net::Type::UNDEFINED        , "TypeUNDEFINED" )
    LOAD_CONSTANT ( Net::Type::LOGICAL          , "TypeLOGICAL" )
    LOAD_CONSTANT ( Net::Type::CLOCK            , "TypeCLOCK" )
    LOAD_CONSTANT ( Net::Type::POWER            , "TypePOWER" )
    LOAD_CONSTANT ( Net::Type::GROUND           , "TypeGROUND" )
    LOAD_CONSTANT ( Net::Direction::UNDEFINED   , "DirectionUNDEFINED" )
    LOAD_CONSTANT ( Net::Direction::IN          , "DirectionIN" )
    LOAD_CONSTANT ( Net::Direction::OUT         , "DirectionOUT" )
    LOAD_CONSTANT ( Net::Direction::INOUT       , "DirectionINOUT" )
    LOAD_CONSTANT ( Net::Direction::TRISTATE    , "DirectionTRISTATE" )
  }



  // x-------------------------------------------------------------x
  // |                "PyNet" Attribute Methods                    |
  // x-------------------------------------------------------------x



  // Standart Accessors (Attributes).
  DirectGetLongAttribute(PyNet_getX,getX,PyNet,Net)
  DirectGetLongAttribute(PyNet_getY,getY,PyNet,Net)  
  
  // Standart Predicates (Attributes).
  DirectGetBoolAttribute(PyNet_isGlobal  ,isGlobal  ,PyNet,Net)
  DirectGetBoolAttribute(PyNet_isExternal,isExternal,PyNet,Net)
  DirectGetBoolAttribute(PyNet_isLogical ,isLogical ,PyNet,Net)
  DirectGetBoolAttribute(PyNet_isClock   ,isClock   ,PyNet,Net)
  DirectGetBoolAttribute(PyNet_isGround  ,isGround  ,PyNet,Net)
  DirectGetBoolAttribute(PyNet_isPower   ,isPower   ,PyNet,Net)
  DirectGetBoolAttribute(PyNet_isSupply  ,isSupply  ,PyNet,Net)

  GetBoundStateAttribute(PyNet_IsPyBound            ,PyNet,Net)

  // Standart destroy (Attribute).
  DBoDestroyAttribute(PyNet_destroy, PyNet)




  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_getName ()"
  GetNameMethod(Net, net)


  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_getType ()"

  static PyObject* PyNet_getType ( PyNet *self )
  {
    trace << "PyNet_getType ()" << endl;
    
    METHOD_HEAD ( "Net.getType()" )

    return ( (PyObject*)Py_BuildValue("i",(long)net->getType().getCode()) );
  }

  

  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_getDirection ()"

  static PyObject* PyNet_getDirection ( PyNet *self )
  {
    trace << "PyNet_getDirection ()" << endl;
   
    METHOD_HEAD ( "Net.getDirection()" )

    return ( (PyObject*)Py_BuildValue("i",(long)net->getDirection().getCode()) );
  }


  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_getPlugs()"

  static PyObject* PyNet_getPlugs(PyNet *self) {
    trace << "PyNet_getPlugs()" << endl;

    METHOD_HEAD("Net.getPlugs()")
        
    PyPlugCollection* pyPlugCollection = NULL;

    HTRY
    Plugs* plugs = new Plugs(net->getPlugs());

    pyPlugCollection = PyObject_NEW(PyPlugCollection, &PyTypePlugCollection);
    if (pyPlugCollection == NULL) { 
        return NULL;
    }

    pyPlugCollection->_object = plugs;
    HCATCH
    
    return (PyObject*)pyPlugCollection;
  }

    
  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_getSegments()"

  static PyObject* PyNet_getSegments(PyNet *self) {
    trace << "PyNet_getSegments()" << endl;

    METHOD_HEAD ("Net.getSegments()")

    PySegmentCollection* pySegmentCollection = NULL;

    HTRY
    Segments* nets = new Segments(net->getSegments());

    pySegmentCollection = PyObject_NEW(PySegmentCollection, &PyTypeSegmentCollection);
    if (pySegmentCollection == NULL) { 
        return NULL;
    }

    pySegmentCollection->_object = nets;
    HCATCH
    
    return (PyObject*)pySegmentCollection;
  }


  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_getPins()"

  static PyObject* PyNet_getPins(PyNet *self) {
    trace << "PyNet_getPins()" << endl;

    METHOD_HEAD ("Net.getPins()")

    PyPinCollection* pyPinCollection = NULL;

    HTRY
    Pins* pins = new Pins(net->getPins());

    pyPinCollection = PyObject_NEW(PyPinCollection, &PyTypePinCollection);
    if (pyPinCollection == NULL) { 
        return NULL;
    }

    pyPinCollection->_object = pins;
    HCATCH
    
    return (PyObject*)pyPinCollection;
  }
  
  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_getExternalComponents()"

  static PyObject* PyNet_getExternalComponents(PyNet *self) {
    trace << "PyNet_getExternalComponents()" << endl;

    METHOD_HEAD ( "Net.getExternalcomponents()" )

    PyComponentCollection* pyComponentCollection = NULL;

    HTRY
    Components* components = new Components(NetExternalComponents::get(net));

    pyComponentCollection = PyObject_NEW(PyComponentCollection, &PyTypeComponentCollection);
    if (pyComponentCollection == NULL) { 
        return NULL;
    }

    pyComponentCollection->_object = components;
    HCATCH
    
    return (PyObject*)pyComponentCollection;
  }




  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_setName ()"

  SetNameMethod(Net, net)


  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_setGlobal ()"

  static PyObject* PyNet_setGlobal ( PyNet *self, PyObject* args ) {
    trace << "PyNet_setGlobal()" << endl;

    HTRY
    METHOD_HEAD ( "Net.setGlobal()" )
    PyObject* arg0;
    if (PyArg_ParseTuple(args,"O:Net.setGlobal", &arg0) && PyBool_Check(arg0)) {
      (arg0 == Py_True)?net->setGlobal(true):net->setGlobal(false);
    } else {
      PyErr_SetString ( ConstructorError, "invalid number of parameters for Net.setGlobal." );
      return NULL;
    }
    HCATCH

    Py_RETURN_NONE;
  }


  
  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_setExternal ()"

  static PyObject* PyNet_setExternal ( PyNet *self, PyObject* args ) {
    trace << "PyNet_setExternal()" << endl;

    HTRY
    METHOD_HEAD ( "Net.setExternal()" )
    PyObject* arg0;
    if (PyArg_ParseTuple(args,"O:Net.setExternal", &arg0) && PyBool_Check(arg0)) {
      (arg0 == Py_True)?net->setExternal(true):net->setExternal(false);
    } else {
      PyErr_SetString ( ConstructorError, "invalid number of parameters for Net.setExternal." );
      return NULL;
    }
    HCATCH

    Py_RETURN_NONE;
  }



  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_setType ()"

  static PyObject* PyNet_setType ( PyNet *self, PyObject* args )
  {
    trace << "PyNet_setType()" << endl;

    HTRY

    METHOD_HEAD ( "Net.setType()" )
    
    PyObject* arg0;
    if (!ParseOneArg("Net.setType", args, INT_ARG, (PyObject**)&arg0)) {
        PyErr_SetString(ConstructorError, "wrong parameter for NetType.");
        return NULL;
    }
    
    net->setType(PyInt_AsType(arg0));

    HCATCH
    
    Py_RETURN_NONE;
  }



  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_setDirection ()"

  static PyObject* PyNet_setDirection ( PyNet *self, PyObject* args )
  {
    trace << "PyNet_setDirection()" << endl;

    HTRY

    METHOD_HEAD ( "Net.setDirection()" )

    PyObject* arg0;
    if ( ! ParseOneArg ( "Net.setDirection", args, INT_ARG, (PyObject**)&arg0 ) ) return ( NULL );

    net->setDirection ( PyInt_AsDirection(arg0) );

    HCATCH

    Py_RETURN_NONE;
  }



  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_setPosition ()"

  static PyObject* PyNet_setPosition ( PyNet *self, PyObject* args )
  {
    trace << "PyNet_setPosition()" << endl;

    HTRY

    METHOD_HEAD ( "Net.setPosition()" )

    PyObject* arg0;
    if ( ! ParseOneArg ( "Net.setPosition", args, POINT_ARG, (PyObject**)&arg0 ) ) return ( NULL );

    net->setPosition ( *PYPOINT_O(arg0) );

    HCATCH

    Py_RETURN_NONE;
  }


  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_merge ()"

  static PyObject* PyNet_merge ( PyNet *self, PyObject* args ) {
    trace << "PyNet_merge()" << endl;

    HTRY
    PyNet* pyNetToMerge;
    METHOD_HEAD ( "Net.merge()" )
    if (PyArg_ParseTuple(args, "O!:Net.merge", &PyTypeNet, &pyNetToMerge)) {
      net->merge(PYNET_O(pyNetToMerge));
    } else {
      PyErr_SetString (ConstructorError, "invalid number of parameters for Net.merge.");
      return NULL;
    }

    HCATCH
    
    Py_RETURN_NONE;
  }






  // ---------------------------------------------------------------
  // PyNet Attribute Method table.

  PyMethodDef PyNet_Methods[] =
    { { "getName"              , (PyCFunction)PyNet_getName                  , METH_NOARGS , "Returns the net name." }
    , { "getType"              , (PyCFunction)PyNet_getType                  , METH_NOARGS
                               , "Returns the signal type (by default set to UNDEFINED)." }
    , { "getDirection"         , (PyCFunction)PyNet_getDirection             , METH_NOARGS
                               , "Returns the signal direction (by default set to UNDEFINED)." }
    , { "getX"                 , (PyCFunction)PyNet_getX                     , METH_NOARGS , "Returns net abscissa." }
    , { "getY"                 , (PyCFunction)PyNet_getY                     , METH_NOARGS , "Returns net ordinate." }
    , { "getExternalComponents", (PyCFunction)PyNet_getExternalComponents , METH_NOARGS , "Returns the collection of net's external components. (only for an external net)" }
    , { "getPlugs"      , (PyCFunction)PyNet_getPlugs          , METH_NOARGS , "Returns the collection of net's plugs." }
    , { "getPins"        , (PyCFunction)PyNet_getPins            , METH_NOARGS , "Returns the collection of net's pins." }
    , { "getSegments"    , (PyCFunction)PyNet_getSegments        , METH_NOARGS , "Returns the collection of net's segments." }
    , { "isGlobal"             , (PyCFunction)PyNet_isGlobal                 , METH_NOARGS, "return true if the net is global" }
    , { "isExternal"           , (PyCFunction)PyNet_isExternal               , METH_NOARGS, "return true if the the net is external." }
    , { "isLogical"            , (PyCFunction)PyNet_isLogical                , METH_NOARGS, "return true if the net is logical ." }
    , { "isClock"              , (PyCFunction)PyNet_isClock                  , METH_NOARGS, "return true if the net is a clock" }
    , { "isPower"              , (PyCFunction)PyNet_isPower                  , METH_NOARGS, "return true if the net is a power" }
    , { "isGround"             , (PyCFunction)PyNet_isGround                 , METH_NOARGS, "return true if the net is a ground" }
    , { "isSupply"             , (PyCFunction)PyNet_isSupply                 , METH_NOARGS, "return true if the net is a supply" }
    , { "isBound"              , (PyCFunction)PyNet_IsPyBound                , METH_NOARGS, "return true if the net is bounded to the hurricane net" }
    , { "setName"              , (PyCFunction)PyNet_setName                  , METH_VARARGS, "Allows to change net name." }
    , { "setGlobal"            , (PyCFunction)PyNet_setGlobal                , METH_VARARGS, "set the net global." }
    , { "setExternal"          , (PyCFunction)PyNet_setExternal              , METH_VARARGS, "set the net external." }
    , { "setType"              , (PyCFunction)PyNet_setType                  , METH_VARARGS, "set the type of the net." }
    , { "setDirection"         , (PyCFunction)PyNet_setDirection             , METH_VARARGS, "set the direction of the net." }
    , { "setPosition"          , (PyCFunction)PyNet_setPosition              , METH_VARARGS, "set the X,Y location of the net." }
    , { "merge"                , (PyCFunction)PyNet_merge                    , METH_VARARGS
                               , "Merges the net <net> to the net <this> which keeps its characteristics (arity, global, external and direction)." }
    , { "destroy"              , (PyCFunction)PyNet_destroy                  , METH_NOARGS
                               , "Destroy associated hurricane object, the python object remains." }
    , {NULL, NULL, 0, NULL}           /* sentinel */
    };




  // x-------------------------------------------------------------x
  // |                  "PyNet" Object Methods                     |
  // x-------------------------------------------------------------x

  DBoDeleteMethod(Net)
  PyTypeObjectLinkPyType(Net)


#else  // End of Python Module Code Part.


// x=================================================================x
// |                "PyNet" Shared Library Code Part                 |
// x=================================================================x


  // ---------------------------------------------------------------
  // Attribute Method  :  "PyNet_create ()"

  PyObject* PyNet_create ( PyObject *module, PyObject *args ) {
    trace << "PyNet_create()" << endl;

    char* name = NULL;
    PyCell* pyCell = NULL;
    Net* net = NULL;
    
    HTRY
    if (PyArg_ParseTuple(args,"O!s:Net.create", &PyTypeCell, &pyCell, &name)) {
        net = Net::create(PYCELL_O(pyCell), Name(name));
    } else {
        PyErr_SetString ( ConstructorError, "invalid number of parameters for Net constructor." );
        return NULL;
    }
    HCATCH

    return PyNet_Link(net);
  }



  // Link/Creation Method.
  DBoLinkCreateMethod(Net)




  // ---------------------------------------------------------------
  // PyNet Object Definitions.

  PyTypeInheritedObjectDefinitions(Net, Entity)


# endif  // End of Shared Library Code Part.


}  // End of extern "C".




}  // End of Isobar namespace.
