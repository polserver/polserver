/*
History
=======

Notes
=======

*/

#ifndef PLIB_FILEMAPSERVER_H
#define PLIB_FILEMAPSERVER_H

#include "clib/binaryfile.h"

#include "mapblock.h"
#include "mapcell.h"
#include "mapserver.h"

class FileMapServer : public MapServer
{
public:
    explicit FileMapServer( const RealmDescriptor& descriptor );
    virtual ~FileMapServer() { }

    virtual MAPCELL GetMapCell( unsigned short x, unsigned short y ) const;

protected:
    mutable BinaryFile _mapfile;
    mutable long _cur_mapblock_index;
    mutable MAPBLOCK _cur_mapblock;

    // not implemented:
    FileMapServer& operator=( const FileMapServer&);
    FileMapServer( const FileMapServer& );
};

#endif
