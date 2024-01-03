/** @file
 *
 * @par History
 * - 2005/06/20 Shinigami: extended new/delete operator with linux malloc_usable_size
 *                         added memory block classification to analyze a little bit (needs defined
 * MEMORYLEAK)
 * - 2006/10/06 Shinigami: Linux needs malloc.h for malloc_usable_size
 *                         FreeBSD 6.1 doesn't know malloc_usable_size (FreeBSD 7.0 will support it)
 * - 2007/04/28 Shinigami: polcore().internal information will be logged in excel-friendly format
 * too (leak.log)
 */


#include "opnew.h"
#include "logfacility.h"
#include "pol_global_config.h"

#ifdef _WIN32
#include <malloc.h>
#endif

#ifdef MEMORYLEAK
#include <time.h>

#define MEMORYLOGBLOCKS 0
#endif

namespace Pol
{
namespace Clib
{
static unsigned int bytes_allocated = 0;
static unsigned int last_bytes_allocated = 0;
static unsigned int blocks_allocated = 0;
static unsigned int last_blocks_allocated = 0;

#ifdef MEMORYLEAK
static unsigned int requested_8 = 0;
static unsigned int requested_16 = 0;
static unsigned int requested_32 = 0;
static unsigned int requested_64 = 0;
static unsigned int requested_128 = 0;
static unsigned int requested_512 = 0;
static unsigned int requested_1024 = 0;
static unsigned int requested_2048 = 0;
static unsigned int requested_4096 = 0;
static unsigned int requested_8192 = 0;
static unsigned int requested_16384 = 0;
static unsigned int requested_65536 = 0;
static unsigned int requested_131072 = 0;
static unsigned int requested_524288 = 0;
static unsigned int requested_1048576 = 0;
static unsigned int requested_16777216 = 0;
static unsigned int requested_huge = 0;

static unsigned int used_8 = 0;
static unsigned int used_16 = 0;
static unsigned int used_32 = 0;
static unsigned int used_64 = 0;
static unsigned int used_128 = 0;
static unsigned int used_512 = 0;
static unsigned int used_1024 = 0;
static unsigned int used_2048 = 0;
static unsigned int used_4096 = 0;
static unsigned int used_8192 = 0;
static unsigned int used_16384 = 0;
static unsigned int used_65536 = 0;
static unsigned int used_131072 = 0;
static unsigned int used_524288 = 0;
static unsigned int used_1048576 = 0;
static unsigned int used_16777216 = 0;
static unsigned int used_huge = 0;

static unsigned int allocated_8 = 0;
static unsigned int allocated_16 = 0;
static unsigned int allocated_32 = 0;
static unsigned int allocated_64 = 0;
static unsigned int allocated_128 = 0;
static unsigned int allocated_512 = 0;
static unsigned int allocated_1024 = 0;
static unsigned int allocated_2048 = 0;
static unsigned int allocated_4096 = 0;
static unsigned int allocated_8192 = 0;
static unsigned int allocated_16384 = 0;
static unsigned int allocated_65536 = 0;
static unsigned int allocated_131072 = 0;
static unsigned int allocated_524288 = 0;
static unsigned int allocated_1048576 = 0;
static unsigned int allocated_16777216 = 0;
static unsigned int allocated_huge = 0;
#endif

#if 0
#ifndef NDEBUG
#ifdef _WIN32
  static unsigned int n_allocs;
  static unsigned int n_frees;
  void* operator new( size_t len )
  {
    void *vp = malloc( len );
    if (vp)
    {
#ifdef _WIN32
    bytes_allocated += _msize( vp );
#elif __linux__
    bytes_allocated += malloc_usable_size( vp );
#endif
    ++blocks_allocated;
    return vp;
    }
    else
    {
    throw std::runtime_error( "Out of memory" );
    }
  }

  void operator delete( void *ptr )
  {
    if (ptr)
    {
#ifdef _WIN32
    bytes_allocated -= _msize( ptr );
#elif __linux__
    bytes_allocated -= malloc_usable_size( ptr );
#endif
    --blocks_allocated;
    free( ptr );
    }
  }
#endif
#endif
#endif

#ifdef MEMORYLEAK
void* do_new( size_t len )
{
  void* vp = malloc( len );
  if ( vp )
  {
#ifdef _WIN32
    len = _msize( vp );
#elif __linux__
    len = malloc_usable_size( vp );
#else
    len = 0;
#endif

    if ( len < 8 )
    {
      requested_8++;
      used_8++;
      allocated_8 += len;
    }
    else if ( len < 16 )
    {
      requested_16++;
      used_16++;
      allocated_16 += len;
    }
    else if ( len < 32 )
    {
      requested_32++;
      used_32++;
      allocated_32 += len;
    }
    else if ( len < 64 )
    {
      requested_64++;
      used_64++;
      allocated_64 += len;
    }
    else if ( len < 128 )
    {
      requested_128++;
      used_128++;
      allocated_128 += len;
    }
    else if ( len < 512 )
    {
      requested_512++;
      used_512++;
      allocated_512 += len;
    }
    else if ( len < 1024 )
    {
      requested_1024++;
      used_1024++;
      allocated_1024 += len;
    }
    else if ( len < 2048 )
    {
      requested_2048++;
      used_2048++;
      allocated_2048 += len;
    }
    else if ( len < 4096 )
    {
      requested_4096++;
      used_4096++;
      allocated_4096 += len;
    }
    else if ( len < 8192 )
    {
      requested_8192++;
      used_8192++;
      allocated_8192 += len;
    }
    else if ( len < 16384 )
    {
      requested_16384++;
      used_16384++;
      allocated_16384 += len;
    }
    else if ( len < 65536 )
    {
      requested_65536++;
      used_65536++;
      allocated_65536 += len;

#if MEMORYLOGBLOCKS == 1
#ifdef _WIN32
      LEAKLOG.Format( "New 64 KB {:x} {}\n" ) << vp << len;
#else
      LEAKLOG.Format( "New 64 KB {:x} {} {} {} {} {} {} {} {} {}\n" )
          << vp << len << __builtin_return_address( 0 ) << __builtin_return_address( 1 )
          << __builtin_return_address( 2 ) << __builtin_return_address( 3 )
          << __builtin_return_address( 4 ) << __builtin_return_address( 5 )
          << __builtin_return_address( 6 ) << __builtin_return_address( 7 );
#endif
#endif
    }
    else if ( len < 131072 )
    {
      requested_131072++;
      used_131072++;
      allocated_131072 += len;
    }
    else if ( len < 524288 )
    {
      requested_524288++;
      used_524288++;
      allocated_524288 += len;

#if MEMORYLOGBLOCKS == 1
#ifdef _WIN32
      LEAKLOG.Format( "New 0.5 MB {:x} {}\n" ) << vp << len;
#else
      LEAKLOG.Format( "New 0.5 M {:x} {} {} {} {} {} {} {} {} {}\n" )
          << vp << len << __builtin_return_address( 0 ) << __builtin_return_address( 1 )
          << __builtin_return_address( 2 ) << __builtin_return_address( 3 )
          << __builtin_return_address( 4 ) << __builtin_return_address( 5 )
          << __builtin_return_address( 6 ) << __builtin_return_address( 7 );
#endif
#endif
    }
    else if ( len < 1048576 )
    {
      requested_1048576++;
      used_1048576++;
      allocated_1048576 += len;
    }
    else if ( len < 16777216 )
    {
      requested_16777216++;
      used_16777216++;
      allocated_16777216 += len;
    }
    else
    {
      requested_huge++;
      used_huge++;
      allocated_huge += len;

#if MEMORYLOGBLOCKS == 1
#ifdef _WIN32
      LEAKLOG.Format( "New huge {:x} {}\n" ) << vp << len;
#else
      LEAKLOG.Format( "New huge {:x} {} {} {} {} {} {} {} {} {}\n" )
          << vp << len << __builtin_return_address( 0 ) << __builtin_return_address( 1 )
          << __builtin_return_address( 2 ) << __builtin_return_address( 3 )
          << __builtin_return_address( 4 ) << __builtin_return_address( 5 )
          << __builtin_return_address( 6 ) << __builtin_return_address( 7 );
#endif
#endif
    }

    bytes_allocated += len;
    ++blocks_allocated;
    return vp;
  }
  else
  {
    throw std::runtime_error( "Out of memory" );
  }
}


void do_delete( void* ptr )
{
  if ( ptr )
  {
#ifdef _WIN32
    size_t len = _msize( ptr );
#elif __linux__
    size_t len = malloc_usable_size( ptr );
#else
    size_t len = 0;
#endif

    if ( len < 8 )
    {
      used_8--;
      allocated_8 -= len;
    }
    else if ( len < 16 )
    {
      used_16--;
      allocated_16 -= len;
    }
    else if ( len < 32 )
    {
      used_32--;
      allocated_32 -= len;
    }
    else if ( len < 64 )
    {
      used_64--;
      allocated_64 -= len;
    }
    else if ( len < 128 )
    {
      used_128--;
      allocated_128 -= len;
    }
    else if ( len < 512 )
    {
      used_512--;
      allocated_512 -= len;
    }
    else if ( len < 1024 )
    {
      used_1024--;
      allocated_1024 -= len;
    }
    else if ( len < 2048 )
    {
      used_2048--;
      allocated_2048 -= len;
    }
    else if ( len < 4096 )
    {
      used_4096--;
      allocated_4096 -= len;
    }
    else if ( len < 8192 )
    {
      used_8192--;
      allocated_8192 -= len;
    }
    else if ( len < 16384 )
    {
      used_16384--;
      allocated_16384 -= len;
    }
    else if ( len < 65536 )
    {
      used_65536--;
      allocated_65536 -= len;

#if MEMORYLOGBLOCKS == 1
#ifdef _WIN32
      LEAKLOG.Format( "Delete 64 KB {:x} {}\n" ) << ptr << len;
#else
      LEAKLOG.Format( "Delete 64 KB {:x} {} {} {} {} {} {} {} {} {}\n" )
          << ptr << len << __builtin_return_address( 0 ) << __builtin_return_address( 1 )
          << __builtin_return_address( 2 ) << __builtin_return_address( 3 )
          << __builtin_return_address( 4 ) << __builtin_return_address( 5 )
          << __builtin_return_address( 6 ) << __builtin_return_address( 7 );
#endif
#endif
    }
    else if ( len < 131072 )
    {
      used_131072--;
      allocated_131072 -= len;
    }
    else if ( len < 524288 )
    {
      used_524288--;
      allocated_524288 -= len;

#if MEMORYLOGBLOCKS == 1
#ifdef _WIN32
      LEAKLOG.Format( "Delete 0.5 MB {:x} {}\n" ) << ptr << len;
#else
      LEAKLOG.Format( "Delete 0.5 MB {:x} {} {} {} {} {} {} {} {} {}\n" )
          << ptr << len << __builtin_return_address( 0 ) << __builtin_return_address( 1 )
          << __builtin_return_address( 2 ) << __builtin_return_address( 3 )
          << __builtin_return_address( 4 ) << __builtin_return_address( 5 )
          << __builtin_return_address( 6 ) << __builtin_return_address( 7 );
#endif
#endif
    }
    else if ( len < 1048576 )
    {
      used_1048576--;
      allocated_1048576 -= len;
    }
    else if ( len < 16777216 )
    {
      used_16777216--;
      allocated_16777216 -= len;
    }
    else
    {
      used_huge--;
      allocated_huge -= len;

#if MEMORYLOGBLOCKS == 1
#ifdef _WIN32
      LEAKLOG.Format( "Delete huge {:x} {}\n" ) << ptr << len;
#else
      LEAKLOG.Format( "Delete huge {:x} {} {} {} {} {} {} {} {} {}\n" )
          << ptr << len << __builtin_return_address( 0 ) << __builtin_return_address( 1 )
          << __builtin_return_address( 2 ) << __builtin_return_address( 3 )
          << __builtin_return_address( 4 ) << __builtin_return_address( 5 )
          << __builtin_return_address( 6 ) << __builtin_return_address( 7 );
#endif
#endif
    }

    bytes_allocated -= len;
    --blocks_allocated;
    free( ptr );
  }
}

#endif

void PrintHeapData()
{
  int delta_bytes = bytes_allocated - last_bytes_allocated;
  int delta_blocks = blocks_allocated - last_blocks_allocated;
  last_bytes_allocated = bytes_allocated;
  last_blocks_allocated = blocks_allocated;

#ifndef MEMORYLEAK
  INFO_PRINT2( "OpNewHeap: allocated {} blocks, {} bytes\nOpNewHeap: delta {} blocks, {} bytes",
               blocks_allocated, bytes_allocated, delta_blocks, delta_bytes );
#else
  DEBUGLOG << "Heap (whole):  " << blocks_allocated << " blocks with " << bytes_allocated
           << " Bytes\n"
           << "Heap (delta):  " << delta_blocks << " blocks with " << delta_bytes << " Bytes\n"
           << "Heap (blocks): " << used_8 << " of " << requested_8 << " blocks of 8 Byte ("
           << allocated_8 << ")\n"
           << "Heap (blocks): " << used_16 << " of " << requested_16 << " blocks of 16 Byte ("
           << allocated_16 << ")\n"
           << "Heap (blocks): " << used_32 << " of " << requested_32 << " blocks of 32 Byte ("
           << allocated_32 << ")\n"
           << "Heap (blocks): " << used_64 << " of " << requested_64 << " blocks of 64 Byte ("
           << allocated_64 << ")\n"
           << "Heap (blocks): " << used_128 << " of " << requested_128 << " blocks of 128 Byte ("
           << allocated_128 << ")\n"
           << "Heap (blocks): " << used_512 << " of " << requested_512 << " blocks of 512 Byte ("
           << allocated_512 << ")\n"
           << "Heap (blocks): " << used_1024 << " of " << requested_1024 << " blocks of 1 KByte ("
           << allocated_1024 << ")\n"
           << "Heap (blocks): " << used_2048 << " of " << requested_2048 << " blocks of 2 KByte ("
           << allocated_2048 << ")\n"
           << "Heap (blocks): " << used_4096 << " of " << requested_4096 << " blocks of 4 KByte ("
           << allocated_4096 << ")\n"
           << "Heap (blocks): " << used_8192 << " of " << requested_8192 << " blocks of 8 KByte ("
           << allocated_8192 << ")\n"
           << "Heap (blocks): " << used_16384 << " of " << requested_16384
           << " blocks of 16 KByte (" << allocated_16384 << ")\n"
           << "Heap (blocks): " << used_65536 << " of " << requested_65536
           << " blocks of 64 KByte (" << allocated_65536 << ")\n"
           << "Heap (blocks): " << used_131072 << " of " << requested_131072
           << " blocks of 128 KByte (" << allocated_131072 << ")\n"
           << "Heap (blocks): " << used_524288 << " of " << requested_524288
           << " blocks of 0.5 MByte (" << allocated_524288 << ")\n"
           << "Heap (blocks): " << used_1048576 << " of " << requested_1048576
           << " blocks of 1 MByte (" << allocated_1048576 << ")\n"
           << "Heap (blocks): " << used_16777216 << " of " << requested_16777216
           << " blocks of 16 MByte (" << allocated_16777216 << ")\n"
           << "Heap (blocks): " << used_huge << " of " << requested_huge << " huge blocks ("
           << allocated_huge << ")\n";


  LEAKLOG << blocks_allocated << "; " << bytes_allocated << "; " << delta_blocks << ";"
          << delta_bytes << ";" << used_8 << ";" << requested_8 << ";" << allocated_8 << ";"
          << used_16 << ";" << requested_16 << ";" << allocated_16 << ";" << used_32 << ";"
          << requested_32 << ";" << allocated_32 << ";" << used_64 << ";" << requested_64 << ";"
          << allocated_64 << ";" << used_128 << ";" << requested_128 << ";" << allocated_128 << ";"
          << used_512 << ";" << requested_512 << ";" << allocated_512 << ";" << used_1024 << ";"
          << requested_1024 << ";" << allocated_1024 << ";" << used_2048 << ";" << requested_2048
          << ";" << allocated_2048 << ";" << used_4096 << ";" << requested_4096 << ";"
          << allocated_4096 << ";" << used_8192 << ";" << requested_8192 << ";" << allocated_8192
          << ";" << used_16384 << ";" << requested_16384 << ";" << allocated_16384 << ";"
          << used_65536 << ";" << requested_65536 << ";" << allocated_65536 << ";" << used_131072
          << ";" << requested_131072 << ";" << allocated_131072 << ";" << used_524288 << ";"
          << requested_524288 << ";" << allocated_524288 << ";" << used_1048576 << ";"
          << requested_1048576 << ";" << allocated_1048576 << ";" << used_16777216 << ";"
          << requested_16777216 << ";" << allocated_16777216 << ";" << used_huge << ";"
          << requested_huge << ";" << allocated_huge << "\n";
#endif
}
}  // namespace Clib
}  // namespace Pol

#ifdef MEMORYLEAK
void* operator new( size_t len )
{
  return Pol::Clib::do_new( len );
}

void* operator new[]( size_t len )
{
  return Pol::Clib::do_new( len );
}

void operator delete( void* ptr )
{
  Pol::Clib::do_delete( ptr );
}

void operator delete[]( void* ptr )
{
  Pol::Clib::do_delete( ptr );
}
#endif
