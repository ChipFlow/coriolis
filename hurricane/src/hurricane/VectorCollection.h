// ****************************************************************************************************
// File: VectorCollection.h
// Authors: R. Escassut
// Copyright (c) BULL S.A. 2000-2004, All Rights Reserved
// ****************************************************************************************************

#ifndef HURRICANE_VECTOR_COLLECTION
#define HURRICANE_VECTOR_COLLECTION

#include "Commons.h"

namespace Hurricane {



// ****************************************************************************************************
// VectorCollection declaration
// ****************************************************************************************************

template<class Element> class VectorCollection : public Collection<Element> {
// ************************************************************************

// Types
// *****

    public: typedef Collection<Element> Inherit;

    public: typedef vector<Element> ElementVector;

    public: class Locator : public Hurricane::Locator<Element> {
    // *******************************************************

        public: typedef Hurricane::Locator<Element> Inherit;

        private: const ElementVector* _elementVector;
        private: typename ElementVector::const_iterator _iterator; // AD

        public: Locator(const ElementVector* elementVector)
        // ************************************************
        :    Inherit(),
            _elementVector(elementVector),
            _iterator()
        {
            if (_elementVector) _iterator = _elementVector->begin();
        };

        public: virtual Element getElement() const
        // ***************************************
        {
            return (isValid()) ? *_iterator : Element();
        };

        public: virtual Hurricane::Locator<Element>* getClone() const
        // **********************************************************
        {
            return new Locator(_elementVector);
        };

        public: virtual bool isValid() const
        // *********************************
        {
            return (_elementVector && (_iterator != _elementVector->end()));
        };

        public: virtual void progress()
        // ****************************
        {
            ++_iterator;
        };

    };

// Attributes
// **********

    private: const ElementVector* _elementVector;

// Constructors
// ************

    public: VectorCollection(const ElementVector* elementVector = NULL)
    // ****************************************************************
    :    Inherit(),
        _elementVector(elementVector)
    {
    };

    public: VectorCollection(const ElementVector& elementVector)
    // *********************************************************
    :    Inherit(),
        _elementVector(&elementVector)
    {
    };

    public: VectorCollection(const VectorCollection& vectorCollection)
    // ***************************************************************
    :    Inherit(),
        _elementVector(vectorCollection._elementVector)
    {
    };

// Operators
// *********

    public: VectorCollection& operator=(const VectorCollection& vectorCollection)
    // **************************************************************************
    {
        _elementVector = vectorCollection._elementVector;
        return *this;
    };

// Accessors
// *********

    public: virtual Collection<Element>* getClone() const
    // **************************************************
    {
        return new VectorCollection(*this);
    }

    public: virtual Hurricane::Locator<Element>* getLocator() const
    // ************************************************************
    {
        // return (_elementVector) ? new Locator<Element>(_elementVector) : NULL;
        // V3
        return (_elementVector) ? new Locator(_elementVector) : NULL;
    }

    public: virtual unsigned getSize() const
    // *************************************
    {
        return (_elementVector) ? _elementVector->size() : 0;
    };

// Others
// ******

    public: virtual string _getTypeName() const
    // **************************************
    {
      return _TName("VectorCollection");
    };

    public: virtual string _getString() const
    // **************************************
    {
        if (!_elementVector)
            return "<" + _getTypeName() + " unbound>";
        else {
            if (_elementVector->empty())
                return "<" + _getTypeName() + " empty>";
            else
                return "<" + _getTypeName() + " " + getString(_elementVector->size()) + ">";
        }
    };

    public: Record* _getRecord() const
    // *************************
    {
      H::Record* record = NULL;
      if (!_elementVector->empty()) {
        record = new Record(_getString());
        unsigned n = 1;
        typename vector<Element>::const_iterator iterator = _elementVector->begin(); // AD
        while (iterator != _elementVector->end()) {
          record->Add(getSlot(getString(n++), *iterator));
          ++iterator;
        }
      }
      return record;
    }

};



// ****************************************************************************************************
// Generic functions
// ****************************************************************************************************

template<class Element>
    inline GenericCollection<Element> getCollection(const vector<Element>& elementVector)
// *************************************************************************************
{
    return VectorCollection<Element>(elementVector);
}

template<class Element>
    inline GenericCollection<Element> getCollection(const vector<Element>* elementVector)
// *************************************************************************************
{
    return VectorCollection<Element>(elementVector);
}



} // End of Hurricane namespace.

#endif // HURRICANE_VECTOR_COLLECTION

// ****************************************************************************************************
// Copyright (c) BULL S.A. 2000-2004, All Rights Reserved
// ****************************************************************************************************
