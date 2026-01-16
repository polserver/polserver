//////////////////////////////////////////////////////////////////////
//
// crypt/cryptbase.cpp
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

#include <string.h>

#include "cryptbase.h"

namespace Pol
{
namespace Crypt
{
// The following tables must be implimented by the relevant derived classes:
//
//   Crypt Boxes
//   Crypt Tables
//   Key Table
//   Seed Table
//   Compression Table

// Constructor / Destructor
CCryptBaseCrypt::CCryptBaseCrypt() : CCryptBase(), lcrypt(), m_type( 0 )
{
  memset( &m_masterKey, 0, sizeof( m_masterKey ) );
  memset( &encrypted_data, 0, sizeof( encrypted_data ) );
}

void CCryptBaseCrypt::SetMasterKeys( unsigned int masterKey1, unsigned int masterKey2 )
{
  m_masterKey[0] = masterKey1 & 0xFFFFFFFF;
  m_masterKey[1] = masterKey2 & 0xFFFFFFFF;
}
}  // namespace Crypt
}  // namespace Pol
