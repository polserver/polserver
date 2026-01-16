/** @file
 *
 * @par History
 *
 * @note ATTENTION
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */

#ifndef __WEAKPTR_H
#define __WEAKPTR_H

#include "refptr.h"
#include <atomic>

//
//    weak_ptr_owner<T>
//        -------- ref_ptr to:     +---------------------+
//                     ------->    |   weak_ptr_link     |
//                                 |      ref_ptr to T*  |
//                                 +---------------------+
//                                    ^   ^
//    weak_ptr<T> ref_ptr to ---------+   |
//                                        |
//    weak_ptr<T> ref_ptr to -------------+
//

template <class T>
class weak_ptr_owner;

template <class T>
class weak_ptr_link : public ref_counted
{
public:
  weak_ptr_link();
  void clear_weakptr();
  T* get_weakptr() const;
  void set_weakptr( T* ptr );

private:
  std::atomic<T*> _ptr;
};

template <class T>
class weak_ptr : protected ref_ptr<weak_ptr_link<T>>
{
  using base = ref_ptr<weak_ptr_link<T>>;

public:
  explicit weak_ptr( weak_ptr_link<T>* wpl );

  weak_ptr& operator=( const weak_ptr_link<T>* wpl );
  weak_ptr& operator=( const weak_ptr_owner<T>& wpo );

  T* get_weakptr() const;
  bool exists() const;
  void clear();

  T* operator->() const;
  T& operator*() const;
};

template <class T>
class weak_ptr_owner : public weak_ptr<T>
{
public:
  weak_ptr_owner();
  ~weak_ptr_owner();
  weak_ptr_owner( const weak_ptr_owner& ) = delete;
  weak_ptr_owner& operator=( const weak_ptr_owner& ) = delete;
  void set( T* obptr );
  weak_ptr<T> non_owning() const;
};

template <class T>
weak_ptr_link<T>::weak_ptr_link() : _ptr( 0 )
{
}
template <class T>
void weak_ptr_link<T>::clear_weakptr()
{
  _ptr = 0;
}
template <class T>
T* weak_ptr_link<T>::get_weakptr() const
{
  return _ptr;
}
template <class T>
void weak_ptr_link<T>::set_weakptr( T* ptr )
{
  _ptr = ptr;
}

template <class T>
weak_ptr<T>::weak_ptr( weak_ptr_link<T>* wpl ) : ref_ptr<weak_ptr_link<T>>( wpl )
{
}
template <class T>
weak_ptr<T>& weak_ptr<T>::operator=( const weak_ptr_link<T>* wpl )
{
  base::set( wpl );
  return *this;
}
template <class T>
weak_ptr<T>& weak_ptr<T>::operator=( const weak_ptr_owner<T>& wpo )
{
  base::set( wpo.get() );
  return *this;
}
template <class T>
T* weak_ptr<T>::get_weakptr() const
{
  return this->get()->get_weakptr();
}
template <class T>
bool weak_ptr<T>::exists() const
{
  return ( this->get() != 0 ) && ( this->get()->get_weakptr() != 0 );
}
template <class T>
void weak_ptr<T>::clear()
{
  base::set( 0 );
}
template <class T>
T* weak_ptr<T>::operator->() const
{
  return this->get()->get_weakptr();
}
template <class T>
T& weak_ptr<T>::operator*() const
{
  return *this->get();
}

template <class T>
weak_ptr_owner<T>::weak_ptr_owner() : weak_ptr<T>( new weak_ptr_link<T> )
{
}
template <class T>
weak_ptr_owner<T>::~weak_ptr_owner()
{
  this->ref_ptr<weak_ptr_link<T>>::get()->clear_weakptr();
}
template <class T>
void weak_ptr_owner<T>::set( T* obptr )
{
  this->ref_ptr<weak_ptr_link<T>>::get()->set_weakptr( obptr );
}
template <class T>
weak_ptr<T> weak_ptr_owner<T>::non_owning() const
{
  return *this;
}

#endif  // __WEAKPTR_H
