/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: printf 64bit constants added
 *
 * @note ATTENTION
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */
#pragma once

#include <cstdint>
#define __PACKED__

/* The PACK_NEEDED define goes after "struct { }" definitions */
#ifdef __GNUC__
#define POSTSTRUCT_PACK __attribute__( ( __packed__ ) )
#else
#define POSTSTRUCT_PACK
#endif


using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

using s8 = signed char;
using s16 = short;
using s32 = int;

using u64 = uint64_t;
using s64 = int64_t;

/* these fail to compile if lengths of U8, U16, or U32 are incorrect. */
static_assert( sizeof( u8 ) == 1, "size missmatch" );
static_assert( sizeof( u16 ) == 2, "size missmatch" );
static_assert( sizeof( u32 ) == 4, "size missmatch" );
static_assert( sizeof( u64 ) == 8, "size missmatch" );
static_assert( sizeof( s8 ) == 1, "size missmatch" );
static_assert( sizeof( s16 ) == 2, "size missmatch" );
static_assert( sizeof( s32 ) == 4, "size missmatch" );
static_assert( sizeof( s64 ) == 8, "size missmatch" );

// here's where Win32 land and Linux land differ:
#if defined( _WIN32 )
static_assert( sizeof( wchar_t ) == 2, "size missmatch" );
#elif defined( __GNUC__ )
static_assert( sizeof( wchar_t ) == 4, "size missmatch" );
#else
#error unknown size for wchar_t
#endif

constexpr auto operator""_u8( unsigned long long v )
{
  return static_cast<u8>( v );
}
constexpr auto operator""_s8( unsigned long long v )
{
  return static_cast<s8>( v );
}
constexpr auto operator""_u16( unsigned long long v )
{
  return static_cast<u16>( v );
}
constexpr auto operator""_s16( unsigned long long v )
{
  return static_cast<s16>( v );
}
constexpr auto operator""_u32( unsigned long long v )
{
  return static_cast<u32>( v );
}
constexpr auto operator""_s32( unsigned long long v )
{
  return static_cast<s32>( v );
}
constexpr auto operator""_u64( unsigned long long v )
{
  return static_cast<u64>( v );
}
constexpr auto operator""_s64( unsigned long long v )
{
  return static_cast<s64>( v );
}
