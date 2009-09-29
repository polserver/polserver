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


#ifndef _CRC_H_INCLUDED
#define _CRC_H_INCLUDED

/* default polynomial used for the table generation */
#define CRC_DEFAULT_POLYNOMIAL	0x04c11db7

/* builds the CRC32 table */
void CCrc(unsigned int polynomial);

/* Mirror a number of bits in a value */
unsigned int Reflect(unsigned int source, int c);

/* calculate the checksum of a given buffer (CRC32) */
unsigned int CalculateCRC32(unsigned char *Buf, unsigned int Len);

#endif
