/*
History
=======
2005/06/01 Shinigami: added getstatics - to fill a list with statics

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/binaryfile.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/wallclock.h"

#include "staticblock.h"
#include "staticserver.h"

StaticServer::StaticServer( const RealmDescriptor& descriptor ) :
    _descriptor( descriptor ),
    _index(),
    _statics()
{
    BinaryFile index_file( _descriptor.path( "statidx.dat" ), ios::in );
    index_file.ReadVector( _index );
    if(_index.empty())
    {
        string message = "Empty file: " + _descriptor.path("statidx.dat");
        throw runtime_error( message );
    }

    BinaryFile statics_file( _descriptor.path( "statics.dat" ), ios::in );
    statics_file.ReadVector( _statics );
    if(_statics.empty())
    {
        string message = "Empty file: " + _descriptor.path("statics.dat");
        throw runtime_error( message );
    }

    Validate();
}

StaticServer::~StaticServer()
{
}

void StaticServer::Validate() const
{
	wallclock_t start = wallclock();
    cout << "Validating statics files: ";
    for( unsigned short y = 0; y < _descriptor.height; y += STATICBLOCK_CHUNK )
    {
        for( unsigned short x = 0; x < _descriptor.width; x += STATICBLOCK_CHUNK )
        {
            ValidateBlock( x, y );
        }
    }
	wallclock_t end = wallclock();
    int ms = wallclock_diff_ms( start, end );

    cout << "Completed in " << ms << " ms." << endl;
}

void StaticServer::ValidateBlock( unsigned short x, unsigned short y ) const
{
    unsigned short x_block = x / STATICBLOCK_CHUNK;
    unsigned short y_block = y / STATICBLOCK_CHUNK;

    size_t block_index = y_block * (_descriptor.width>>STATICBLOCK_SHIFT) + x_block;
    if (block_index+1 >= _index.size())
    {
        string message = "statics integrity error(1): x=" + tostring(x) + ", y=" + tostring(y);
        throw runtime_error( message );
    }
    unsigned int first_entry_index = _index[ block_index ].index;
    unsigned int num = _index[ block_index+1 ].index - first_entry_index;
    if (first_entry_index+num > _statics.size())
    {
        string message = "statics integrity error(2): x=" + tostring(x) + ", y=" + tostring(y);
        throw runtime_error( message );
    }
}

bool StaticServer::findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const
{
    passert( x < _descriptor.width && y < _descriptor.height );

    unsigned short x_block = x >> STATICBLOCK_SHIFT;
    unsigned short y_block = y >> STATICBLOCK_SHIFT;
    unsigned short xy = ((x & STATICCELL_MASK) << 4) | (y & STATICCELL_MASK);

    unsigned int block_index = x_block + y_block * (_descriptor.width >> STATICBLOCK_SHIFT);
    unsigned int first_entry_index = _index[ block_index ].index;
    unsigned int num = _index[ block_index+1 ].index - first_entry_index;
    
    if (num)
    {
        const STATIC_ENTRY* entry = &_statics[ first_entry_index ];
        while (num--)
        {
            if (entry->xy == xy && entry->objtype == objtype)
            {
                return true;
            }
            ++entry;
        }
    }
    return false;
}

void StaticServer::getstatics( StaticEntryList& statics, unsigned short x, unsigned short y ) const
{
    passert( x < _descriptor.width && y < _descriptor.height );

    unsigned short x_block = x >> STATICBLOCK_SHIFT;
    unsigned short y_block = y >> STATICBLOCK_SHIFT;
    unsigned short xy = ((x & STATICCELL_MASK) << 4) | (y & STATICCELL_MASK);

    unsigned int block_index = x_block + y_block * (_descriptor.width >> STATICBLOCK_SHIFT);
    unsigned int first_entry_index = _index[ block_index ].index;
    unsigned int num = _index[ block_index+1 ].index - first_entry_index;
    
    if (num)
    {
        const STATIC_ENTRY* entry = &_statics[ first_entry_index ];
        while (num--)
        {
            if (entry->xy == xy)
            {
                statics.push_back( *entry );
            }
            ++entry;
        }
    }
}
