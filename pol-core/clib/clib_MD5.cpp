/** @file
 *
 * @par History
 * - 2003/06/29 Dave: MD5_Encrypt(), changed the order. the "safe" method was also really slow, so
 * do it only if the "fast" method fails.
 *                    also, store provider handle for further speed improvements
 */


#include "clib_MD5.h"

#include <fmt/format.h>

#include "stlutil.h"

#ifdef WINDOWS
#include "Header_Windows.h"
#include <wincrypt.h>

#include "logfacility.h"

namespace Pol::Clib
{
static HCRYPTPROV hProv = 0;

bool MD5_Encrypt( const std::string& in, std::string& out )
{
  HCRYPTHASH hHash = 0;

  if ( !hProv )
  {
    if ( !CryptAcquireContext( &hProv, nullptr, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )
    {
      if ( !CryptAcquireContext( &hProv, nullptr, nullptr, PROV_RSA_FULL, 0 ) )
      {
        ERROR_PRINTLN( "Error {} acquiring crypt context", GetLastError() );
        return false;
      }
    }
  }

  if ( !CryptCreateHash( hProv, CALG_MD5, 0, 0, &hHash ) )
  {
    ERROR_PRINTLN( "Error {} creating hash", GetLastError() );
    return false;
  }

  // Hash password string.
  if ( !CryptHashData( hHash, (const unsigned char*)( in.data() ),
                       static_cast<unsigned long>( in.length() ), 0 ) )
  {
    ERROR_PRINTLN( "Error {} adding data to hash", GetLastError() );
    return false;
  }
  unsigned long len = 16;
  unsigned char buf[16];
  if ( !CryptGetHashParam( hHash, HP_HASHVAL, buf, &len, 0 ) )
  {
    ERROR_PRINTLN( "Error {} getting hash value", GetLastError() );
    return false;
  }

  out = "";
  for ( auto& elem : buf )
  {
    fmt::format_to( std::back_inserter( out ), "{:02x}", (int)elem );
  }

  CryptDestroyHash( hHash );
  return true;
}

void MD5_Cleanup()
{
  if ( hProv )
  {
    CryptReleaseContext( hProv, 0 );
  }
}

#else

#include <openssl/evp.h>


namespace Pol::Clib
{
bool MD5_Encrypt( const std::string& in, std::string& out )
{
  auto ctx = EVP_MD_CTX_new();

  EVP_DigestInit_ex( ctx, EVP_md5(), nullptr );
  EVP_DigestUpdate( ctx, in.c_str(), in.length() );

  unsigned char hash[16];
  EVP_DigestFinal_ex( ctx, hash, nullptr );
  EVP_MD_CTX_free( ctx );

  out = "";
  for ( auto& elem : hash )
  {
    fmt::format_to( std::back_inserter( out ), "{:02x}", (int)elem );
  }

  return true;
}
void MD5_Cleanup()
{
  // OpenSSL cleanup, if any
}

#endif

bool MD5_Compare( const std::string& a, const std::string& b )
{
  return stringicmp( a, b ) == 0;
}
}  // namespace Pol::Clib
