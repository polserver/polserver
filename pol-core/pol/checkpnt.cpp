/*
History
=======

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/logfile.h"

#include "polcfg.h"
#include "checkpnt.h"

const char* last_checkpoint = "";
void checkpoint( const char* msg, unsigned short minlvl /* = 11 */ )
{
    last_checkpoint = msg;
    if (config.loglevel >= minlvl)
    {
        cout << "checkpoint: " << msg << endl;
    }
}

Checkpoint::Checkpoint( const char* file ) : _file(file), _line(0)
{
}
Checkpoint::~Checkpoint()
{
    if (_line)
        Log2( "Abnormal end after checkpoint: File %s, line %u\n", _file, _line );
}
