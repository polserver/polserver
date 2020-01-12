
#include "uoinstallfinder.h"
#include "../clib/cfgelem.h"
#include "../clib/logfacility.h"
#include <stdexcept>
#include <string>

namespace Pol
{
namespace Plib
{
std::string UOInstallFinder::remove_elem( Clib::ConfigElem& elem )
{
  std::string uodata_root = elem.remove_string( "UoDataFileRoot", "" );
  if ( uodata_root.empty() )
  {
    uodata_root = Plib::UOInstallFinder::getInstallDir();
    if ( uodata_root.empty() )
    {
      elem.throw_error(
          "Property 'UoDataFileRoot' was not found, and no Ultima Online installation could be "
          "found on the system." );
    }

    INFO_PRINT << "Using Ultima Online data files from installation directory " << uodata_root
               << "\n";
  }
  return uodata_root;
}
}  // namespace Plib
}  // namespace Pol


#ifdef _WIN32

#include "../clib/fileutil.h"

#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>

LONG GetStringRegKey( HKEY hKey, const std::string& strValueName, std::string& strValue )
{
  CHAR szBuffer[512];
  DWORD dwBufferSize = sizeof( szBuffer );
  ULONG nError;
  nError = RegQueryValueExA( hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize );
  if ( ERROR_SUCCESS == nError )
  {
    strValue = szBuffer;
  }
  return nError;
}

const std::vector<std::string> KNOWN_KEYS = {
    "Electronic Arts\\EA Games\\Ultima Online Classic",
    "Electronic Arts\\EA Games\\Ultima Online Stygian Abyss Classic",
    "Origin Worlds Online\\Ultima Online\\KR Legacy Beta",
    "Origin Worlds Online\\Ultima Online Samurai Empire\\3d\\1.0",
    "Origin Worlds Online\\Ultima Online Samurai Empire\\2d\\1.0",
    "Origin Worlds Online\\Ultima Online Samurai Empire BETA\\3d\\1.0",
    "Origin Worlds Online\\Ultima Online Samurai Empire BETA\\2d\\1.0",
    "EA Games\\Ultima Online: Mondain's Legacy\\1.0",
    "EA Games\\Ultima Online: Mondain's Legacy\\1.00.0000",
    "EA GAMES\\Ultima Online: Samurai Empire\\1.00.0000",
    "EA Games\\Ultima Online: Mondain's Legacy",
    "EA GAMES\\Ultima Online Samurai Empire\\1.00.0000",
    "EA GAMES\\Ultima Online: Samurai Empire\\1.0",
    "EA GAMES\\Ultima Online Samurai Empire",
    "EA GAMES\\Ultima Online Samurai Empire\\1.0",
    "Origin Worlds Online\\Ultima Online\\1.0",
    "Origin Worlds Online\\Ultima Online Third Dawn\\1.0",
};

const std::vector<std::string> KNOWN_VALUES = {"ExePath", "Install Dir", "InstallDir"};

#ifdef _WIN64
const bool is64bit = true;
#else
const bool is64bit = false;
#endif

namespace Pol
{
namespace Plib
{
std::string UOInstallFinder::getInstallDir()
{
  HKEY hKey;
  LONG lRes;
  std::string installPath;
  for ( const auto& keyItr : KNOWN_KEYS )
  {
    auto keyPath = ( is64bit ? "SOFTWARE\\WOW6432Node\\" : "SOFTWARE\\" ) + keyItr;
    lRes = RegOpenKeyExA( HKEY_LOCAL_MACHINE, keyPath.c_str(), 0, KEY_READ, &hKey );
    if ( lRes != ERROR_SUCCESS )
    {
      lRes = RegOpenKeyExA( HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey );
      if ( lRes != ERROR_SUCCESS )
        continue;
    }
    for ( const auto& valueItr : KNOWN_VALUES )
    {
      lRes = GetStringRegKey( hKey, valueItr, installPath );
      if ( lRes == ERROR_SUCCESS )
      {
        if ( Clib::IsDirectory( installPath.c_str() ) )
        {
          RegCloseKey( hKey );
          return installPath;
        }
      }
    }
    RegCloseKey( hKey );
  }
  return "";
}



}  // namespace Plib
}  // namespace Pol

#else

namespace Pol
{
namespace Plib
{
std::string UOInstallFinder::getInstallDir()
{
  return "";
}
}  // namespace Plib
}  // namespace Pol

#endif