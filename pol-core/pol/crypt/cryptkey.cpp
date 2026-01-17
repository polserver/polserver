/********************************************************************
** cryptkey.cpp : Encryption keys (and 'type' codes)
**
** Created: 3/1/2003, 2:59
** Author:  TJ Houston <tjh@myrathi.co.uk>
**
**
** Notes
**********
**    TJ: up to 3.0.6j added
**    Tomi: Remade the whole list of clients and added all from the first Beta patch to the latest
*SA.
**    Tomi: Removed the enabled param.
**
*********************************************************************/

#include "cryptkey.h"

#include <cstring>

#include "../../clib/clib.h"
#include "../../clib/logfacility.h"


namespace Pol::Crypt
{
bool compareVersion( int ver1major, int ver1minor, int ver1build, int ver2major, int ver2minor,
                     int ver2build );

void CalculateCryptKeys( const std::string& name, TCryptInfo& infoCrypt )
{
  size_t len = name.length();
  if ( ( strnicmp( "none", name.c_str(), len ) == 0 ) ||
       ( strnicmp( "ignition", name.c_str(), len ) == 0 ) ||
       ( strnicmp( "uorice", name.c_str(), len ) == 0 ) )
  {
    infoCrypt.eType = CRYPT_NOCRYPT;
    infoCrypt.uiKey1 = 0;
    infoCrypt.uiKey2 = 0;
  }
  else if ( strnicmp( "2.0.0x", name.c_str(), 6 ) == 0 )
  {
    infoCrypt.eType = CRYPT_BLOWFISH_TWOFISH;
    infoCrypt.uiKey1 = 0x2D13A5FD;
    infoCrypt.uiKey2 = 0xA39D527F;
  }
  else
  {
    try
    {
      size_t dot1 = name.find_first_of( '.', 0 );
      size_t dot2 = name.find_first_of( '.', dot1 + 1 );

      int major = atoi( name.substr( 0, dot1 ).c_str() );
      int minor = atoi( name.substr( dot1 + 1, dot2 - dot1 - 1 ).c_str() );
      int build = atoi( name.substr( dot2 + 1, len - dot2 - 1 ).c_str() );

      int temp = ( ( major << 9 | minor ) << 10 | build ) ^ ( ( build * build ) << 5 );
      infoCrypt.uiKey1 = ( temp << 4 ) ^ ( minor * minor ) ^ ( minor * 0x0B000000 ) ^
                         ( build * 0x380000 ) ^ 0x2C13A5FD;
      temp = ( ( ( major << 9 | build ) << 10 | minor ) * 8 ) ^ ( build * build * 0x0c00 );
      infoCrypt.uiKey2 =
          temp ^ ( minor * minor ) ^ ( minor * 0x6800000 ) ^ ( build * 0x1c0000 ) ^ 0x0A31D527F;

      if ( compareVersion( 1, 25, 35, major, minor, build ) )
        infoCrypt.eType = CRYPT_OLD_BLOWFISH;
      else if ( ( major == 1 ) && ( minor == 25 ) && ( build == 36 ) )
        infoCrypt.eType = CRYPT_1_25_36;
      else if ( compareVersion( 2, 0, 0, major, minor, build ) )
        infoCrypt.eType = CRYPT_BLOWFISH;
      else if ( compareVersion( 2, 0, 3, major, minor, build ) )
        infoCrypt.eType = CRYPT_BLOWFISH_TWOFISH;
      else
        infoCrypt.eType = CRYPT_TWOFISH;
    }
    catch ( ... )
    {
      infoCrypt.eType = CRYPT_NOCRYPT;
      infoCrypt.uiKey1 = 0;
      infoCrypt.uiKey2 = 0;
      POLLOG_ERRORLN( "Malformed encryption version string: {} using Ignition encryption engine",
                      name );
    }
  }
}

bool compareVersion( int ver1major, int ver1minor, int ver1build, int ver2major, int ver2minor,
                     int ver2build )
{
  if ( ver1major > ver2major )
    return true;
  if ( ver1major < ver2major )
    return false;
  if ( ver1minor > ver2minor )
    return true;
  else if ( ver1minor < ver2minor )
    return false;
  else if ( ver1build > ver2build )
    return true;
  else if ( ver1build < ver2build )
    return false;
  else
    return true;
}

// Just for the docs, old list of all Cryptkeys
// TWOFISH
// AddClient("7.0.4",   0x2F7385BD, 0xA2AD127F, CRYPT_TWOFISH);
// AddClient("7.0.3",   0x2F3BB7CD, 0xA2895E7F, CRYPT_TWOFISH);
// AddClient("7.0.2",   0x2FE3ADDD, 0xA2E5227F, CRYPT_TWOFISH);
// AddClient("7.0.1",   0x2FABA7ED, 0xA2C17E7F, CRYPT_TWOFISH);
// AddClient("7.0.0",   0x2F93A5FD, 0xA2DD527F, CRYPT_TWOFISH);
// AddClient("6.0.14",  0x2C022D1D, 0xA31DA27F, CRYPT_TWOFISH);
// AddClient("6.0.13",  0x2DCAF72D, 0xA3F71E7F, CRYPT_TWOFISH);
// AddClient("6.0.12",  0x2DB2853D, 0xA3CA127F, CRYPT_TWOFISH);
// AddClient("6.0.11",  0x2D7B574D, 0xA3AD9E7F, CRYPT_TWOFISH);
// AddClient("6.0.10",  0x2D236D5D, 0xA380A27F, CRYPT_TWOFISH);
// AddClient("6.0.9",   0x2EEB076D, 0xA263BE7F, CRYPT_TWOFISH);
// AddClient("6.0.8",   0x2ED3257D, 0xA27F527F, CRYPT_TWOFISH);
// AddClient("6.0.7",   0x2E9BC78D, 0xA25BFE7F, CRYPT_TWOFISH);
// AddClient("6.0.6",   0x2E43ED9D, 0xA234227F, CRYPT_TWOFISH);
// AddClient("6.0.5",   0x2E0B97AD, 0xA210DE7F, CRYPT_TWOFISH);
// AddClient("6.0.4",   0x2FF385BD, 0xA2ED127F, CRYPT_TWOFISH);
// AddClient("6.0.3",   0x2FBBB7CD, 0xA2C95E7F, CRYPT_TWOFISH);
// AddClient("6.0.2",   0x2F63ADDD, 0xA2A5227F, CRYPT_TWOFISH);
// AddClient("6.0.1",   0x2F2BA7ED, 0xA2817E7F, CRYPT_TWOFISH);
// AddClient("5.0.9",   0x2F6B076D, 0xA2A3BE7F, CRYPT_TWOFISH);
// AddClient("5.0.8",   0x2F53257D, 0xA2BF527F, CRYPT_TWOFISH);
// AddClient("5.0.7",   0x2F1BC78D, 0xA29BFE7F, CRYPT_TWOFISH);
// AddClient("5.0.6",   0x2FC3ED9D, 0xA2F4227F, CRYPT_TWOFISH);
// AddClient("5.0.5",   0x2F8B97AD, 0xA2D0DE7F, CRYPT_TWOFISH);
// AddClient("5.0.4",   0x2E7385BD, 0xA22D127F, CRYPT_TWOFISH);
// AddClient("5.0.3",   0x2E3BB7CD, 0xA2095E7F, CRYPT_TWOFISH);
// AddClient("5.0.2",   0x2EE3ADDD, 0xA265227F, CRYPT_TWOFISH);
// AddClient("5.0.1",   0x2EABA7ED, 0xA2417E7F, CRYPT_TWOFISH);
// AddClient("5.0.0",   0x2E93A5FD, 0xA25D527F, CRYPT_TWOFISH);
// AddClient("4.0.11",  0x2C7B574D, 0xA32D9E7F, CRYPT_TWOFISH);
// AddClient("4.0.10",  0x2C236D5D, 0xA300A27F, CRYPT_TWOFISH);
// AddClient("4.0.9",   0x2FEB076D, 0xA2E3BE7F, CRYPT_TWOFISH);
// AddClient("4.0.8",   0x2FD3257D, 0xA2FF527F, CRYPT_TWOFISH);
// AddClient("4.0.7",   0x2F9BC78D, 0xA2DBFE7F, CRYPT_TWOFISH);
// AddClient("4.0.6",   0x2F43ED9D, 0xA2B4227F, CRYPT_TWOFISH);
// AddClient("4.0.5",   0x2F0B97AD, 0xA290DE7F, CRYPT_TWOFISH);
// AddClient("4.0.4",   0x2EF385BD, 0xA26D127F, CRYPT_TWOFISH);
// AddClient("4.0.3",   0x2EBBB7CD, 0xA2495E7F, CRYPT_TWOFISH);
// AddClient("4.0.2",   0x2E63ADDD, 0xA225227F, CRYPT_TWOFISH);
// AddClient("4.0.1",   0x2E2BA7ED, 0xA2017E7F, CRYPT_TWOFISH);
// AddClient("4.0.0",   0x2E13A5FD, 0xA21D527F, CRYPT_TWOFISH);
// AddClient("3.0.8",   0x2C53257D, 0xA33F527F, CRYPT_TWOFISH);
// AddClient("3.0.7",   0x2C1BC78D, 0xA31BFE7F, CRYPT_TWOFISH);
// AddClient("3.0.6",   0x2CC3ED9D, 0xA374227F, CRYPT_TWOFISH);
// AddClient("3.0.5",   0x2C8B97AD, 0xA350DE7F, CRYPT_TWOFISH);
// AddClient("3.0.4",   0x2D7385BD, 0xA3AD127F, CRYPT_TWOFISH);
// AddClient("3.0.3",   0x2D3BB7CD, 0xA3895E7F, CRYPT_TWOFISH);
// AddClient("3.0.2",   0x2DE3ADDD, 0xA3E5227F, CRYPT_TWOFISH);
// AddClient("3.0.1",   0x2DABA7ED, 0xA3C17E7F, CRYPT_TWOFISH);
// AddClient("3.0.0",   0x2D93A5FD, 0xA3DD527F, CRYPT_TWOFISH);
// AddClient("2.0.9",   0x2CEB076D, 0xA363BE7F, CRYPT_TWOFISH);
// AddClient("2.0.8",   0x2CD3257D, 0xA37F527F, CRYPT_TWOFISH);
// AddClient("2.0.7",   0x2C9BC78D, 0xA35BFE7F, CRYPT_TWOFISH);
// AddClient("2.0.6",   0x2C43ED9D, 0xA334227F, CRYPT_TWOFISH);
// AddClient("2.0.5",   0x2C0B97AD, 0xA310DE7F, CRYPT_TWOFISH);
// AddClient("2.0.4",   0x2DF385BD, 0xA3ED127F, CRYPT_TWOFISH);

//// BLOWFISH + TWOFISH

// AddClient("2.0.3",   0x2DBBB7CD, 0xA3C95E7F, CRYPT_BLOWFISH_TWOFISH);
// AddClient("2.0.2",   0x2D63ADDD, 0xA3A5227F, CRYPT_BLOWFISH_TWOFISH);
// AddClient("2.0.1",   0x2D2BA7ED, 0xA3817E7F, CRYPT_BLOWFISH_TWOFISH);
// AddClient("2.0.0x",  0x2D13A5FD, 0xA39D527F, CRYPT_BLOWFISH_TWOFISH);

//// BLOWFISH

// AddClient("2.0.0",   0x2D13A5FD, 0xA39D527F, CRYPT_BLOWFISH);
// AddClient("1.26.4",  0x32750719, 0x0A2D100B, CRYPT_BLOWFISH);
// AddClient("1.26.3",  0x323D3569, 0x0A095C0B, CRYPT_BLOWFISH);
// AddClient("1.26.2",  0x32E52F79, 0x0A65200B, CRYPT_BLOWFISH);
// AddClient("1.26.1",  0x32AD2549, 0x0A417C0B, CRYPT_BLOWFISH);
// AddClient("1.26.0",  0x32952759, 0x0A5D500B, CRYPT_BLOWFISH);
// AddClient("1.25.37", 0x378757DC, 0x0595DCC6, CRYPT_BLOWFISH);

//// BLOWFISH WITH SPECIAL LOGINCRYPT ONLY FOR THIS VERSION

// AddClient("1.25.36",  0x387FC5CC, 0x021510C6, CRYPT_1_25_36);

//// BLOWFISH WITH OLD LOGINCRYPT

// AddClient("1.25.35",  0x383477BC, 0x02345CC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.34",  0x38ECEDAC, 0x025720C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.33",  0x38A5679C, 0x02767CC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.32",  0x389DE58C, 0x026950C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.31",  0x395A647C, 0x0297BCC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.30",  0x3902EE6C, 0x02BCA0C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.28",  0x39B3C64C, 0x02EA10C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.26",  0x3A20AE2C, 0x0319A0C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.25",  0x3AE9041C, 0x037F3CC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.23",  0x3A99C4FC, 0x03437CC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.21",  0x3B0E94DC, 0x03875CC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.20",  0x3BF6C6CC, 0x03FD10C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.19",  0x3BBF34BC, 0x03DBDCC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.18",  0x3C676EAC, 0x002820C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.16",  0x3C17E68C, 0x001350C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.15",  0x3CDC257C, 0x00723CC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.13",  0x3D4CB55C, 0x00B71CC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.12",  0x3D34C74C, 0x008A10C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.11",  0x3DFD153C, 0x00ED9CC6, CRYPT_OLD_BLOWFISH);
// AddClient("1.25.10",  0x3DA52F2C, 0x00C0A0C6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.41",  0xD963477C, 0x32EABCD6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.40",  0xD95AE56C, 0x32F350D6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.39",  0xD915879C, 0x32DAFCD6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.38",  0xD9CD2D8C, 0x32B220D6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.37",  0xD984D7BC, 0x3295DCD6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.36",  0xD67C45AC, 0x351510D6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.35",  0xD637F7DC, 0x35345CD6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.34",  0xD6EF6DCC, 0x355720D6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.33",  0xD6A6E7FC, 0x35767CD6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.32",  0xD69E65EC, 0x356950D6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.31",  0xD759E41C, 0x3597BCD6, CRYPT_OLD_BLOWFISH);
// AddClient("1.23.27",  0xD47BD45C, 0x34081CD6, CRYPT_OLD_BLOWFISH);

//// NO ENCRYPTION

// AddClient("ignition", 0,          0,          CRYPT_NOCRYPT);
// AddClient("none",    0,          0,          CRYPT_NOCRYPT); //dave added 3/14, avoid newbie
// confusion
// AddClient("uorice",    0,          0,          CRYPT_NOCRYPT); //dave added 3/14, avoid newbie
// confusion
}  // namespace Pol::Crypt
