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
class ref_counted
{
protected:
  ref_counted();
  virtual ~ref_counted() = default;
  ref_counted& operator=( const ref_counted& ) = delete;
  ref_counted( const ref_counted& ) = delete;

public:
  unsigned int add_ref();
  unsigned int release();
  unsigned int count() const;

private:
  std::atomic<unsigned int> _count;
};

// **** ref_ptr class, assuming T implements ref_counted interface
template <class T>
class ref_ptr
{
public:
  explicit ref_ptr( T* ptr = nullptr );
  ref_ptr( const ref_ptr& rptr );
  ref_ptr( ref_ptr&& rptr ) noexcept;
  ~ref_ptr();

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
  ref_ptr& operator=( ref_ptr&& rptr ) noexcept;

  void set( T* ptr );
  void clear();

private:
  void add_ref();
  void release();

  std::atomic<T*> _ptr;
};

inline ref_counted::ref_counted() : _count( 0 ) {}

inline unsigned int ref_counted::add_ref()
{
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

template <class T>
ref_ptr<T>::ref_ptr( T* ptr ) : _ptr( ptr )
{
  add_ref();
}

template <class T>
ref_ptr<T>::ref_ptr( const ref_ptr& rptr ) : _ptr( rptr.get() )
{
  add_ref();
}

template <class T>
ref_ptr<T>::ref_ptr( ref_ptr&& rptr ) noexcept : _ptr( rptr._ptr.exchange( nullptr ) )
{
}

template <class T>
ref_ptr<T>::~ref_ptr()
{
  release();
}

template <class T>
T* ref_ptr<T>::get() const
{
  return _ptr.load();
}

template <class T>
T* ref_ptr<T>::operator->() const
{
  return get();
}

template <class T>
T& ref_ptr<T>::operator*() const
{
  return *get();
}

template <class T>
bool ref_ptr<T>::operator!() const
{
  return get() == nullptr;
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
  if ( *this != rptr )
  {
    release();
    _ptr = rptr.get();
    add_ref();
  }
  return *this;
}

template <class T>
ref_ptr<T>& ref_ptr<T>::operator=( ref_ptr<T>&& rptr ) noexcept
{
  if ( *this != rptr )
  {
    release();
    _ptr = rptr._ptr.exchange( nullptr );
  }
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
  T* pointee = _ptr.load();
  if ( pointee )
    pointee->add_ref();
}

template <class T>
void ref_ptr<T>::release()
{
  T* pointee = _ptr.exchange( nullptr );
  if ( !pointee )
    return;
  if ( pointee->release() == 0 )
    delete pointee;
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
