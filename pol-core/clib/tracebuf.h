/** @file
 *
 * @par History
 */


#ifndef CLIB_TRACEBUF_H
#define CLIB_TRACEBUF_H


namespace Pol::Clib
{
#ifndef NDEBUG

#ifndef TRACEBUF_DEPTH
#define TRACEBUF_DEPTH 5000
#endif

struct TraceBufferElem
{
  const char* tag;
  unsigned int value;
};

extern TraceBufferElem tracebuffer[TRACEBUF_DEPTH];
extern unsigned tracebuffer_insertpoint;

inline void _tracebuffer_addelem( const char* tag, unsigned int value )
{
  tracebuffer[tracebuffer_insertpoint].tag = tag;
  tracebuffer[tracebuffer_insertpoint].value = value;
  ++tracebuffer_insertpoint;
  if ( tracebuffer_insertpoint >= TRACEBUF_DEPTH )
    tracebuffer_insertpoint = 0;
}
#endif

void LogTraceBuffer();
}  // namespace Pol::Clib

#ifdef NDEBUG
#define TRACEBUF_ADDELEM( tag, value ) /**/
#else
#define TRACEBUF_ADDELEM( tag, value ) Clib::_tracebuffer_addelem( tag, value )
#endif


#endif  // CLIB_TRACEBUF_H
