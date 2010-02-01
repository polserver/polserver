// History
//   2006/09/26 Shinigami: GCC 3.4.x fix - added "this" to weak_ptr

#ifndef __WEAKPTR_H
#define __WEAKPTR_H

#include "refptr.h"

template<class T> class weak_ptr_owner;

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

template<class T>
class weak_ptr_link : public ref_counted
{
public:
    weak_ptr_link() : _ptr(0) {}

    void clear_weakptr() { _ptr = 0; }
    T* get_weakptr() const { return _ptr; }
    void set_weakptr(T* ptr) { _ptr = ptr; }
private:
    T* _ptr;
};


template<class T>
class weak_ptr : protected ref_ptr< weak_ptr_link<T> >
{
    typedef ref_ptr< weak_ptr_link<T> > base;
public:
    explicit weak_ptr( weak_ptr_link<T>* wpl ) : ref_ptr< weak_ptr_link<T> >(wpl) {}

    weak_ptr& operator=( const weak_ptr_link<T>* wpl ) { base::set(wpl); }
    weak_ptr& operator=( const weak_ptr_owner<T>& wpo );
    
    T* get_weakptr( void ) const { return this->get()->get_weakptr(); }
    bool exists() const { return (this->get() != NULL) && (this->get()->get_weakptr() != NULL); }
    void clear() { base::set(0); }

    T* operator->( void ) const { return this->get()->get_weakptr(); }
	T& operator*( void ) const { return *this->get(); }
private:
};

template<class T>
class weak_ptr_owner : public weak_ptr<T>
{
public:
    weak_ptr_owner() : weak_ptr<T>( new weak_ptr_link<T> )
    {
    }
    
    ~weak_ptr_owner() 
    {
        this->ref_ptr< weak_ptr_link<T> >::get()->clear_weakptr();
    }

    void set( T* obptr ) 
    { 
        this->ref_ptr< weak_ptr_link<T> >::get()->set_weakptr(obptr); 
    }
};

template<class T>
weak_ptr<T>& weak_ptr<T>::operator=( const weak_ptr_owner<T>& wpo ) 
{
    base::set( wpo.get() );
    return *this;
//    base::set( 
}


#endif	// __WEAKPTR_H
