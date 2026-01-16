/** @file
 *
 * @par History
 */


#include "tracebuf.h"

#ifndef NDEBUG
#include "logfacility.h"
#include <fmt/format.h>
#include <iterator>
#endif


namespace Pol::Clib
{
#ifndef NDEBUG
TraceBufferElem tracebuffer[TRACEBUF_DEPTH];
unsigned tracebuffer_insertpoint;
#endif

void LogTraceBuffer()
{
#ifndef NDEBUG
  std::string tmp = "TraceBuffer:\n";
  for ( unsigned i = tracebuffer_insertpoint; i < TRACEBUF_DEPTH; ++i )
  {
    if ( tracebuffer[i].tag )
      fmt::format_to( std::back_inserter( tmp ), "{}={}\n", tracebuffer[i].tag,
                      tracebuffer[i].value );
  }
  for ( unsigned i = 0; i < tracebuffer_insertpoint; ++i )
  {
    if ( tracebuffer[i].tag )
      fmt::format_to( std::back_inserter( tmp ), "{}={}\n", tracebuffer[i].tag,
                      tracebuffer[i].value );
  }
  tmp += "End of TraceBuffer.\n";
  POLLOG( tmp );
#endif
}
}  // namespace Pol::Clib
