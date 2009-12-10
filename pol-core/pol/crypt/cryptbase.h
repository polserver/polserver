//////////////////////////////////////////////////////////////////////
//
// crypt/cryptbase.h
//
// Author:    Beosil <beosil@swileys.com>
// Date:      18. Apr. 2000
//
// Converted: Myrathi <tjh@myrathi.co.uk>
// Date:      03. Jan. 2003
//
// Copyright (C) 1999-2000 Bruno 'Beosil' Heidelberger
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//
// History:
// - 03. Jan. 2003 : Updated for use alongside POL's multi-encryption code.
// - 18. Apr. 2000 : Keys updated for client 2.0.0
// - 27. Jan. 2000 : Keys updated for client 1.26.4
// - 18. Jan. 2000 : Keys updated for client 1.26.3
// - 23. Nov. 1999 : Keys updated for client 1.26.2 and some compatibility fixes
// - 21. Sep. 1999 : Full redesign to fix the "21036 bytes"-bug
// -  9. Sep. 1999 : Keys updated for client 1.26.1
// -  2. Sep. 1999 : Keys and boxes updated for client 1.26.0b and minor bugfixes
// - 13. Aug. 1999 : First release, working with client 1.26.0(a)
//
//////////////////////////////////////////////////////////////////////

#ifndef __CRYPTBASE_H__
#define __CRYPTBASE_H__

#define CRYPT_AUTO_VALUE		0x80

#define CRYPT_GAMEKEY_LENGTH	6
#define CRYPT_GAMEKEY_COUNT		25

#define CRYPT_GAMESEED_LENGTH	8
#define CRYPT_GAMESEED_COUNT	25

#define CRYPT_GAMETABLE_START	1
#define CRYPT_GAMETABLE_STEP	3
#define CRYPT_GAMETABLE_MODULO	11
#define CRYPT_GAMETABLE_TRIGGER	21036

class CCryptBase
{
// Constructor / Destructor
public:

	CCryptBase();
	virtual ~CCryptBase();

// Misc
public:
	enum { typeLogin, typeGame, typeAuto };

// Class Variables
protected:
	static bool		m_bTablesReady;

// Member Variables
protected:
	int				m_type;
	unsigned int	m_loginSeed;
	unsigned int	m_loginKey[2];
	unsigned char	m_gameSeed[CRYPT_GAMESEED_LENGTH];
	int				m_gameTable;
	int				m_gameBlockPos;
	int				m_gameStreamPos;

	unsigned int	m_masterKey[2];

// Member Functions
public:

	virtual void	Decrypt(void *pvIn, void *pvOut, int len) = 0;
	virtual void	Init(void *pvSeed, int type = typeAuto) = 0;
};

#endif //__CRYPTBASE_H__
