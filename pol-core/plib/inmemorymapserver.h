/*
History
=======

Notes
=======

*/

#ifndef PLIB_INMEMORYMAPSERVER_H
#define PLIB_INMEMORYMAPSERVER_H

#include "filemapserver.h"

class InMemoryMapServer : public MapServer
{
public:
    explicit InMemoryMapServer( const RealmDescriptor& descriptor );
    virtual ~InMemoryMapServer();

    virtual MAPCELL GetMapCell( unsigned short x, unsigned short y ) const;

private:
    vector<MAPBLOCK> _mapblocks;

    // not implemented:
    InMemoryMapServer& operator=( const InMemoryMapServer& );
    InMemoryMapServer( const InMemoryMapServer& );
};

#endif
