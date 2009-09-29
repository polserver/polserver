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
* 	Dec 24th, 2004 -- this module contains an assembly of nifty functions ;)
* 
\******************************************************************************/

#ifndef _CRACKTOOLS_H_INCLUDED
#define _CRACKTOOLS_H_INCLUDED

int GetFunctionByRef(BYTE *Buf, DWORD BufSize, DWORD PosInFunction);
int GetFunctionRef(BYTE *Buf, DWORD BufSize, DWORD Function, DWORD Which);
int GetFunctionUnsafe(BYTE *Buf, DWORD BufSize, DWORD PosInFunction, DWORD NOPCount);
int FindCave(BYTE *Buf, DWORD BufSize, DWORD StartAt, DWORD CaveSize);
int GetTextRef(BYTE *Buf, DWORD BufSize, DWORD ImageBase, const char *Text, BOOL Exact, int Which);
int FleXSearch(BYTE *Src, BYTE *Buf, DWORD SrcSize, DWORD BufSize, DWORD StartAt, int FlexByte, DWORD Which);
BYTE ByteNotInBuf(BYTE *Buf, DWORD BufSize);

#endif
