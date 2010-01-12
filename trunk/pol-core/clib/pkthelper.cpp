/*
History
=======

Notes
=======

*/

#include <limits>
#include "pkthelper.h"

void WritetoCharBuffer(char *buffer, u32 x, unsigned int *offset )
{
	buffer[(*offset)++] = x & UCHAR_MAX;
	buffer[(*offset)++] = (x >> CHAR_BIT) & UCHAR_MAX;
	buffer[(*offset)++] = (x >> CHAR_BIT*2) & UCHAR_MAX;
	buffer[(*offset)++] = (x >> CHAR_BIT*3) & UCHAR_MAX;
}

void WritetoCharBuffer(char *buffer, s32 x, unsigned int *offset )
{
	buffer[(*offset)++] = x & UCHAR_MAX;
	buffer[(*offset)++] = (x >> CHAR_BIT) & UCHAR_MAX;
	buffer[(*offset)++] = (x >> CHAR_BIT*2) & UCHAR_MAX;
	buffer[(*offset)++] = (x >> CHAR_BIT*3) & UCHAR_MAX;
}

void WritetoCharBuffer(char *buffer, u16 x, unsigned int *offset )
{
	buffer[(*offset)++] = x & UCHAR_MAX;
	buffer[(*offset)++] = (x >> CHAR_BIT) & UCHAR_MAX;
}

void WritetoCharBuffer(char *buffer, s16 x, unsigned int *offset )
{
	buffer[(*offset)++] = x & UCHAR_MAX;
	buffer[(*offset)++] = (x >> CHAR_BIT) & UCHAR_MAX;
}

void WritetoCharBuffer(char *buffer, u8 x, unsigned int *offset )
{
	buffer[(*offset)++] = x;
}

void WritetoCharBuffer(char *buffer, s8 x, unsigned int *offset )
{
	buffer[(*offset)++] = x;
}