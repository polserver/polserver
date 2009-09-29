/******************************************************************************\
* 
* 
*  Copyright (C) 2004 Daniel 'Necr0Potenc3' Cavalcanti
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
* 	Dec 28th, 2004 -- this module contains an assembly of nifty functions ;)
*   Jan 21th, 2005 -- added the functions GetFunctionUnsafe and FindCave
* 
\******************************************************************************/

#include <windows.h>
#include "CrackTools.h"


/* given a position inside a function, track the beginning of the function and the address */
/* slower than GetFunctionRef haha */
int GetFunctionByRef(BYTE *Buf, DWORD BufSize, DWORD PosInFunction)
{
	DWORD i = 0;

	/* go down from where we are in the buffer till the beginning */
	for(i = PosInFunction; i > 0; i--)
	{
		/* search for a CALL <this position> */
		if(GetFunctionRef(Buf, BufSize, i, 1) != -1)
			return i;
	}

	return -1;
}

/*
* compilers leave "filling" bytes between functions sometimes (dont ask me why)
* it's usually NOP for M$ compilers and BPX for other compilers
* try to find the usual opcodes in the beginning of a function
* with a couple of filling bytes behind
* usually <fill bytes> and one of the following:
* PUSH <32 bit reg> (5X) or
* SUB ESP, <long value> (83 EC) or SUP ESP, <short value (81 EC) or
* MOV <32  bit reg>, <some value in esp> (8B)
* It's not very safe (thus the name Unsafe) but it works :)
* this should be used in case GetFunctionByRef fails
* RTD: search other fillbytes other than NOPs
*/
int GetFunctionUnsafe(BYTE *Buf, DWORD BufSize, DWORD PosInFunction, DWORD NOPCount)
{
	DWORD i = 0;
	BYTE *FillBuf = NULL;

	FillBuf = (BYTE*)malloc(NOPCount);

	/* SOOOOO shouldn't happen... lol */
	if(FillBuf == NULL)
		return -1;

	/* fill "buffy" with nops... rofl */
	for(i = 0; i < NOPCount; i++)
		FillBuf[i] = 0x90;

	/* go down from where we are in the buffer till the beginning */
	for(i = PosInFunction; i > 0; i--)
	{
		/* check if the bytes before this one match the fill bytes */
		if(!memcmp(Buf + i - NOPCount, FillBuf, NOPCount))
		{
			/* if they are, try to find what we usually find in the start of a function */
			if( (Buf[i] >= 0x50 && Buf[i]) <= 0x5F ||
				((Buf[i] == 0x83 || Buf[i] == 81) && Buf[i + 1] == 0xEC) ||
				(Buf[i] == 0x8B))
			{
				free(FillBuf);
				return i;
			}
		}
	}

	free(FillBuf);
	return -1;
}

/* finds a cave of CaveSize zeroes in Buf */
int FindCave(BYTE *Buf, DWORD BufSize, DWORD StartAt, DWORD CaveSize)
{
	DWORD i = 0;
	BYTE *Cave = NULL;

	Cave = (BYTE*)malloc(CaveSize);
	memset(Cave, 0, sizeof(Cave));

	for(i = StartAt; i < BufSize - CaveSize; i++)
	{
		if(!memcmp(Buf + i, Cave, CaveSize))
		{
			free(Cave);
			return i;
		}
	}


	return -1;
}


/* tries to find a "CALL func" in buf. returns -1 if not found */
/* VERY slow... reliable though */
int GetFunctionRef(BYTE *Buf, DWORD BufSize, DWORD Function, DWORD Which)
{
	DWORD Count = 0, Call = 0, i = 0;
	char Ref[5] = { 0xe8, 0x00, 0x00, 0x00, 0x00 };

	for(i = 0; i < BufSize - sizeof(Ref); i++)
    {
		/* if this doesn't even look like a call, skip */
		if(Buf[i] != 0xe8)
			continue;

		/* search for CALL (to-from-5) */
        Call = Function - i - 5;
		memcpy(Ref + 1, &Call, sizeof(DWORD));
		if(!memcmp(Buf + i, Ref, 5))
		{
			Count++;
			if(Count >= Which)
				return i;
		}
	}

	return -1;
}

/* searches for PUSH <offset of text> */
int GetTextRef(BYTE *Buf, DWORD BufSize, DWORD ImageBase, const char *Text, BOOL Exact, int Which)
{
	char PushText[5] = { 0x68, 0x00, 0x00, 0x00, 0x00 };
	unsigned int TextPos = 0, TextPush = 0;

	/* find the text's position, if Exact is true, it will include the null char in the search */
	TextPos = ImageBase + FleXSearch((BYTE*)Text, Buf, (DWORD)(strlen(Text) + Exact), BufSize, 0, 0x100, 1);
	if(TextPos == ImageBase - 1)
		return -2;

	/* find PUSH <offset of text> */
	memcpy(PushText + 1, &TextPos, sizeof(TextPos));
	TextPush = FleXSearch(PushText, Buf, sizeof(PushText), BufSize, 0, 0x100, 1);

	/* returns -1 if not found */
	return TextPush;
}

int FleXSearch(BYTE *Src, BYTE *Buf, DWORD SrcSize, DWORD BufSize, DWORD StartAt, int FlexByte, DWORD Which)
{
    DWORD Count = 0, i = 0, j = 0;
	BOOL UseFlex = FlexByte & 100;
	BYTE FByte = FlexByte & 0xff;
    
    for(i = StartAt; i < BufSize - SrcSize; i++)
    {
        /* check if we can read src_size bytes from this location */
        if(IsBadReadPtr((void*)(Buf + 1), SrcSize))
            return 0;
            
		/* compare src_size from src against src_size bytes from buf */
		for(j = 0; j < SrcSize; j++)
		{
			/* if there's a difference and this isn't the flex_byte, move on */
			if((UseFlex && Src[j] != FByte) && Src[j] != Buf[i + j])
				break;
			else if(!UseFlex && Src[j] != Buf[i + j])
				break;

			/* if its the last byte for comparison, everything matched */
			if(j == (SrcSize - 1))
			{
				Count++;
				if(Count >= Which)
					return i;
			}
		}
  }

	/* if it got this far, then couldnt find it */
	return -1;
}

BYTE ByteNotInBuf(BYTE *Buf, DWORD BufSize)
{
	BYTE i = 0;
	DWORD j = 0;

	for(i = 0; i < 0xFF; i++)
	{
        for(j = 0; j < BufSize; j++)
		{
			if(Buf[j] == i)
				break;

			if(j == (BufSize - 1))
				return i;
		}
	}

	return 0xFF;
}
