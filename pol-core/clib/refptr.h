/** @file
 *
 * @par History
 *
 * @note ATTENTION
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */

////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1997 Software Masters
// All rights reserved
//
////////////////////////////////////////////////////////////////////

#ifndef __REFPTR_H
#define __REFPTR_H

#include <atomic>

// **** base class for ref counted classes

#define REFPTR_DEBUG 0

#define REFERER_PARAM( x )

class ref_counted
{
  // Construction
protected:
  ref_counted();
  ~ref_counted() = default;

public:
  // Operations
  unsigned int add_ref();
  unsigned int release();
  unsigned int count() const;
#if REFPTR_DEBUG
  unsigned int instance() const;
#endif
  // Representation
protected:
  std::atomic<unsigned int> _count;
#if REFPTR_DEBUG
  unsigned int _cumulative_references;
  unsigned int _instance;
  static unsigned int _ctor_calls;
#endif

private:  // not implemented
  ref_counted& operator=( const ref_counted& );
  ref_counted( const ref_counted& );
};

// **** ref_ptr class, assuming T implements ref_counted interface
#if REFPTR_DEBUG
extern unsigned int refptr_count;
#endif

template <class T>
class ref_ptr
{
  // Construction
public:
  explicit ref_ptr( T* ptr = 0 );
  ref_ptr( const ref_ptr& rptr );
  ref_ptr( ref_ptr&& rptr ) noexcept;
  ~ref_ptr();

  // Operations
  T* get() const;
  T* operator->() const;
  T& operator*() const;

  bool operator!() const;
  bool operator==( const ref_ptr& rptr ) const;
  bool operator!=( const ref_ptr& rptr ) const;
  bool operator<( const ref_ptr& rptr ) const;
  bool operator<=( const ref_ptr& rptr ) const;
  bool operator>( const ref_ptr& rptr ) const;
  bool operator>=( const ref_ptr& rptr ) const;

  bool operator==( T* ptr ) const;
  bool operator!=( T* ptr ) const;
  bool operator<( T* ptr ) const;
  bool operator<=( T* ptr ) const;
  bool operator>( T* ptr ) const;
  bool operator>=( T* ptr ) const;

  ref_ptr& operator=( const ref_ptr& rptr );
  ref_ptr& operator=( ref_ptr&& rptr );

  void set( T* ptr );
  void clear();

protected:
  void add_ref();
  void release();

  // Representation
private:
  std::atomic<T*> _ptr;
};

inline ref_counted::ref_counted()
    : _count( 0 )
#if REFPTR_DEBUG
      ,
      _cumulative_references( 0 ),
      _instance( ++_ctor_calls )
#endif
{
}

inline unsigned int ref_counted::add_ref()
{
#if REFPTR_DEBUG
  ++_cumulative_references;
#endif
  return ++_count;
}
inline unsigned int ref_counted::release()
{
  return --_count;
}
inline unsigned int ref_counted::count() const
{
  return _count;
}
#if REFPTR_DEBUG
inline unsigned int ref_counted::instance() const
{
  return _instance;
}
#endif

template <class T>
ref_ptr<T>::ref_ptr( T* ptr ) : _ptr( ptr )
{
  add_ref();
#if REFPTR_DEBUG
  ++refptr_count;
#endif
}
template <class T>
ref_ptr<T>::ref_ptr( const ref_ptr& rptr ) : _ptr( rptr.get() )
{
  add_ref();
#if REFPTR_DEBUG
  ++refptr_count;
#endif
}

template <class T>
ref_ptr<T>::ref_ptr( ref_ptr&& rptr ) noexcept : _ptr( rptr._ptr.exchange( nullptr ) )
{
#if REFPTR_DEBUG
  --refptr_count;
#endif
}

template <class T>
ref_ptr<T>::~ref_ptr()
{
#if REFPTR_DEBUG
  --refptr_count;
#endif
  release();
}

template <class T>
T* ref_ptr<T>::get() const
{
  return _ptr;
}
template <class T>
T* ref_ptr<T>::operator->() const
{
  return _ptr;
}
template <class T>
T& ref_ptr<T>::operator*() const
{
  return *_ptr;
}

template <class T>
bool ref_ptr<T>::operator!() const
{
  return get() == 0;
}
template <class T>
bool ref_ptr<T>::operator==( const ref_ptr<T>& rptr ) const
{
  return get() == rptr.get();
}
template <class T>
bool ref_ptr<T>::operator!=( const ref_ptr<T>& rptr ) const
{
  return get() != rptr.get();
}
template <class T>
bool ref_ptr<T>::operator<( const ref_ptr<T>& rptr ) const
{
  return get() < rptr.get();
}
template <class T>
bool ref_ptr<T>::operator<=( const ref_ptr<T>& rptr ) const
{
  return get() <= rptr.get();
}
template <class T>
bool ref_ptr<T>::operator>( const ref_ptr<T>& rptr ) const
{
  return get() > rptr.get();
}
template <class T>
bool ref_ptr<T>::operator>=( const ref_ptr<T>& rptr ) const
{
  return get() >= rptr.get();
}

template <class T>
bool ref_ptr<T>::operator==( T* ptr ) const
{
  return get() == ptr;
}
template <class T>
bool ref_ptr<T>::operator!=( T* ptr ) const
{
  return get() != ptr;
}
template <class T>
bool ref_ptr<T>::operator<( T* ptr ) const
{
  return get() < ptr;
}
template <class T>
bool ref_ptr<T>::operator<=( T* ptr ) const
{
  return get() <= ptr;
}
template <class T>
bool ref_ptr<T>::operator>( T* ptr ) const
{
  return get() > ptr;
}
template <class T>
bool ref_ptr<T>::operator>=( T* ptr ) const
{
  return get() >= ptr;
}
template <class T>
ref_ptr<T>& ref_ptr<T>::operator=( const ref_ptr<T>& rptr )
{
  release();
  _ptr = rptr.get();
  add_ref();

  return *this;
}

template <class T>
ref_ptr<T>& ref_ptr<T>::operator=( ref_ptr<T>&& rptr )
{
  release();
  _ptr = rptr._ptr.exchange( nullptr );
  return *this;
}

template <class T>
void ref_ptr<T>::set( T* ptr )
{
  if ( *this == ptr )  // protection against self assignment
    return;
  release();
  _ptr = ptr;
  add_ref();
}
template <class T>
void ref_ptr<T>::clear()
{
  release();
}

template <class T>
void ref_ptr<T>::add_ref()
{
  T* Pointee = _ptr.load();
  if ( Pointee )
  {
    Pointee->add_ref( REFERER_PARAM( this ) );
  }
}
template <class T>
void ref_ptr<T>::release()
{
  T* Pointee = _ptr.exchange( nullptr );
  if ( Pointee )
  {
    if ( 0 == Pointee->release( REFERER_PARAM( this ) ) )
    {
      delete Pointee;
    }
  }
}

template <class T>
bool operator==( T* ptr, const ref_ptr<T>& rptr )
{
  return ptr == rptr.get();
}

template <class T>
bool operator!=( T* ptr, const ref_ptr<T>& rptr )
{
  return ptr != rptr.get();
}

template <class T>
bool operator<( T* ptr, const ref_ptr<T>& rptr )
{
  return ptr < rptr.get();
}

template <class T>
bool operator<=( T* ptr, const ref_ptr<T>& rptr )
{
  return ptr <= rptr.get();
}

template <class T>
bool operator>( T* ptr, const ref_ptr<T>& rptr )
{
  return ptr > rptr.get();
}

template <class T>
bool operator>=( T* ptr, const ref_ptr<T>& rptr )
{
  return ptr >= rptr.get();
}

#endif  // __REFPTR_H
