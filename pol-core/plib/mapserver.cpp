/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include "clib/binaryfile.h"
#include "clib/passert.h"
#include "clib/strutil.h"

#include "filemapserver.h"
#include "inmemorymapserver.h"
#include "mapblock.h"
#include "mapserver.h"
#include "mapshape.h"
#include "mapsolid.h"
#include "realmdescriptor.h"

MapServer::MapServer( const RealmDescriptor& descriptor ) :
    _descriptor( descriptor )
{
    LoadSolids();

    // the first-level index points into the second-level index, so load the second-level index first.
    LoadSecondLevelIndex();

    LoadFirstLevelIndex();
}

MapServer::~MapServer()
{
}

void MapServer::LoadSolids()
{
    string filename = _descriptor.path( "solids.dat" );

    BinaryFile infile( filename, ios::in );
    infile.ReadVector( _shapedata );
}

void MapServer::LoadSecondLevelIndex()
{
    string filename = _descriptor.path( "solidx2.dat" );

    BinaryFile infile( filename, ios::in );
    fstream::pos_type filesize = infile.FileSize();
    if (filesize < fstream::pos_type(SOLIDX2_FILLER_SIZE))
        throw runtime_error( filename + " must have size of at least " + decint(SOLIDX2_FILLER_SIZE) + " bytes." );

    fstream::pos_type databytes = filesize - fstream::pos_type(SOLIDX2_FILLER_SIZE);
    if ((databytes % sizeof(SOLIDX2_ELEM)) != 0)
        throw runtime_error( filename + " does not contain an integral number of elements." );

    unsigned count = static_cast<unsigned int>(databytes / sizeof(SOLIDX2_ELEM));
    _index2.resize( count );
    infile.Seek( SOLIDX2_FILLER_SIZE );
    infile.Read( &_index2[0], count );
    
    // integrity check
    for( unsigned i = 0; i < _index2.size(); ++i )
    {
        SOLIDX2_ELEM& elem = _index2.at(i);
        passert( elem.baseindex < _shapedata.size() );

        for( unsigned x = 0; x < SOLIDX_X_SIZE; ++x )
        {
            for( unsigned y = 0; y < SOLIDX_Y_SIZE; ++y )
            {
                unsigned idx = elem.baseindex + elem.addindex[x][y];
                passert( idx < _shapedata.size() );
            }
        }
    }
}

void MapServer::LoadFirstLevelIndex()
{
    string filename = _descriptor.path( "solidx1.dat" );

    BinaryFile infile( filename, ios::in );

    unsigned n_blocks = (_descriptor.width / SOLIDX_X_SIZE) * (_descriptor.height / SOLIDX_Y_SIZE);
    _index1.resize( n_blocks );
    
    for( unsigned i = 0; i < n_blocks; ++i )
    {
        unsigned long tmp;
        infile.Read( tmp );
        if (tmp)
        {
            // tmp is an offset, in the file..  turn it into a pointer into the second-level index.
            tmp = (tmp - SOLIDX2_FILLER_SIZE) / sizeof(SOLIDX2_ELEM);
            _index1[i] = &_index2.at(tmp);
        }
        else
        {
            _index1[i] = NULL;
        }
    }
}

void MapServer::GetMapShapes( MapShapeList& shapes, unsigned x, unsigned y, unsigned long anyflags ) const
{
    passert( x < _descriptor.width && y < _descriptor.height );

    MAPCELL cell = GetMapCell( x, y );
    MapShape shape;

    if (cell.flags & anyflags)
    {
        shape.flags = cell.flags;
        shape.z = cell.z-1; //assume now map is at z-1 with height 1 for solidity
        shape.height = 1;
        shapes.push_back( shape );
    }

    if (cell.flags & FLAG::MORE_SOLIDS)
    {
        unsigned xblock = x >>  SOLIDX_X_SHIFT;
        unsigned xcell  = x &   SOLIDX_X_CELLMASK;
        unsigned yblock = y >>  SOLIDX_Y_SHIFT;
        unsigned ycell  = y &   SOLIDX_Y_CELLMASK;

        long block = yblock * (_descriptor.width >> SOLIDX_X_SHIFT) + xblock;
        SOLIDX2_ELEM* pIndex2 = _index1[ block ];
        unsigned long index = pIndex2->baseindex + pIndex2->addindex[xcell][ycell];
        const SOLIDS_ELEM* pElem = &_shapedata[index];
        for(;;)
        {
            if (pElem->flags & anyflags)
            {
                shape.z = pElem->z;
                shape.height = pElem->height;
                shape.flags = pElem->flags;
                shapes.push_back( shape );
            }
            if (pElem->flags & FLAG::MORE_SOLIDS)
                ++pElem;
            else
                break;
        } 
    }
}

MapServer* MapServer::Create( const RealmDescriptor& descriptor )
{
    if (descriptor.mapserver_type == "memory")
    {
        return new InMemoryMapServer( descriptor );
    }
    else if (descriptor.mapserver_type == "file")
    {
        return new FileMapServer( descriptor );
    }
    else
    {
        throw runtime_error( "Undefined mapserver type: " + descriptor.mapserver_type );
    }
}
