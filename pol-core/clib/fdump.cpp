/** @file
 *
 * @par History
 */


#include "clib/fdump.h"
#include <fmt/format.h>

namespace Pol::Clib
{
void dump16( std::back_insert_iterator<std::string> buffer, const unsigned char* s, int len )
{
  int i;
  for ( i = 0; i < 16; i++ )
  {
    if ( i < len )
      fmt::format_to( buffer, "{:02x} ", (int)s[i] );
    else
      std::fill_n( buffer, 3, ' ' );
    if ( i == 7 )
      buffer = ' ';
  }
  std::fill_n( buffer, 2, ' ' );

  for ( i = 0; i < 16; i++ )
  {
    // Printable ASCII only, tested explicitly rather than with isprint(): ProgramMain sets
    // LC_CTYPE to en_US.UTF-8, and in a UTF-8 locale the libcs disagree about bytes >= 0x80 --
    // glibc calls them unprintable, BSD/macOS calls them printable. Taking isprint() at its
    // word there put raw bytes into a log every other tool reads as UTF-8, which made a
    // packet dump of 0xe4 undecodable and failed the CI summary step on macOS only.
    if ( i >= len || s[i] < 0x20 || s[i] > 0x7e )
      buffer = '.';
    else
      fmt::format_to( buffer, "{}", (char)s[i] );

    if ( i == 7 )
      buffer = ' ';
  }
  buffer = '\n';
}

void fdump( std::back_insert_iterator<std::string> buffer, const void* data, int len )
{
  int i;
  const unsigned char* s = (const unsigned char*)data;

  for ( i = 0; i < len; i += 16 )
  {
    int nprint = len - i;
    if ( nprint > 16 )
      nprint = 16;
    fmt::format_to( buffer, "{:04x} ", i );
    dump16( buffer, &s[i], nprint );
  }
}
}  // namespace Pol::Clib
