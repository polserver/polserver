/** @file
 *
 * @par History
 * - 2006/10/28 Shinigami: GCC 4.1.1 fix - invalid use of constructor as a template
 * - 2009/09/05 Turley:    Added struct .? and .- as shortcut for .exists() and .erase()
 * - 2009/12/21 Turley:    ._method() call fix
 */
#pragma once

#include "bobjectimp.h"

#include "../clib/fixalloc.h"
#include "../clib/passert.h"
#include "../clib/refptr.h"

#include <iosfwd>
#include <vector>


/**
 * Bscript namespace is for escript stuff, like compiler and basic modules
 */
namespace Pol::Bscript
{

using ValueStackCont = std::vector<BObjectRef>;

class BObject : public ref_counted
{
public:
  explicit BObject( BObjectImp* objimp ) : ref_counted(), objimp( objimp ) { passert( objimp ); }
  BObject( const BObject& obj ) : ref_counted(), objimp( obj.objimp ) {}
  ~BObject() override = default;
  BObject& operator=( const BObject& ) = delete;
  size_t sizeEstimate() const;

  void* operator new( std::size_t len );
  void operator delete( void* );

  bool operator!=( const BObject& obj ) const;
  bool operator==( const BObject& obj ) const;
  bool operator<( const BObject& obj ) const;
  bool operator<=( const BObject& obj ) const;
  bool operator>( const BObject& obj ) const;
  bool operator>=( const BObject& obj ) const;

  BObjectImp* operator->() const { return objimp.get(); }
  bool isTrue() const { return objimp->isTrue(); }

  BObject* clone() const;

  bool isa( BObjectImp::BObjectType type ) const;

  friend std::ostream& operator<<( std::ostream&, const BObject& );
  void printOn( std::ostream& ) const;

  template <typename T = BObjectImp>
  T* impptr();
  template <typename T = BObjectImp>
  const T* impptr() const;
  template <typename T>
  T* impptr_if();  // also als freestanding function available
  template <typename T>
  T* impptr_if() const;  // also als freestanding function available

  template <typename T = BObjectImp>
  T& impref();
  template <typename T = BObjectImp>
  const T& impref() const;

  virtual void setimp( BObjectImp* imp );

private:
  ref_ptr<BObjectImp> objimp;
};

class BConstObject : public BObject
{
public:
  explicit BConstObject( BObjectImp* objimp ) : BObject( objimp ) {}
  ~BConstObject() override = default;
  void setimp( BObjectImp* ) override { /* do nothing */ };
  // This class does not use a specific fixed allocator, sharing the BObject
  // one. Do not add new members to this class.
};

using BObjectImpRefVec = std::vector<ref_ptr<BObjectImp>>;

extern Clib::fixed_allocator<BObject, 256> bobject_alloc;

inline void* BObject::operator new( std::size_t /*len*/ )
{
  return bobject_alloc.allocate();
}

inline void BObject::operator delete( void* p )
{
  bobject_alloc.deallocate( p );
}

inline bool BObject::isa( BObjectImp::BObjectType type ) const
{
  return objimp->isa( type );
}

template <typename T>
inline T* BObject::impptr()
{
  return static_cast<T*>( objimp.get() );
}

template <typename T>
inline const T* BObject::impptr() const
{
  return static_cast<T*>( objimp.get() );
}

template <typename T>
T* BObject::impptr_if()
{
  return impptrIf<T>( objimp.get() );
}

template <typename T>
T* BObject::impptr_if() const
{
  return impptrIf<T>( objimp.get() );
}

template <typename T>
inline T& BObject::impref()
{
  return static_cast<T&>( *objimp );
}
template <typename T>
inline const T& BObject::impref() const
{
  return static_cast<T&>( *objimp );
}
inline void BObject::setimp( BObjectImp* imp )
{
  objimp.set( imp );
}

class BObjectRef : public ref_ptr<BObject>
{
public:
  explicit BObjectRef( BObject* pobj = nullptr ) : ref_ptr<BObject>( pobj ) {}
  explicit BObjectRef( BObjectImp* pimp ) : ref_ptr<BObject>( new BObject( pimp ) ) {}
  void set( BObject* obj ) { ref_ptr<BObject>::set( obj ); }
  void set( BObjectImp* imp ) { ref_ptr<BObject>::set( new BObject( imp ) ); }
  size_t sizeEstimate() const;
};

}  // namespace Pol::Bscript
