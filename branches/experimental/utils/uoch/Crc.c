/******************************************************************************\
* 
* 
*  Copyright (C) 2004 Daniel 'Necr0Potenc3' Cavalcanti
*  Copyright (C) 2000 Bruno 'Beosil' Heidelberger
* 
* 
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
* 
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
* 
* 	Jan 1st, 2005 -- stole this from Beosil's Ignition :)
*   NEW YEAR! YEAAAAAAAAAAAAAAH!
* 
\******************************************************************************/


#include <stdio.h>
#include "Crc.h"


/* contains the built CRC Table */
unsigned int CRCTable[256];
int TableBuilt = 0;


/******************************************************************************\
* 
* 	Construct a checksum instance from a given polynomial
* 
* 	PARAMETERS:
* 		unsigned int polynomial		Polynomial to use in checksum calculations
* 
* 	RETURNS:
* 		-none-
* 
\******************************************************************************/

void CCrc(unsigned int polynomial)
{
	int i = 0, j = 0;
	for(i = 0; i < 256; i++)
	{
		CRCTable[i] = Reflect(i, 8) << 24;

		for(j = 0; j < 8; j++)
			CRCTable[i] = (CRCTable[i] << 1) ^ ((CRCTable[i] & (1 << 31) ? polynomial : 0));

		CRCTable[i] = Reflect(CRCTable[i], 32);
	}
}

/******************************************************************************\
* 
* 	Mirror a number of bits in a value
* 
* 	PARAMETERS:
* 		unsigned int source		Source value
* 		int c					Number of bits to mirror
* 
* 	RETURNS:
* 		unsigned int			Mirrored value
* 
\******************************************************************************/

unsigned int Reflect(unsigned int source, int c) 
{
	unsigned int value = 0;

	int i;
	for(i = 1; i < (c + 1); i++)
	{
		if(source & 0x1)
		{
			value |= 1 << (c - i);
		}

		source >>= 1;
	}

	return value;
} 



unsigned int CalculateCRC32(unsigned char *Buf, unsigned int Len)
{
	unsigned int i = 0, crc = 0xffffffff;

	if(!TableBuilt)
	{
		CCrc(CRC_DEFAULT_POLYNOMIAL);
		TableBuilt = 1;
	}

	for(i = 0; i < Len; i++)
		crc = (crc >> 8) ^ CRCTable[(crc & 0xff) ^ Buf[i]];

	return crc ^ 0xffffffff;
}
