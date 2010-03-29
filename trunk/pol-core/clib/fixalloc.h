/*
History
=======
2005/06/20 Shinigami: added log_stuff and everything needed to analyze a little bit (needs defined MEMORYLEAK)
2006/10/06 Shinigami: malloc.h -> stdlib.h
2007/04/28 Shinigami: polcore().internal information will be logged in excel-friendly format too (leak.log)

Notes
=======

*/

#ifndef __CLIB_FIXALLOC_H
#define __CLIB_FIXALLOC_H

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef MEMORYLEAK
#include <clib/mlog.h>
#endif

template<int N, int B>
class fixed_allocator
{
    public:
        union Buffer {
            Buffer* next;
            char data[N];
        };
    void* allocate();
    void deallocate( void* );

    void* allocate( size_t size );
    void deallocate( void* size, size_t n );
    
#ifdef MEMORYLEAK
    fixed_allocator();
    ~fixed_allocator();
    void log_stuff(const string& detail);
#endif

protected:
    void* refill( void );
private:
    Buffer* freelist_;
#ifdef MEMORYLEAK
    int buffers;
    int requests;
    int max_requests;
#endif
};

#ifdef MEMORYLEAK
template<int N, int B>
fixed_allocator<N,B>::fixed_allocator()
{
    freelist_ = NULL;
    buffers = 0;
    requests = 0;
    max_requests = 0;
};

template<int N, int B>
fixed_allocator<N,B>::~fixed_allocator()
{
    log_stuff("destructor");
}

template<int N, int B>
void fixed_allocator<N,B>::log_stuff(const string& detail)
{
    if (mlog.is_open())
        mlog << "fixed_allocator[" << detail << "]: " << buffers << " Buffer with " << sizeof( Buffer[B] ) <<
             " Bytes allocated [" << requests << " Requests of " << max_requests << "]" << endl;

	if (llog.is_open())
        llog << buffers << ";" << sizeof( Buffer[B] ) << ";" << requests << ";" << max_requests << ";";
}
#endif

template<int N, int B>
void* fixed_allocator<N,B>::allocate()
{
#ifdef MEMORYLEAK
    requests++;
    if (max_requests < requests)
        max_requests = requests;
#endif

    Buffer* p = freelist_;
    if (p != NULL)
    {
        freelist_ = p->next;
        return p;
    }
    else
    {
        return refill();
    }
}

template<int N, int B>
void* fixed_allocator<N,B>::refill()
{
    size_t nbytes = sizeof( Buffer[B] );
    
    Buffer* morebuf = static_cast<Buffer*>( ::operator new(nbytes) );

#ifdef MEMORYLEAK
    buffers++;
#endif
    
    Buffer* walk = morebuf + 1;
    int count = B-2;
    while (count--)
    {
        Buffer* next = walk+1;
        walk->next = next;
        walk++;
    }
    walk->next = NULL;
    freelist_ = morebuf+1;
    return morebuf;
}

template<int N, int B>
void fixed_allocator<N,B>::deallocate( void* vp )
{
#ifdef MEMORYLEAK
    requests--;
#endif

    Buffer* buf = static_cast<Buffer*>(vp);
    buf->next = freelist_;
    freelist_ = buf;
}

template<int N, int B>
void* fixed_allocator<N,B>::allocate( size_t size ) 
{
    assert( size == B );
    if (size == B)
        return allocate();
    else
        return ::operator new(size);
}

template<int N, int B>
void fixed_allocator<N,B>::deallocate( void* vp, size_t size )
{
    assert( size == B );
    if (size == B)
        deallocate(vp);
    else
        ::operator delete(vp);
}

#endif
