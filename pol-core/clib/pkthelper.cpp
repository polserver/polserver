/** @file
 *
 * @par History
 */


#include <climits>
#include "pkthelper.h"
namespace Pol
{
namespace Clib
{
void WritetoCharBuffer( char* buffer, u32 x, unsigned int* offset )
{
  buffer[( *offset )++] = x & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT ) & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT * 2 ) & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT * 3 ) & UCHAR_MAX;
}

void WritetoCharBuffer( char* buffer, s32 x, unsigned int* offset )
{
  buffer[( *offset )++] = x & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT ) & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT * 2 ) & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT * 3 ) & UCHAR_MAX;
}

void WritetoCharBuffer( char* buffer, u16 x, unsigned int* offset )
{
  buffer[( *offset )++] = x & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT ) & UCHAR_MAX;
}

void WritetoCharBuffer( char* buffer, s16 x, unsigned int* offset )
{
  buffer[( *offset )++] = x & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT ) & UCHAR_MAX;
}

void WritetoCharBuffer( char* buffer, u8 x, unsigned int* offset )
{
  buffer[( *offset )++] = x;
}

void WritetoCharBuffer( char* buffer, s8 x, unsigned int* offset )
{
  buffer[( *offset )++] = x;
}

void WritetoCharBufferFlipped( char* buffer, u32 x, unsigned int* offset )
{
  buffer[( *offset )++] = ( x >> CHAR_BIT * 3 ) & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT * 2 ) & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT ) & UCHAR_MAX;
  buffer[( *offset )++] = x & UCHAR_MAX;
}

void WritetoCharBufferFlipped( char* buffer, s32 x, unsigned int* offset )
{
  buffer[( *offset )++] = ( x >> CHAR_BIT * 3 ) & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT * 2 ) & UCHAR_MAX;
  buffer[( *offset )++] = ( x >> CHAR_BIT ) & UCHAR_MAX;
  buffer[( *offset )++] = x & UCHAR_MAX;
}

void WritetoCharBufferFlipped( char* buffer, u16 x, unsigned int* offset )
{
  buffer[( *offset )++] = ( x >> CHAR_BIT ) & UCHAR_MAX;
  buffer[( *offset )++] = x & UCHAR_MAX;
}

void WritetoCharBufferFlipped( char* buffer, s16 x, unsigned int* offset )
{
  buffer[( *offset )++] = ( x >> CHAR_BIT ) & UCHAR_MAX;
  buffer[( *offset )++] = x & UCHAR_MAX;
}
}
}