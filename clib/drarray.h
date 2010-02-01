/*
History
=======

Notes
=======

*/

#ifndef __DRARRAY_H
#define __DRARRAY_H

#ifndef __STDIO_H
#	include <stdio.h>
#endif

/* GenDRArray - direct resizing array
   note: not to be used for classes that use:
           . copy constructors
           . destructors
*/
class GenDRArray
{
  protected:
    char *elems;
    unsigned int limit;
    unsigned int delta;
    unsigned int nitems;

    unsigned int datasize;
  public:
    GenDRArray(
            unsigned int idatasize,
            unsigned int ilimit = 0,
            unsigned int idelta = 3);

    virtual ~GenDRArray()
      {
        flush();
        pack();
      };
    int resizeto(unsigned newlimit);

    int count(void) {  return nitems; }

    void flush(void);
    void pack(void);

    void *retrieve(unsigned x);

    int insert(void *data, unsigned posn);
    int append(void *data, unsigned *position = NULL);

    int remove(unsigned posn);

    int read(FILE *fp);
    int read(char *fn);

    void setsize(int x) { nitems = x; };
};

/* Templaye DRArray - direct resizing array
   note: not to be used for classes that use:
           . copy constructors
           . destructors

   Mostly used like DRArray<int> intStorage;


*/

template <class T>
class DRArray : private GenDRArray
{
    T **ar;
  public:
    DRArray(int ilimit = 0,
            int idelta = 3) :
            GenDRArray(sizeof(T), ilimit, idelta),
            ar((T **)&elems) { };
    ~DRArray() { };

    T& retrieve(unsigned x)
      {
        T *data = (T *) GenDRArray::retrieve(x);
        return *data;
      }
    T& operator[](int x) { return retrieve(x); };
    int insert(T& data, unsigned posn)
      {
        return GenDRArray::insert(&data, posn);
      }
    int append(T& data, unsigned *position = NULL)
      {
		return GenDRArray::append(&data, position);
      };

    using GenDRArray::count;
    using GenDRArray::flush;
    using GenDRArray::remove;

};
#endif
