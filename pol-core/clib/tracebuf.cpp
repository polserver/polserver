/** @file
 *
 * @par History
 */


#include "tracebuf.h"
#include "logfacility.h"

namespace Pol
{
namespace Clib
{
#ifndef NDEBUG
TraceBufferElem tracebuffer[ TRACEBUF_DEPTH ];
unsigned tracebuffer_insertpoint;
#endif

void LogTraceBuffer()
{
#ifndef NDEBUG
  fmt::Writer tmp;
  tmp << "TraceBuffer:\n";
  for( unsigned i = tracebuffer_insertpoint; i < TRACEBUF_DEPTH; ++i )
  {
    if ( tracebuffer[i].tag )
      tmp << tracebuffer[i].tag << "=" << tracebuffer[i].value << "\n";
  }
  for( unsigned i = 0; i < tracebuffer_insertpoint; ++i )
  {
    if (tracebuffer[i].tag)
      tmp << tracebuffer[i].tag << "=" << tracebuffer[i].value << "\n";
  }
  tmp << "End of TraceBuffer.\n";
  POLLOG << tmp.str();
#endif
}
}
}