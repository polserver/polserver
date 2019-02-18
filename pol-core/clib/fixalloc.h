/** @file
 *
 * @par History
 * - 2005/06/20 Shinigami: added log_stuff and everything needed to analyze a little bit (needs
 * defined MEMORYLEAK)
 * - 2006/10/06 Shinigami: malloc.h -> stdlib.h
 * - 2007/04/28 Shinigami: polcore().internal information will be logged in excel-friendly format
 * too (leak.log)
 *
 * @note ATTENTION
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */


#ifndef __CLIB_FIXALLOC_H
#define __CLIB_FIXALLOC_H

#include "pol_global_config.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef MEMORYLEAK
#include "logfacility.h"
#endif
namespace Pol
{
namespace Clib
{
template <size_t N, size_t B>
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
  void log_stuff( const std::string& detail );
#endif

  size_t memsize = 0;

protected:
  void* refill( void );

private:
  Buffer* freelist_ = nullptr;
#ifdef MEMORYLEAK
  int buffers;
  int requests;
  int max_requests;
#endif
};

#ifdef MEMORYLEAK
template <size_t N, size_t B>
fixed_allocator<N, B>::fixed_allocator()
{
  freelist_ = nullptr;
  buffers = 0;
  requests = 0;
  max_requests = 0;
};

template <size_t N, size_t B>
fixed_allocator<N, B>::~fixed_allocator()
{
  log_stuff( "destructor" );
}

template <size_t N, size_t B>
void fixed_allocator<N, B>::log_stuff( const std::string& detail )
{
  DEBUGLOG << "fixed_allocator[" << detail << "]: " << buffers << " Buffer with "
           << sizeof( Buffer[B] ) << " Bytes allocated [" << requests << " Requests of "
           << max_requests << "]\n";

  LEAKLOG << buffers << ";" << sizeof( Buffer[B] ) << ";" << requests << ";" << max_requests << ";";
}
#endif

template <size_t N, size_t B>
void* fixed_allocator<N, B>::allocate()
{
#ifdef LEAK_DEBUG
  return ::operator new( N );
#endif
#ifdef MEMORYLEAK
  requests++;
  if ( max_requests < requests )
    max_requests = requests;
#endif

  Buffer* p = freelist_;
  if ( p != nullptr )
  {
    freelist_ = p->next;
    return p;
  }
  else
  {
    return refill();
  }
}

template <size_t N, size_t B>
void* fixed_allocator<N, B>::refill()
{
  size_t nbytes = sizeof( Buffer[B] );

  Buffer* morebuf = static_cast<Buffer*>( ::operator new( nbytes ) );

#ifdef MEMORYLEAK
  buffers++;
#endif
  memsize += nbytes;
  Buffer* walk = morebuf + 1;
  int count = B - 2;
  while ( count-- )
  {
    Buffer* next = walk + 1;
    walk->next = next;
    walk++;
  }
  walk->next = nullptr;
  freelist_ = morebuf + 1;
  return morebuf;
}

template <size_t N, size_t B>
void fixed_allocator<N, B>::deallocate( void* vp )
{
#ifdef LEAK_DEBUG
  return ::operator delete( vp );
#endif
#ifdef MEMORYLEAK
  requests--;
#endif

  Buffer* buf = static_cast<Buffer*>( vp );
  buf->next = freelist_;
  freelist_ = buf;
}

template <size_t N, size_t B>
void* fixed_allocator<N, B>::allocate( size_t size )
{
#ifdef LEAK_DEBUG
  return ::operator new( size );
#endif
  assert( size == B );
  if ( size == B )
    return allocate();
  else
    return ::operator new( size );
}

template <size_t N, size_t B>
void fixed_allocator<N, B>::deallocate( void* vp, size_t size )
{
#ifdef LEAK_DEBUG
  return ::operator delete( vp );
#endif
  assert( size == B );
  if ( size == B )
    deallocate( vp );
  else
    ::operator delete( vp );
}
}  // namespace Clib
}  // namespace Pol
#endif
