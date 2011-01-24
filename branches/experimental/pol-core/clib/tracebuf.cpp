/*
History
=======

Notes
=======

*/

#include "stl_inc.h"
#include "logfile.h"
#include "tracebuf.h"

#ifndef NDEBUG
TraceBufferElem tracebuffer[ TRACEBUF_DEPTH ];
unsigned tracebuffer_insertpoint;
#endif

void LogTraceBuffer()
{
#ifndef NDEBUG
    Log( "TraceBuffer:\n" );
    for( unsigned i = tracebuffer_insertpoint; i < TRACEBUF_DEPTH; ++i )
    {
        if (tracebuffer[i].tag)
            Log( "%s=%ld\n", tracebuffer[i].tag, tracebuffer[i].value );
    }
    for( unsigned i = 0; i < tracebuffer_insertpoint; ++i )
    {
        if (tracebuffer[i].tag)
            Log( "%s=%ld\n", tracebuffer[i].tag, tracebuffer[i].value );
    }
    Log( "End of TraceBuffer.\n" );
#endif
}
