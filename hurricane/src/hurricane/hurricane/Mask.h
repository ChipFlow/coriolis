
// -*- C++ -*-
//
// This file is part of the Coriolis Software.
// Copyright (c) UPMC/LIP6 2008-2008, All Rights Reserved
//
// ===================================================================
//
// $Id$
//
// x-----------------------------------------------------------------x
// |                                                                 |
// |                  H U R R I C A N E                              |
// |     V L S I   B a c k e n d   D a t a - B a s e                 |
// |                                                                 |
// |  Author      :                    Jean-Paul Chaput              |
// |  E-mail      :            Jean-Paul.Chaput@lip6.fr              |
// | =============================================================== |
// |  C++ Header  :       "./Mask.h"                                 |
// | *************************************************************** |
// |  U p d a t e s                                                  |
// |                                                                 |
// x-----------------------------------------------------------------x


#ifndef  __HURRICANE_MASK__
#define  __HURRICANE_MASK__

#include  <iomanip>
#include  "hurricane/Commons.h"


namespace Hurricane {


  template<typename IntType>
  class Mask {
    public:
    // Methods.
      inline         Mask             ( IntType mask=0 );
      inline bool    zero             () const;
      inline Mask&   set              ( const Mask mask );
      inline Mask&   unset            ( const Mask mask );
      inline bool    contains         ( const Mask mask ) const;
      inline bool    intersect        ( const Mask mask ) const;
      inline Mask    nthbit           ( unsigned int ) const;
      inline Mask    operator compl   () const;
      inline Mask    operator bitand  ( const Mask mask ) const;
      inline Mask    operator bitor   ( const Mask mask ) const;
      inline Mask    operator xor     ( const Mask mask ) const;
      inline Mask    lshift           ( unsigned int ) const;
      inline Mask    rshift           ( unsigned int ) const;
      inline Mask&   operator |=      ( const Mask mask );
      inline Mask&   operator &=      ( const Mask mask );
      inline bool    operator ==      ( const Mask mask ) const;
      inline bool    operator !=      ( const Mask mask ) const;
      inline bool    operator <       ( const Mask mask ) const;
      inline bool    operator >       ( const Mask mask ) const;
      inline         operator IntType () const;
    // Hurricane Managment.
      inline string  _getTypeName     () const;
      inline string  _getString       () const;
      inline Record* _getRecord       () const;
    protected:
    // Internal: Attributes.
      static size_t  _width;
             IntType _mask;
  };


// Inline Functions.
  template<typename IntType>
  inline Mask<IntType>::Mask ( IntType mask ) : _mask(mask) { }

  template<typename IntType>
  inline bool  Mask<IntType>::zero () const
  { return _mask == 0; }

  template<typename IntType>
  inline Mask<IntType>& Mask<IntType>::set ( const Mask<IntType> mask )
  { _mask |=  mask._mask; return *this; }

  template<typename IntType>
  inline Mask<IntType>& Mask<IntType>::unset ( const Mask<IntType> mask )
  { _mask &= ~mask._mask; return *this; }

  template<typename IntType>
  inline bool  Mask<IntType>::contains ( const Mask<IntType> mask ) const
  { return (_mask & mask._mask) && !(~_mask & mask._mask); }

  template<typename IntType>
  inline bool  Mask<IntType>::intersect ( const Mask<IntType> mask ) const
  { return _mask & mask._mask; }

  template<typename IntType>
  inline Mask<IntType>  Mask<IntType>::nthbit ( unsigned int nth ) const
  { 
    IntType select = 1;
    for ( ; select ; select=select<<1 ) {
      if ( _mask & select ) nth--;
      if ( !nth ) break;
    }
    return select;
  }

  template<typename IntType>
  inline Mask<IntType>  Mask<IntType>::operator compl () const
  { return ~_mask; }

  template<typename IntType>
  inline Mask<IntType>  Mask<IntType>::operator bitand ( const Mask<IntType> mask ) const
  { return _mask & mask._mask; }

  template<typename IntType>
  inline Mask<IntType>  Mask<IntType>::operator bitor ( const Mask<IntType> mask ) const
  { return _mask | mask._mask; }

  template<typename IntType>
  inline Mask<IntType>  Mask<IntType>::operator xor ( const Mask<IntType> mask ) const
  { return _mask ^ mask._mask; }

  template<typename IntType>
  inline Mask<IntType>  Mask<IntType>::lshift ( unsigned int s ) const
  { return _mask << s; }

  template<typename IntType>
  inline Mask<IntType>  Mask<IntType>::rshift ( unsigned int s ) const
  { return _mask >> s; }

  template<typename IntType>
  inline Mask<IntType>& Mask<IntType>::operator |= ( const Mask<IntType> mask )
  { _mask |= mask._mask; return *this; } 

  template<typename IntType>
  inline Mask<IntType>& Mask<IntType>::operator &= ( const Mask<IntType> mask )
  { _mask &= mask._mask; return *this; } 

  template<typename IntType>
  inline bool  Mask<IntType>::operator == ( const Mask<IntType> mask ) const
  { return _mask == mask._mask; } 

  template<typename IntType>
  inline bool  Mask<IntType>::operator != ( const Mask<IntType> mask ) const
  { return _mask == mask._mask; } 

  template<typename IntType>
  inline bool  Mask<IntType>::operator < ( const Mask<IntType> mask ) const
  { return _mask < mask._mask; } 

  template<typename IntType>
  inline bool  Mask<IntType>::operator > ( const Mask<IntType> mask ) const
  { return _mask > mask._mask; } 

  template<typename IntType>
  inline  Mask<IntType>::operator IntType () const { return _mask; }

  template<typename IntType>
  inline string  Mask<IntType>::_getTypeName     () const { return _TName("Mask"); }

  template<typename IntType>
  inline string  Mask<IntType>::_getString () const
  {
    std::ostringstream formatted;
    formatted << "0x" << std::hex << std::setw(_width) << std::setfill('0') << _mask;
    return formatted.str();
  }

  template<typename IntType>
  inline Record* Mask<IntType>::_getRecord () const
  {
    Record* record = new Record ( _getString() );
    record->add(getSlot("Mask", &_mask));
    return record;
  }


  template<typename IntType>
  size_t  Mask<IntType>::_width = sizeof(IntType)<<2;


} // End of Hurricane namespace.


# endif  // __HURRICANE_MASK__