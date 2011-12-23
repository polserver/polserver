///*
//History
//=======
//2006/10/07 Shinigami: malloc.h -> stdlib.h 
//                      GCC 3.4.x fix - added "this" to RefRArray
//
//Notes
//=======
//
//*/
//
//#ifndef __RARRAY_H
//#define __RARRAY_H
//
//#include <assert.h>
//
//#ifndef __STDIO_H
//#include <stdio.h>
//#endif
//#ifndef __ALLOC_H
//#include <stdlib.h>
//#endif
////#ifndef __MEM_H
////#include <mem.h>
////#endif
//
//class GenRArray
//{
//  protected:
//    void **ar;
//    unsigned int limit;			// how many pointers there is currently storage for
//    unsigned int delta;			// how many pointers to grow by
//    unsigned int nitems;		// how many pointers are used
////    void (*killer)(void *data);
//    virtual void destroy(void *data);
//  protected:
//
//    void *get(void *target, int (*f)(void *, void *));
//
//    GenRArray(
//  //      void (*pKiller)(void *) = NULL,
//       int ilimit = 0, int idelta= 3);
//
//    virtual ~GenRArray();
//    void sort(int (*comp)(void *a, void *b));
//
//    int read(FILE *fp, size_t size);
//    int write(FILE *fp, size_t size);
//
//    int read(unsigned char *fname, size_t size);
//    int write(unsigned char *fname, size_t size);
//
//  public:
//    void resizeto(unsigned int newlimit);
//    int append(void *ptr) { return insert(ptr,nitems); };
//
//    const void *retrieve(unsigned position) const;
//    const void *operator[](unsigned x) const { return retrieve(x); };
//
//    void *retrieve(unsigned position);
//    void *operator[](unsigned x) { return retrieve(x); };
//
//
//
//    int find(void *data, unsigned *which);
//    unsigned count(void) const { return nitems; }
//    int isEmpty(void) { return (nitems == 0); }
//    int kill(unsigned position);
//    void flush(void);
//    int insert(void *ptr, unsigned position);
//	int setat( void *ptr, unsigned position);
//    int remove(unsigned position);
//    void *get(unsigned position);
//    void traverse(void (*f)(void *elem, void *staticdata), void *sdata);
//
//    /* Functions provided for Standard C++ similarity */
//    unsigned size() const { return nitems; }
//};
//
//template <class T>
//class RArray : public GenRArray
//{
//  protected:
//    virtual void destroy(void* data) { delete static_cast<T*>(data); }
//  public:
//    RArray(int ilimit = 0, int idelta = 3) :
//          GenRArray(ilimit, idelta) { }
//    virtual ~RArray() { flush(); }
//
//	T* retrieve(unsigned x) const { assert( x < nitems ); return static_cast<T*>(ar[x]); }
//	
//	T* &operator[](unsigned x) { assert( x < nitems ); return reinterpret_cast<T*&>(ar[x]); }
//
//    T* operator[](unsigned x) const {  assert( x < nitems ); return static_cast<T*>(ar[x]); }
//    T* get(unsigned x) { return static_cast<T*>(get(x)); }
//
//    int find(T* data, unsigned* which) { return GenRArray::find(data,which); }
//    int insert(T* ptr, unsigned posn) { return GenRArray::insert(ptr,posn); }
//    int append(T* ptr) { return GenRArray::insert(ptr,nitems); }
//    void sort(int (*comp)(T* a, T* b))
//     { GenRArray::sort((int (*)(void*, void*))comp); }
//
//    T* get(void *target, int (*f)(void *, T*))
//     { return static_cast<T*>( GenRArray::get(target, (int (*)(void *, void *)) f) ); }
//
//    int read(FILE* fp) { return GenRArray::read(fp,sizeof(T)); }
//    int write(FILE* fp) { return GenRArray::write(fp,sizeof(T)); }
//
//    int read(unsigned char *fn) { return GenRArray::read(fn,sizeof(T)); }
//    int write(unsigned char *fn) { return GenRArray::write(fn,sizeof(T)); }
//
//    void traverse(void (*f)(T* elem, void* staticdata), void* sdata)
//     { GenRArray::traverse((void (*)(void*, void*)) f, sdata); }
//
//    int setat( T* ptr, unsigned idx ) { return GenRArray::setat( ptr,idx ); }
//
//    using GenRArray::count;
//    using GenRArray::isEmpty;
//    using GenRArray::flush;
//    using GenRArray::remove;
//    using GenRArray::kill;
//
//    using GenRArray::size;
//};
//
//template <class T, class A>
//void ForEach( RArray<T>& arr, void (*f)(T *elem, A* staticdata), A* staticdata )
//{
//	unsigned idx;
//	for(idx = 0; idx < arr.count(); idx++)
//	{
//		T* elem = arr[idx];
//		if (elem) 
//			(*f)(elem, staticdata);
//	}
//}
//
//template <class T, class A>
//void ConstForEach( RArray<T>& arr, void (*f)(T* elem, const A* staticdata), const A* staticdata )
//{
//	unsigned idx;
//	for(idx = 0; idx < arr.count(); idx++)
//	{
//		T* elem = arr[idx];
//		if (elem) 
//			(*f)(elem, staticdata);
//	}
//}
//
//// RefRArray - an array which does not own its objects, and so does not delete them.
//template <class T>
//class RefRArray : public RArray<T>
//{
//  protected:
//    virtual void destroy(void *data) { }
//  public:
////    RefRArray(int ilimit = 0, int idelta = 3) :
////          RArray<T>(ilimit, idelta) { }
//    virtual ~RefRArray() { this->flush(); }
//};
//#endif
