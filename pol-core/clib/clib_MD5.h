/** @file
 *
 * @par History
 */


#ifndef CLIB_MD5_H
#define CLIB_MD5_H

#include <string>

namespace Pol
{
namespace Clib
{
bool MD5_Encrypt( const std::string& in, std::string& out );
bool MD5_Compare( const std::string& a, const std::string& b );
void MD5_Cleanup();
}
}
#endif  // CLIB_MD5_H
