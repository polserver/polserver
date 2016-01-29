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

// **** base class for ref counted classes

#define REFPTR_DEBUG 0

#define REFERER_PARAM(x)

class ref_counted
{
// Construction
protected:
    ref_counted( void ) : _count( 0 )
#if REFPTR_DEBUG
        , _cumulative_references(0)
        , _instance(++_ctor_calls) 
#endif
    {}

public:
// Operations
    unsigned int add_ref( void ) {
#if REFPTR_DEBUG
        ++_cumulative_references; 
#endif
        return ++_count; 
    }
    unsigned int release( void ) { return --_count; }
    unsigned int count( void ) const { return _count; }
#if REFPTR_DEBUG
    unsigned int instance( void ) const { return _instance; }
#endif
    // Representation
protected:
    unsigned int _count;
#if REFPTR_DEBUG
    unsigned int _cumulative_references;
    unsigned int _instance;
    static unsigned int _ctor_calls;
#endif

private: // not implemented
    ref_counted& operator=( const ref_counted& );
    ref_counted( const ref_counted& );
};



// **** ref_ptr class, assuming T implements ref_counted interface
#if REFPTR_DEBUG
extern unsigned int refptr_count;
#endif

template < class T >
class ref_ptr
{
// Construction
public:
    explicit ref_ptr( T* ptr = 0 ) : _ptr( ptr ) 
    { 
        add_ref();
#if REFPTR_DEBUG
        ++refptr_count;
#endif
    }
    
    ref_ptr( const ref_ptr & rptr ) : _ptr( rptr._ptr )
    {
        add_ref();
#if REFPTR_DEBUG
        ++refptr_count;
#endif
    }

    ~ref_ptr( void ) 
    { 
#if REFPTR_DEBUG
        --refptr_count;
#endif
        release(); 
    }

// Operations
    T* get( void ) const        { return _ptr; }
    T* operator->( void ) const { return _ptr; }
    T& operator*( void ) const  { return *_ptr; }

    bool operator!( void ) const { return get() == 0; }
    bool operator==( const ref_ptr & rptr ) const { return get() == rptr.get(); }
    bool operator!=( const ref_ptr & rptr ) const { return get() != rptr.get(); }
    bool operator<( const ref_ptr & rptr ) const { return get() < rptr.get(); }
    bool operator<=( const ref_ptr & rptr ) const { return get() <= rptr.get(); }
    bool operator>( const ref_ptr & rptr ) const { return get() > rptr.get(); }
    bool operator>=( const ref_ptr & rptr ) const { return get() >= rptr.get(); }

    bool operator==( T* ptr ) const { return get() == ptr; }
    bool operator!=( T* ptr ) const {   return get() != ptr; }
    bool operator<( T* ptr ) const { return get() < ptr; }
    bool operator<=( T* ptr ) const { return get() <= ptr; }
    bool operator>( T* ptr ) const { return get() > ptr; }
    bool operator>=( T* ptr ) const { return get() >= ptr; }

    ref_ptr & operator=( const ref_ptr & rptr )
    {
        release();
        _ptr = rptr._ptr;
        add_ref();

        return *this;
    }

    void set( T* ptr )
    {
        release();
        _ptr = ptr;
        add_ref();
    }

    void clear()
    {
        release();
    }

protected:
    void add_ref( void )
    {
        if( _ptr )
        {
            _ptr->add_ref( REFERER_PARAM(this) );
        }
    }

    void release( void )
    {
        if( _ptr )
        {
            if ( 0 == _ptr->release( REFERER_PARAM(this) ) )
            {
                delete _ptr;
            }
            _ptr = 0;
        }
    }

// Representation
private:
    T * _ptr;
};


template< class T >
bool operator==( T* ptr, const ref_ptr< T > & rptr )
{
    return ptr == rptr.get();
}

template< class T >
bool operator!=( T* ptr, const ref_ptr< T > & rptr )
{
    return ptr != rptr.get();
}

template< class T >
bool operator<( T* ptr, const ref_ptr< T > & rptr )
{
    return ptr < rptr.get();
}

template< class T >
bool operator<=( T* ptr, const ref_ptr< T > & rptr )
{
    return ptr <= rptr.get();
}

template< class T >
bool operator>( T* ptr, const ref_ptr< T > & rptr )
{
    return ptr > rptr.get();
}

template< class T >
bool operator>=( T* ptr, const ref_ptr< T > & rptr )
{
    return ptr >= rptr.get();
}

#endif  // __REFPTR_H
