/** @file
 *
 * @par History
 * - 2006/09/23 Shinigami: printf 64bit constants added
 *
 * @note ATTENTION
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */


#ifndef __CLIB_RAWTYPES_H
#define __CLIB_RAWTYPES_H

#define __PACKED__

/* The PACK_NEEDED define goes after "struct { }" definitions */
#ifdef __GNUC__
#define POSTSTRUCT_PACK __attribute__( ( __packed__ ) )
#else
#define POSTSTRUCT_PACK
#endif


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#ifdef _WIN32
typedef unsigned __int64 u64;
typedef signed __int64 s64;
#define OUT64 "I64"
#else
typedef unsigned long long u64;
typedef signed long long s64;
#define OUT64 "ll"
#endif


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
#ifdef __cplusplus

#if defined( _WIN32 )
static_assert( sizeof( wchar_t ) == 2, "size missmatch" );
#elif defined( __GNUC__ )
static_assert( sizeof( wchar_t ) == 4, "size missmatch" );
#else
#error unknown size for wchar_t
#endif

#endif

#endif /* __STYPE_H */
