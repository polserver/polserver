/*
History
=======


Notes
=======

*/

#ifndef CLIB_TRACEBUF_H
#define CLIB_TRACEBUF_H

#ifdef NDEBUG

#define TRACEBUF_ADDELEM(tag,value) /**/

#else

#ifndef TRACEBUF_DEPTH
#define TRACEBUF_DEPTH 5000
#endif

struct TraceBufferElem
{
    const char* tag;
    unsigned int value;
};

extern TraceBufferElem tracebuffer[ TRACEBUF_DEPTH ];
extern unsigned tracebuffer_insertpoint;

inline void TRACEBUF_ADDELEM( const char* tag, unsigned int value )
{
    tracebuffer[ tracebuffer_insertpoint ].tag   = tag;
    tracebuffer[ tracebuffer_insertpoint ].value = value;
    ++tracebuffer_insertpoint;
    if (tracebuffer_insertpoint >= TRACEBUF_DEPTH)
        tracebuffer_insertpoint = 0;

}
#endif

void LogTraceBuffer();

#endif
