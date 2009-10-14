#ifndef __RSTACK_H
#define __RSTACK_H

#ifndef __cplusplus
#error RStack is C++ only!
#endif

#ifndef __STRING_H
#include <string.h>
#endif

//#ifdef _WIN32
//#include <float.h>
//#else
//#include <values.h>
//#endif

#ifndef __RARRAY_H
#include "rarray.h"
#endif

#include <assert.h>

#include <iosfwd>

template <class T>
class RStack : private GenRArray
{
    virtual void destroy(void *data)
      {
        delete (T *) data;
      }
  public:
    RStack(int iLimit = 0, int iDelta = 3) :
         GenRArray(iLimit, iDelta) { };
    virtual ~RStack() { flush(); }
    int push(T *ptr)
      {
        return GenRArray::append(ptr);
      }
    T *pop(void)
      {
		assert( count() >= 1 );
        T *data = (T *) GenRArray::get(count()-1);
        return data;
      }
    T *top(void)
      {
		assert( count() >= 1 );
        T * data = (T *) GenRArray::retrieve(count()-1);
        return data;
      }
    
    using GenRArray::isEmpty;
    using GenRArray::flush;

    unsigned size() const { return GenRArray::count(); }

    void printOn( ostream& ) const;
};

template <class T>
void RStack<T>::printOn( ostream& os ) const
{
	os << "RStack: [ ";
	for( unsigned i = 0; i < GenRArray::count(); i++ )
	{
		T *obj = (T *) GenRArray::retrieve(i);
		// os << "\t" << i << ": " << *obj << endl;
		os << "  " << *obj;
	}
	os << " ]";
}

template <class T>
ostream& operator << (ostream& os, const RStack<T>& obj )
{
	obj.printOn( os );
	return os;
}



template <class T>
class RQueue: private GenRArray
{
    virtual void destroy(void *data) { delete (T *) data; }
  public:
    RQueue(int iLimit = 0, int iDelta = 3) :
         GenRArray(iLimit, iDelta) { }
    virtual ~RQueue() { flush(); }
    int put(T *ptr)
      {
        /* TMALLOC assert(tm_isalloced(&ptr)); */
        return GenRArray::append(ptr);
      }
    T *get(void)
      {
        T *data = (T *) GenRArray::get(0);
        /* TMALLOC assert(tm_isalloced(data)); */
        return data;
        //  return * ((T *) GenRArray::get(0));
      }
    T *peek(void)
      {
        T *data = (T *) GenRArray::retrieve(0);
        /* TMALLOC assert(tm_isalloced(data)); */
        return data;
        // return * ((T *) GenRArray::retrieve(0));
      }

    using GenRArray::isEmpty;
    using GenRArray::flush;

    void printOn( ostream& ) const;
};

template <class T>
void RQueue<T>::printOn( ostream& os ) const
{
	os << "RQueue: [ ";
	for( unsigned i = 0; i < GenRArray::count(); i++ )
	{
		T *obj = (T *) GenRArray::retrieve(i);
		// os << "\t" << i << ": " << *obj << endl;
		os << "  " << *obj;
	}
	os << " ]";
}

template <class T>
ostream& operator << (ostream& os, const RQueue<T>& obj )
{
	obj.printOn( os );
	return os;
}



template <class T>
class RDeque: private GenRArray
{
    virtual void destroy(void *data)
      {
        assert(tm_isalloced(data));
        delete (T *) data;
      }
  public:
    RDeque(int iLimit = 0, int iDelta = 3) :
       GenRArray(iLimit, iDelta) { }
    virtual ~RDeque() { flush(); }

    int putRight(T *ptr)
      {
        assert(tm_isalloced(ptr));
        return GenRArray::append(ptr);
      }
    T *getRight(void) { return GenRArray::get(count()-1); }
    T *peekRight(void) { return GenRArray::retrieve(count()-1); }

    int putLeft(T *ptr)
      {
        assert(tm_isalloced(ptr));
        return GenRArray::insert(ptr, 0);
      }
    T *getLeft(void) { return GenRArray::get(0); }
    T *peekLeft(void) { return GenRArray::retrieve(0); }

    using GenRArray::isEmpty;
    using GenRArray::flush;
};

#endif
