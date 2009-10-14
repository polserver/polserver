//////////////////////////////////////////////////////////////////////
//
// crypt.h
//
// Author:  Beosil
// E-Mail:  beosil@swileys.com
// Version: 1.26.4
// Date:    27. Jan. 2000
//
// Copyright (C) 1999 Bruno 'Beosil' Heidelberger
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
//////////////////////////////////////////////////////////////////////

#if !defined(_CRYPT_H_INCLUDED_)
#define _CRYPT_H_INCLUDED_

#define CRYPT_AUTO_VALUE	0x80

#define CRYPT_LOGINKEY_1	0x32750719
#define CRYPT_LOGINKEY_2	0x0a2d100b

#define CRYPT_GAMEKEY_LENGTH	6
#define CRYPT_GAMEKEY_COUNT		25

#define CRYPT_GAMESEED_LENGTH	8
#define CRYPT_GAMESEED_COUNT	25

#define CRYPT_GAMETABLE_START	1
#define CRYPT_GAMETABLE_STEP	3
#define CRYPT_GAMETABLE_MODULO	11
#define CRYPT_GAMETABLE_TRIGGER	21036

class CCrypt  
{
// Constructor / Destructor
public:	CCrypt();
		~CCrypt();

// Misc
public:		enum	{ typeLogin, typeGame, typeAuto };

// Class Variables
protected:	static bool		m_bTablesReady;

// Member Variables
protected:	int				m_type;
			unsigned int	m_loginSeed;
			unsigned int	m_loginKey[2];
			unsigned char	m_gameSeed[CRYPT_GAMESEED_LENGTH];
			int				m_gameTable;
			int				m_gameBlockPos;
			int				m_gameStreamPos;

// Member Functions
public:		void	Decrypt(void *pvIn, void *pvOut, int len);
			void	Init(void *pvSeed, int type = typeAuto);
			int		Pack(void *pvIn, void *pvOut, int len);

protected:	void	InitTables();
			void	RawDecrypt(unsigned int *pValues, int table);
};

#endif // !defined(_CRYPT_H_INCLUDED_)
