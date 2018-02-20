/** @file
 *
 * @par History
 * - 2003/06/29 Dave: MD5_Encrypt(), changed the order. the "safe" method was also really slow, so
 * do it only if the "fast" method fails.
 *                    also, store provider handle for further speed improvements
 */


#include "clib_MD5.h"

#include <iomanip>
#include <sstream>
#ifdef WINDOWS
#include "pol_global_config_win.h"
#else
#include "pol_global_config.h"
#endif

#include "logfacility.h"
#include "stlutil.h"

#ifdef WINDOWS
#include "Header_Windows.h"
#include <wincrypt.h>

namespace Pol
{
namespace Clib
{
HCRYPTPROV hProv = NULL;

bool MD5_Encrypt( const std::string& in, std::string& out )
{
  // bool bResult = true;

  // HCRYPTKEY hKey = NULL;
  // HCRYPTKEY hXchgKey = NULL;
  HCRYPTHASH hHash = NULL;

  if ( !hProv )
  {
    if ( !CryptAcquireContext( &hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )
    {
      if ( !CryptAcquireContext( &hProv, NULL, NULL, PROV_RSA_FULL, 0 ) )
      {
        ERROR_PRINT << "Error " << GetLastError() << " acquiring crypt context\n";
        return false;
      }
    }
  }

  if ( !CryptCreateHash( hProv, CALG_MD5, 0, 0, &hHash ) )
  {
    ERROR_PRINT << "Error " << GetLastError() << " creating hash\n";
    return false;
  }

  // Hash password string.
  if ( !CryptHashData( hHash, (const unsigned char*)( in.data() ),
                       static_cast<unsigned long>( in.length() ), 0 ) )
  {
    ERROR_PRINT << "Error " << GetLastError() << " adding data to hash\n";
    return false;
  }
  unsigned long len = 16;
  unsigned char buf[16];
  if ( !CryptGetHashParam( hHash, HP_HASHVAL, buf, &len, 0 ) )
  {
    ERROR_PRINT << "Error " << GetLastError() << " getting hash value\n";
    return false;
  }

  std::ostringstream os;
  for ( unsigned int i = 0; i < len; i++ )
  {
    os << std::setfill( '0' ) << std::setw( 2 ) << std::hex << (int)buf[i];
  }
  out = os.str();

  CryptDestroyHash( hHash );
  return true;
}

void MD5_Cleanup()
{
  CryptReleaseContext( hProv, 0 );
}

#elif defined( HAVE_OPENSSL )

#include <openssl/md5.h>

namespace Pol
{
namespace Clib
{
bool MD5_Encrypt( const std::string& in, std::string& out )
{
  unsigned char sum[16];

  MD5( reinterpret_cast<const unsigned char*>( in.c_str() ), in.length(), sum );

  std::ostringstream os;
  for ( auto& elem : sum )
  {
    os << std::setfill( '0' ) << std::setw( 2 ) << std::hex << (int)elem;
  }
  out = os.str();

  return true;
}
void MD5_Cleanup()
{
  // OpenSSL cleanup, if any
}

#else
extern "C" {
#include "MD5.h"  //TODO: rework the following code - does not work with up-to-date header files anymore
}
namespace Pol
{
namespace Clib
{
bool MD5_Encrypt( const std::string& in, std::string& out )
{
  struct md5_ctx ctx;
  unsigned char sum[16];

  __md5_init_ctx( &ctx );
  __md5_process_bytes( in.c_str(), in.length(), &ctx );
  __md5_finish_ctx( &ctx, sum );

  std::ostringstream os;
  for ( unsigned int i = 0; i < sizeof( sum ); i++ )
  {
    os << std::setfill( '0' ) << std::setw( 2 ) << std::hex << (int)sum[i];
  }
  out = os.str();

  return true;
}

void MD5_Cleanup()
{
  // MD5 cleanup, if any. Consider storing ctx until server shutdown.
}

#endif

bool MD5_Compare( const std::string& a, const std::string& b )
{
  bool ret = false;
  if ( stringicmp( a, b ) == 0 )
    ret = true;

  return ret;
}
}
}
