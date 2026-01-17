/** @file
 *
 * @par History
 */


#include "sckutil.h"

#include "esignal.h"
#include "strutil.h"
#include "wnsckt.h"


namespace Pol::Clib
{
void writeline( Socket& sck, const std::string& s )
{
  sck.send( (void*)s.c_str(), static_cast<unsigned int>( s.length() ) );
  sck.send( "\r\n", 2 );
}

}  // namespace Pol::Clib
