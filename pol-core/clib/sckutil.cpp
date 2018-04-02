/** @file
 *
 * @par History
 */


#include "sckutil.h"

#include "esignal.h"
#include "strutil.h"
#include "wnsckt.h"

namespace Pol
{
namespace Clib
{
bool readline( Socket& sck, std::string& s, bool* timeout_exit, unsigned int timeout_secs,
               unsigned maxlen )
{
  if ( timeout_exit )
    *timeout_exit = false;
  s = "";
  unsigned char ch;
  unsigned timeouts_left = timeout_secs / 2;
  while ( !exit_signalled && sck.connected() )
  {
    if ( sck.recvbyte( &ch, 2000 ) )
    {
      timeouts_left = timeout_secs / 2;
      if ( isprint( ch ) )
      {
        s.append( 1, ch );
        if ( maxlen && s.length() > maxlen )
        {
          sck.close();
          return false;
        }
      }
      else
      {
        if ( ch == '\n' )
          return true;
      }
    }
    else
    {
      if ( timeout_secs && !--timeouts_left )
      {
        if ( timeout_exit )
          *timeout_exit = true;
        return false;
      }
    }
  }
  return false;
}

void writeline( Socket& sck, const std::string& s )
{
  sck.send( (void*)s.c_str(), static_cast<unsigned int>( s.length() ) );
  sck.send( "\r\n", 2 );
}

bool readstring( Socket& sck, std::string& s, unsigned int interchar_secs, unsigned length )
{
  s = "";
  unsigned char ch;
  unsigned timeouts_left = interchar_secs / 2;
  while ( !exit_signalled && sck.connected() )
  {
    if ( sck.recvbyte( &ch, 2000 ) )
    {
      timeouts_left = interchar_secs / 2;

      s.append( 1, ch );
      if ( s.length() == length )
      {
        return true;
      }
    }
    else
    {
      if ( !--timeouts_left )
        return false;
    }
  }
  return false;
}
}
}
