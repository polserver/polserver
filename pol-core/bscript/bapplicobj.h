#pragma once

#include "bobjectimp.h"

#include <string>

namespace Pol::Bscript
{
class BApplicObjType
{
};

class BApplicObjBase : public BObjectImp
{
public:
  explicit BApplicObjBase( const BApplicObjType* object_type );

  const BApplicObjType* object_type() const;

public:  // Class Machinery
  BObjectImp* copy() const override = 0;

  std::string getStringRep() const override;
  void printOn( std::ostream& ) const override;
  size_t sizeEstimate() const override = 0;

private:
  const BApplicObjType* object_type_;
};

inline BApplicObjBase::BApplicObjBase( const BApplicObjType* object_type )
    : BObjectImp( OTApplicObj ), object_type_( object_type )
{
}

inline const BApplicObjType* BApplicObjBase::object_type() const
{
  return object_type_;
}


template <class T>
class BApplicObj : public BApplicObjBase
{
public:
  explicit BApplicObj( const BApplicObjType* object_type );
  BApplicObj( const BApplicObjType*, T );

  T& value();
  const T& value() const;
  T* operator->();

  const char* typeOf() const override = 0;
  u8 typeOfInt() const override = 0;
  BObjectImp* copy() const override = 0;
  size_t sizeEstimate() const override;

protected:
  T obj_;
};

template <class T>
BApplicObj<T>::BApplicObj( const BApplicObjType* object_type ) : BApplicObjBase( object_type )
{
}

template <class T>
BApplicObj<T>::BApplicObj( const BApplicObjType* object_type, T obj )
    : BApplicObjBase( object_type ), obj_( std::move( obj ) )
{
}

template <class T>
inline size_t BApplicObj<T>::sizeEstimate() const
{
  return sizeof( BApplicObj<T> );
}

template <class T>
T& BApplicObj<T>::value()
{
  return obj_;
}

template <class T>
const T& BApplicObj<T>::value() const
{
  return obj_;
}

template <class T>
T* BApplicObj<T>::operator->()
{
  return &obj_;
}
}  // namespace Pol::Bscript
