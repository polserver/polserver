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

#pragma once

#include "pol_global_config.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>

#ifdef MEMORYLEAK
#include "logfacility.h"
#endif

namespace Pol::Clib
{
template <typename T, size_t B>
class fixed_allocator
{
  static_assert( B > 1, "batch size must be greater than 1" );

  static constexpr size_t N = sizeof( T );
  static constexpr size_t Align = alignof( T );

  static_assert( Align >= alignof( void* ), "minimum alignment not reached, illegal type" );

  union alignas( Align ) Buffer
  {
    Buffer* next;
    char data[N];
  };

  static constexpr size_t ChunkBytes = sizeof( Buffer[B] );

public:
  fixed_allocator() = default;
  fixed_allocator( const fixed_allocator& ) = delete;
  fixed_allocator& operator=( const fixed_allocator& ) = delete;
  fixed_allocator( fixed_allocator&& ) = delete;
  fixed_allocator& operator=( fixed_allocator&& ) = delete;

  [[nodiscard]] void* allocate();
  void deallocate( void* vp ) noexcept;

  [[nodiscard]] void* allocate( size_t size );
  void deallocate( void* vp, size_t size ) noexcept;

#ifdef MEMORYLEAK
  ~fixed_allocator();
  void log_stuff( const std::string& detail );
#else
  ~fixed_allocator() = default;
#endif

  size_t memsize = 0;

private:
  void* refill();

  Buffer* freelist_ = nullptr;
#ifdef MEMORYLEAK
  int buffers_ = 0;
  int requests_ = 0;
  int max_requests_ = 0;
#endif
};

#ifdef MEMORYLEAK
template <typename T, size_t B>
fixed_allocator<T, B>::~fixed_allocator()
{
  log_stuff( "destructor" );
}

template <typename T, size_t B>
void fixed_allocator<T, B>::log_stuff( const std::string& detail )
{
  DEBUGLOGLN( "fixed_allocator[{}]: {} Buffer with {} Bytes allocated [{} Requests of {}]", detail,
              buffers_, ChunkBytes, requests_, max_requests_ );

  LEAKLOG( "{};{};{};{};", buffers_, ChunkBytes, requests_, max_requests_ );
}
#endif

template <typename T, size_t B>
void* fixed_allocator<T, B>::allocate()
{
#ifdef LEAK_DEBUG
  return ::operator new( N );
#else
#ifdef MEMORYLEAK
  ++requests_;
  if ( max_requests_ < requests_ )
    max_requests_ = requests_;
#endif

  Buffer* p = freelist_;
  if ( p != nullptr )
  {
    freelist_ = p->next;
    return p;
  }

  return refill();
#endif
}

template <typename T, size_t B>
void* fixed_allocator<T, B>::refill()
{
  Buffer* morebuf = static_cast<Buffer*>( ::operator new( ChunkBytes ) );

#ifdef MEMORYLEAK
  ++buffers_;
#endif
  memsize += ChunkBytes;

  for ( size_t i = 1; i + 1 < B; ++i )
    morebuf[i].next = &morebuf[i + 1];
  morebuf[B - 1].next = nullptr;

  freelist_ = &morebuf[1];
  return morebuf;
}

template <typename T, size_t B>
void fixed_allocator<T, B>::deallocate( void* vp ) noexcept
{
#ifdef LEAK_DEBUG
  return ::operator delete( vp );
#else
#ifdef MEMORYLEAK
  --requests_;
#endif

  Buffer* buf = static_cast<Buffer*>( vp );
  buf->next = freelist_;
  freelist_ = buf;
#endif
}

template <typename T, size_t B>
void* fixed_allocator<T, B>::allocate( size_t size )
{
#ifdef LEAK_DEBUG
  return ::operator new( size );
#else
  assert( size == N );
  if ( size == N )
    return allocate();
  return ::operator new( size );
#endif
}

template <typename T, size_t B>
void fixed_allocator<T, B>::deallocate( void* vp, size_t size ) noexcept
{
#ifdef LEAK_DEBUG
  return ::operator delete( vp );
#else
  assert( size == N );
  if ( size == N )
    deallocate( vp );
  else
    ::operator delete( vp );
#endif
}
}  // namespace Pol::Clib
