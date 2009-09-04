/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _WIN32
#pragma warning( disable: 4786 )
#endif

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../bscript/config.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fdump.h"
#include "../clib/strutil.h"
#include "../clib/dirlist.h"

#include "../pol/crypwrap.h"
#include "../pol/ustruct.h"
#include "../pol/polfile.h"
#include "../pol/uofile.h"
#include "../pol/uofilei.h"
#include "../pol/udatfile.h"
#include "../pol/polcfg.h"
#include "../pol/multi/multidef.h"

unsigned long mapcache_misses;
unsigned long mapcache_hits;
bool BoatShapeExists(unsigned short graphic)
{
    return false;
}

PolConfig config;

int keyid[] = {
0x0002, 0x01f5, 0x0226, 0x0347, 0x0757, 0x0286, 0x03b6, 0x0327,
0x0e08, 0x0628, 0x0567, 0x0798, 0x19d9, 0x0978, 0x02a6, 0x0577,
0x0718, 0x05b8, 0x1cc9, 0x0a78, 0x0257, 0x04f7, 0x0668, 0x07d8,
0x1919, 0x1ce9, 0x03f7, 0x0909, 0x0598, 0x07b8, 0x0918, 0x0c68,
0x02d6, 0x1869, 0x06f8, 0x0939, 0x1cca, 0x05a8, 0x1aea, 0x1c0a,
0x1489, 0x14a9, 0x0829, 0x19fa, 0x1719, 0x1209, 0x0e79, 0x1f3a,
0x14b9, 0x1009, 0x1909, 0x0136, 0x1619, 0x1259, 0x1339, 0x1959,
0x1739, 0x1ca9, 0x0869, 0x1e99, 0x0db9, 0x1ec9, 0x08b9, 0x0859,
0x00a5, 0x0968, 0x09c8, 0x1c39, 0x19c9, 0x08f9, 0x18f9, 0x0919,
0x0879, 0x0c69, 0x1779, 0x0899, 0x0d69, 0x08c9, 0x1ee9, 0x1eb9,
0x0849, 0x1649, 0x1759, 0x1cd9, 0x05e8, 0x0889, 0x12b9, 0x1729,
0x10a9, 0x08d9, 0x13a9, 0x11c9, 0x1e1a, 0x1e0a, 0x1879, 0x1dca,
0x1dfa, 0x0747, 0x19f9, 0x08d8, 0x0e48, 0x0797, 0x0ea9, 0x0e19,
0x0408, 0x0417, 0x10b9, 0x0b09, 0x06a8, 0x0c18, 0x0717, 0x0787,
0x0b18, 0x14c9, 0x0437, 0x0768, 0x0667, 0x04d7, 0x08a9, 0x02f6,
0x0c98, 0x0ce9, 0x1499, 0x1609, 0x1baa, 0x19ea, 0x39fa, 0x0e59,
0x1949, 0x1849, 0x1269, 0x0307, 0x06c8, 0x1219, 0x1e89, 0x1c1a,
0x11da, 0x163a, 0x385a, 0x3dba, 0x17da, 0x106a, 0x397a, 0x24ea,
0x02e7, 0x0988, 0x33ca, 0x32ea, 0x1e9a, 0x0bf9, 0x3dfa, 0x1dda,
0x32da, 0x2eda, 0x30ba, 0x107a, 0x2e8a, 0x3dea, 0x125a, 0x1e8a,
0x0e99, 0x1cda, 0x1b5a, 0x1659, 0x232a, 0x2e1a, 0x3aeb, 0x3c6b,
0x3e2b, 0x205a, 0x29aa, 0x248a, 0x2cda, 0x23ba, 0x3c5b, 0x251a,
0x2e9a, 0x252a, 0x1ea9, 0x3a0b, 0x391b, 0x23ca, 0x392b, 0x3d5b,
0x233a, 0x2cca, 0x390b, 0x1bba, 0x3a1b, 0x3c4b, 0x211a, 0x203a,
0x12a9, 0x231a, 0x3e0b, 0x29ba, 0x3d7b, 0x202a, 0x3adb, 0x213a,
0x253a, 0x32ca, 0x23da, 0x23fa, 0x32fa, 0x11ca, 0x384a, 0x31ca,
0x17ca, 0x30aa, 0x2e0a, 0x276a, 0x250a, 0x3e3b, 0x396a, 0x18fa,
0x204a, 0x206a, 0x230a, 0x265a, 0x212a, 0x23ea, 0x3acb, 0x393b,
0x3e1b, 0x1dea, 0x3d6b, 0x31da, 0x3e5b, 0x3e4b, 0x207a, 0x3c7b,
0x277a, 0x3d4b, 0x0c08, 0x162a, 0x3daa, 0x124a, 0x1b4a, 0x264a,
0x33da, 0x1d1a, 0x1afa, 0x39ea, 0x24fa, 0x373b, 0x249a, 0x372b,
0x1679, 0x210a, 0x23aa, 0x1b8a, 0x3afb, 0x18ea, 0x2eca, 0x0627,
0x00d4 // terminator
} ;

/* transforms table above into:
    { nbits, bits_reversed [[ LSB .. MSB ]] },
*/
int print_ctable()
{
    printf( "#include \"ctable.h\"\n\n" );

    printf( "SVR_KEYDESC keydesc[ 257 ] = {\n" );
    for( int i = 0; i <= 256; i++ )
    {
        int nbits = keyid[ i ] & 0x0F;
        unsigned short inmask = 0x10;
        const unsigned short val = static_cast<const unsigned short>(keyid[i]);
        unsigned short valout = 0;
        while (nbits--)
        {
            valout <<= 1;
            if (val & inmask)
                valout |= 1;
            inmask <<= 1;
        }
        printf( "    { %2u, 0x%4.04x, 0x%4.04x },\n", 
                   keyid[i] & 0x0F, 
                   keyid[i] >> 4, 
                   valout );
    }
    printf( "};\n" );
    return 0;
}

unsigned char buffer[ 10000 ];

int Usage( int ret )
{
	fprintf( stderr, "Usage: uotool func [params ...]\n" );
	fprintf( stderr, "  Functions:\n" );
	fprintf( stderr, "    tiledump filename        Dump object information\n" );
	fprintf( stderr, "    vertile                  Dump updated object info\n" );
	fprintf( stderr, "    rawdump filename hdrlen reclen\n" );
    fprintf( stderr, "    ctable                   Print Server encryption table\n" );
    fprintf( stderr, "    findgraphic graphic      finds coords of statics with graphic\n" );
    fprintf( stderr, "    findlandtileflags flags  finds landtiles with flags\n" );
    fprintf( stderr, "    contour                  write binary file 6144x4096 z heights\n" );
    fprintf( stderr, "    mapdump x1 y1 [x2 y2]    dumps map+statics info to html tables\n" );
    fprintf( stderr, "    landtileflagsearch flags [notflags]  prints landtiles matching args\n" );
    fprintf( stderr, "    flagsearch flags [notflags]          prints tiles matching flags\n" );
    fprintf( stderr, "    landtilehist             prints landtile histogram\n" );
    fprintf( stderr, "    staticshist              histogram of number of statics at a location\n" );
    fprintf( stderr, "    zhist                    histogram of map z\n" );
    fprintf( stderr, "    multis                   prints multi definitions \n" );
    fprintf( stderr, "    verdata                  prints verdata info\n" );
    fprintf( stderr, "    statics                  prints statics info\n" );
    fprintf( stderr, "    sndlist                  prints sound list info\n" );
    fprintf( stderr, "    verlandtile              prints verdata landtile info\n" );
    fprintf( stderr, "    loschange                prints differences in LOS handling \n" );
    return ret;
}
#define TILES_START 0x68800

void display_tileinfo( unsigned short objtype, const USTRUCT_TILE& tile )
{

		             printf( "objtype:  0x%4.04x\n", objtype );
		             printf( "  name:   %s\n", tile.name );
	if (tile.flags)  printf( "  flags:  0x%8.08lX\n", tile.flags );
	if (tile.weight) printf( "  weight: 0x%2.02X\n", tile.weight );
	if (tile.layer)  printf( "  layer:  0x%2.02X\n", tile.layer );
	if (tile.unk6)   printf( "  unk6:   0x%2.02X\n", tile.unk6 );
	if (tile.unk7)   printf( "  unk7:   0x%2.02X\n", tile.unk7 );
	if (tile.unk8)   printf( "  unk8:   0x%2.02X\n", tile.unk8 );
	if (tile.unk9)   printf( "  unk9:   0x%2.02X\n", tile.unk9 );
	if (tile.anim)   printf( "  anim:   0x%8.08lX\n", tile.anim );
	if (tile.height) printf( "  height: 0x%2.02X\n", tile.height );
	if (tile.unk14)  printf( "  unk14:  0x%2.02X\n", tile.unk14 );
	if (tile.unk15)  printf( "  unk15:  0x%2.02X\n", tile.unk15 );
}

int tiledump( int argc, char **argv )
{
	USTRUCT_TILE tile;
	u32 version;
	if (argc != 3) 
		return Usage( 1 );

	FILE *fp = fopen( argv[2], "rb" );
	fseek( fp, TILES_START, SEEK_SET );
	int recnum = 0;
	unsigned short objtype = 0;
	for(;;)
	{
		if (recnum == 0)
		{
			if (fread( &version, sizeof version, 1, fp ) != 1)
				break;
			printf( "Block Version: %08lX\n", version );
		}
		if (fread(  &tile, sizeof tile, 1, fp ) != 1)
			break;
		display_tileinfo( objtype, tile );

 		
		++objtype;
		++recnum;
		if (recnum == 32)
			recnum = 0;

	}
    return 0;
}

int vertile( int argc, char **argv )
{
	USTRUCT_TILE tile;
    
    open_uo_data_files();
    read_uo_data();

	int i;
	for( i = 0; i <= 0xFFFF; i++ )
	{
		unsigned short objtype = (unsigned short) i;
		read_objinfo( objtype, tile );
        display_tileinfo( objtype, tile );
	}
	return 0;
}

int verlandtile( int argc, char **argv )
{
	USTRUCT_LAND_TILE landtile;
    
    open_uo_data_files();
    read_uo_data();

	int i;
	for( i = 0; i < 0x4000; i++ )
	{
		unsigned short objtype = (unsigned short) i;
		readlandtile( objtype, &landtile );
        if (landtile.flags || landtile.unk || landtile.name[0])
        {
            cout << "Land Tile: " << hex << objtype << dec << endl;
            cout << "\tflags: " << hex << landtile.flags << dec << endl;
            cout << "\t  unk: " << hex << landtile.unk << dec << endl;
            cout << "\t name: " << landtile.name << endl;
        }
	}
	return 0;
}

int landtilehist( int argc, char **argv )
{
	USTRUCT_LAND_TILE landtile;
    
    open_uo_data_files();
    read_uo_data();

    typedef map<string, unsigned> M;
    M tilecount;

	int i;
	for( i = 0; i < 0x4000; i++ )
	{
		unsigned short objtype = (unsigned short) i;
		readlandtile( objtype, &landtile );
        tilecount[ landtile.name ] = tilecount[ landtile.name ] + 1;
	}

    for( M::iterator itr = tilecount.begin(); itr != tilecount.end(); ++itr )
    {
        cout << (*itr).first << ": " << (*itr).second << endl;
    }

	return 0;
}

int landtilecfg( int argc, char **argv )
{
	USTRUCT_LAND_TILE landtile;
    
    open_uo_data_files();
    read_uo_data();

    typedef map<string, unsigned> M;
    M tilecount;

	int i;
	for( i = 0; i < 0x4000; i++ )
	{
		unsigned short objtype = (unsigned short) i;
		readlandtile( objtype, &landtile );
        tilecount[ landtile.name ] = tilecount[ landtile.name ] + 1;
	}

    for( M::iterator itr = tilecount.begin(); itr != tilecount.end(); ++itr )
    {
        cout << (*itr).first << ": " << (*itr).second << endl;
    }

	return 0;
}

int flagsearch( int argc, char **argv )
{
	USTRUCT_TILE tile;
    
    open_uo_data_files();
    read_uo_data();
    
    if (argc < 3) return 1;

    unsigned long flags = strtoul( argv[2], NULL, 0 );
    unsigned long notflags = 0;
    if (argc >= 4)
        notflags = strtoul( argv[3], NULL, 0 );

	int i;
	for( i = 0; i <= 0xFFFF; i++ )
	{
		unsigned short objtype = (unsigned short) i;
		read_objinfo( objtype, tile );
        if ( ((tile.flags & flags) == flags) &&
             ((~tile.flags & notflags) == notflags) )
        {
            display_tileinfo( objtype, tile );
        }
	}
	return 0;
}

int landtileflagsearch( int argc, char **argv )
{
   
    open_uo_data_files();
    read_uo_data();
    
    if (argc < 3) return 1;

    unsigned long flags = strtoul( argv[2], NULL, 0 );
    unsigned long notflags = 0;
    if (argc >= 4)
        notflags = strtoul( argv[3], NULL, 0 );

	USTRUCT_LAND_TILE landtile;
    
	int i;
	for( i = 0; i < 0x4000; i++ )
	{
		unsigned short objtype = (unsigned short) i;
		readlandtile( objtype, &landtile );
        if ((landtile.flags & flags) == flags &&
            (~landtile.flags & notflags) == notflags)
        {
            cout << "Land Tile: " << hex << objtype << dec << endl;
            cout << "\tflags: " << hex << landtile.flags << dec << endl;
            cout << "\t  unk: " << hex << landtile.unk << dec << endl;
            cout << "\t name: " << landtile.name << endl;
        }
	}
	return 0;
}

int loschange( int argc, char **argv )
{
	USTRUCT_TILE tile;
    
    open_uo_data_files();
    read_uo_data();
    
	for( int i = 0; i <= 0xFFFF; i++ )
	{
		unsigned short objtype = (unsigned short) i;
		read_objinfo( objtype, tile );
        
        bool old_lostest = (tile.flags & USTRUCT_TILE::FLAG_WALKBLOCK) != 0;

        bool new_lostest = (tile.flags & (USTRUCT_TILE::FLAG_WALKBLOCK|USTRUCT_TILE::FLAG_NO_SHOOT)) != 0;


        if (old_lostest != new_lostest)
        {
            display_tileinfo( objtype, tile );
            printf( " Old LOS: %s\n", old_lostest?"true":"false" );
            printf( " New LOS: %s\n", new_lostest?"true":"false" );

        }
         
	}
	return 0;
}

int print_verdata_info()
{
    open_uo_data_files();
    read_uo_data();
    long int num_version_records, i;
    USTRUCT_VERSION vrec;

    // FIXME: should read this once per run, per file.
    fseek(verfile, 0, SEEK_SET);
    fread(&num_version_records, sizeof num_version_records, 1, verfile); // ENDIAN-BROKEN
    
    cout << "There are " << num_version_records << " version records." << endl;

    int filecount[ 32 ];
    int inv_filecount = 0;
    memset( filecount, 0, sizeof filecount );

    for (i = 0; i < num_version_records; i++)
    {
        fread(&vrec, sizeof vrec, 1, verfile);
        if (vrec.file < 32)
            ++filecount[ vrec.file ];
        else
            ++inv_filecount;
    }
    for( int i = 0; i < 32; ++i )
    {
        if (filecount[i])
            cout << "File " << hex << i << dec << ": " << filecount[i] << " versioned blocks." << endl;
    }
    if (inv_filecount)
        cout << inv_filecount << " invalid file indexes" << endl;
    return 0;
}

int print_statics()
{
    cout << "Reading UO data.." << endl;
    open_uo_data_files();
    read_uo_data();
    long water = 0;
    long strange_water = 0;
    long cnt = 0;
    for( u16 y = 30; y < 50; y++ )
    {
        for( u16 x = 5900; x < 5940; x++ )
        {
            std::vector<StaticRec> vec;
            readallstatics( vec, x, y );
            bool hdrshown = false;

            if (!vec.empty())
            {
                for( std::vector<StaticRec>::iterator itr = vec.begin(); itr != vec.end(); ++itr )
                {
                    int height = tileheight( (*itr).graphic );
                    if ((*itr).graphic >= 0x1796 && (*itr).graphic <= 0x17b2)
                    {
                        if ( (*itr).z == -5 && height == 0 )
                            water++;
                        else
                            strange_water++;
                        continue;
                    }
                    if (!hdrshown)
                        cout << x << "," << y << ":" << endl;
                    hdrshown = true;
                    cout << "\tOBJT= 0x" << hex << (*itr).graphic << dec 
                         << "  Z=" << int( (*itr).z ) << "  HT=" << height 
                         << "  FLAGS=" << hex << tile_uoflags( (*itr).graphic ) << dec
                         << endl;
                    ++cnt;
                }
            }
        }
    }
    cout << cnt << " statics exist." << endl;
    cout << water << " water tiles exist." << endl;
    cout << strange_water << " strange water tiles exist." << endl;
    return 0;
}



void elimdupes( StaticList& list )
{
    bool any = true;
    do
    {
        any = false;
        for( unsigned i = 0; !any && (i < list.size()); ++i )
        {
            for( unsigned j = 0; !any && (j < list.size()); ++j )
            {
                if (i == j)
                    continue;
                if (tile_uoflags(list[i].graphic) == tile_uoflags(list[j].graphic) &&
                    tileheight(list[i].graphic) == tileheight(list[j].graphic) &&
                    list[i].z == list[j].z)
                {
                    list[i] = list.back();
                    list.pop_back();
                    any = true;
                }
            }
        }
    } while (any);
}

#if 0
int test_new_statics()
{
    cout << "Reading UO data.." << endl;
    open_uo_data_files();
    read_uo_data();
    load_pol_static_files();

    for( unsigned x = 0; x < 6144; ++x )
    {
        for( unsigned y = 0; y < 4096; ++y )
        {
            StaticList list1, list2;
            readstatics( list1, x, y );
            readstatics2( list2, x, y );

			for( unsigned i = 0; i < list1.size(); ++i )
            {
                StaticRec& r1 = list1[i];
                // see if this is represented
                // really should check that there aren't any extras, too!
                bool found = false;
                for( unsigned j = 0; j < list2.size(); ++j )
                {
                    StaticRec& r2 = list2[j];
                    if (tileheight(r1.graphic) == tileheight(r2.graphic) &&
                        r1.z == r2.z)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    cout << "elem not found: x=" << x << ", y=" << y << ",i=" << i
                         << ", r1.graphic=" << r1.graphic << ", r1.z=" << int(r1.z)
                         << endl;
                    cout << "list1:" << endl;
                    for( unsigned j = 0; j < list1.size(); ++j )
                    {
                        cout << "gid=" << list1[j].graphic 
                             << " z=" << int(list1[j].z) 
                             << " ht=" << int(tileheight(list1[j].graphic))
                             << endl;
                    }
                    cout << "list2:" << endl;
                    for( unsigned j = 0; j < list2.size(); ++j )
                    {
                        cout << "gid=" << list2[j].graphic 
                             << " z=" << int(list2[j].z) 
                             << " ht=" << int(tileheight(list2[j].graphic))
                             << endl;
                    }
                }
            }
        }
    }
    return 0;
}
#endif

int rawdump( int argc, char **argv )
{
	if (argc != 5) 
		return Usage( 1 );
	
	FILE *fp = fopen( argv[2], "rb" );
	int hdrlen = atoi( argv[3] );
	int reclen = atoi( argv[4] );
	long int recnum = 0;
	if (!fp) 
		return 1;

	if (hdrlen)
	{
		if (fread( buffer, hdrlen, 1, fp ) != 1)
		{	
			return 1;
		}
		printf( "Header:\n" );
		fdump( stdout, buffer, hdrlen );
	}
	while (fread( buffer, reclen, 1, fp ) == 1)
	{
		printf( "Record %ld (%lx):\n", recnum, recnum );
		fdump( stdout, buffer, reclen );

		++recnum;
	}
	printf( "%ld records read.\n",recnum );
	return 0;
}

unsigned long read_ulong( istream& is )
{
    unsigned char a[4];
    is.read( (char *)a, sizeof a );
    return (a[3] << 24) | (a[2] << 16) | (a[1] << 8) | a[0];
}

int print_sndlist( int argc, char **argv )
{
        unsigned long offset;
        unsigned long length;
        unsigned long serial;
        char filename[ 15 ];

    string soundidxname = config.uo_datafile_root + "soundidx.mul";
    string soundname = config.uo_datafile_root + "sound.mul";
    ifstream soundidx( soundidxname.c_str(), ios::in | ios::binary );
    ifstream sound( soundname.c_str(), ios::in | ios::binary );
    int i;
    i = 0;
    while (soundidx.good())
    {
        ++i;
        offset = read_ulong( soundidx );
        length = read_ulong( soundidx );
        serial = read_ulong( soundidx );
        if (!soundidx.good()) 
            break;
        
        if (offset == 0xFFffFFffLu)
            continue;

        sound.seekg( offset, ios::beg );
        if (!sound.good())
            break;

        sound.read( filename, sizeof filename );
        cout << hex << i << ", " << hex << serial << dec << ": " << filename << endl;
    }
    return 0;
}

void read_multidefs();

void print_multihull( u16 i, MultiDef* multi)
{
    if (multi->hull.empty())
        return;

    USTRUCT_TILE tile;
    read_objinfo( i+0x4000, tile );
    cout << "Multi 0x" << hex << i+0x4000 << dec 
         << " -- " << tile.name << ":" << endl;
    for( int y = multi->minry; y <= multi->maxry; ++y )
    {
        for( int x = multi->minrx; x <= multi->maxrx; ++x )
        {
            unsigned short key = multi->getkey( x,y );
            bool external = multi->hull2.count(key) != 0;
            bool internal = multi->internal_hull2.count(key) != 0;
            bool origin = (x == 0 && y == 0);

            if (external && !internal)
                cout << 'H';
            else if (!external && internal)
                cout << 'i';
            else if (external && internal)
                cout << 'I';
            else if (origin)
                cout << '*';
            else
                cout << ' ';
        }
        cout << endl;
    }
    cout << endl;
}

void print_widedata( u16 i, MultiDef* multi )
{
    if (multi->hull.empty())
        return;

    USTRUCT_TILE tile;
    read_objinfo( i+0x4000, tile );
    cout << "Multi 0x" << hex << i+0x4000 << dec 
         << " -- " << tile.name << ":" << endl;
    for( int y = multi->minry; y <= multi->maxry; ++y )
    {
        for( int x = multi->minrx; x <= multi->maxrx; ++x )
        {
            const MULTI_ELEM* elem = multi->find_component( x, y );
            if (elem != NULL)
            {
                cout << hex << setw(8) << elem->is_static << dec << ":";
            }
            else
            {
                cout << hex << "        " << dec << ":";
            }
        }
        cout << endl;
    }
    cout << endl;
}

void print_multidata( u16 i, MultiDef* multi )
{
    if (multi->hull.empty())
        return;

    USTRUCT_TILE tile;
    read_objinfo( i+0x4000, tile );
    cout << "Multi 0x" << hex << i+0x4000 << dec 
         << " -- " << tile.name << ":" << endl;
    
    for( MultiDef::Components::iterator itr = multi->components.begin(), end = multi->components.end();
         itr != end;
         ++itr )
    {
        const MULTI_ELEM* elem = itr->second;
        cout << "0x" << hex << elem->objtype
             << " %d" << (int) elem->is_static 
             << ":" << dec <<  elem->x << "," << elem->y << "," << elem->z << endl;

    }
}

int print_multis(void)
{
    cout << "Reading UO data.." << endl;
    open_uo_data_files();
    read_uo_data();
    read_multidefs();

    for( u16 i = 0; i < 0x1000; ++i )
    {
        if (multidefs_by_multiid[i])
        {
            print_multihull( i, multidefs_by_multiid[i] );
            print_multidata( i, multidefs_by_multiid[i] );
        }
    }

    return 0;

}

int z_histogram()
{
    unsigned long zcount[256];
    cout << "Reading UO data.." << endl;
    open_uo_data_files();
    read_uo_data();
    for( u16 x = 0; x < 6143; ++x )
    {
        cout << ".";
        for( u16 y = 0; y < 4095; ++y )
        {
            USTRUCT_MAPINFO mi;
            int z;
            getmapinfo( x, y, &z, &mi );
            assert( z >= -128 && z <= 127 );
            ++zcount[ z+128 ];
        }
    }
    cout << endl;
    for( int i = 0; i < 256; ++i )
    {
        if (zcount[i])
            cout << i-128 << ": " << zcount[i] << endl;
    }
    return 0;
}

int statics_histogram()
{
    unsigned long counts[1000];
    memset( counts, 0, sizeof counts );
    cout << "Reading UO data.." << endl;
    open_uo_data_files();
    read_uo_data();
    for( u16 x = 0; x < 6143; x += 8 )
    {
        cout << ".";
        for( u16 y = 0; y < 4095; y += 8 )
        {
            USTRUCT_STATIC* p;
            int count;

            readstaticblock( &p, &count, x, y );
            if (count > 1000)
                cerr << "doh: count=" << count << endl;

            ++counts[ count ];

        }
    }
    cout << endl;
    for( int i = 0; i < 1000; ++i )
    {
        if (counts[i])
            cout << i << ": " << counts[i] << endl;
    }
    return 0;
}

int write_polmap( const char* filename, unsigned short xbegin, unsigned short xend)
{
    cout << "Writing " << filename << endl;
    FILE* fp = fopen( filename, "wb" );
    int num = xend+1-xbegin;
    for( u16 xs = xbegin; xs < xend; xs += 8 )
    {
        int percent = (xs-xbegin) * 100 / num;
        cout << "\r" << percent << "%";
        for( u16 ys = 0; ys < 4096; ys += 8 )
        {
            int z;
            USTRUCT_POL_MAPINFO_BLOCK blk;
            memset( &blk, 0, sizeof blk );
            for( u16 xi = 0; xi < 8; ++xi )
            {
                for( u16 yi = 0; yi < 8; ++yi )
                {
                    u16 x = xs+xi;
                    u16 y = ys+yi;
                    if (x == 6143) x = 6142;
                    if (y == 4095) y = 4094;
                    bool walkok = groundheight( x, y, &z );
                    blk.z[xi][yi] = static_cast<signed char>(z);
                    if (walkok)
                        blk.walkok[xi] |= (1 << yi);
                }
            }
            fwrite( &blk, sizeof blk, 1, fp );
        }
    }
    fclose(fp);
    return 0;
}

int write_polmap()
{
    cout << "Reading UO data.." << endl;
    open_uo_data_files();
    read_uo_data();
    write_polmap( "dngnmap0.pol", 5120, 6144 );
    write_polmap( "map0.pol", 0, 6144 );
    return 0;
}

int print_water_data()
{
    open_uo_data_files();
    readwater();
    return 0;
}

inline bool is_water( u16 objtype )
{
    return (objtype >= 0x1796 && objtype <= 0x17b2);
}
bool has_water( u16 x, u16 y )
{
    StaticList vec;
    vec.clear();
    readstatics( vec, x, y );
    for( StaticList::iterator itr = vec.begin(), end = vec.end(); itr != end; ++itr )
    {
        const StaticRec& rec = *itr;
        if (is_water(rec.graphic))
            return true;
    }
    return false;
}

u16 wxl = 1420, wxh = 1480, wyl=1760, wyh=1780;

int water_search( int argc, char *argv[])
{
    open_uo_data_files();
    for( u16 y = wyl; y < wyh; y++ )
    {
        for( u16 x = wxl; x < wxh; x++ )
        {
            if (has_water(x,y))
                cout << "W";
            else
                cout << ".";
        }
        cout << endl;
    }
    cout << endl;
    return 0;
}

void safe_getmapinfo( unsigned short x, unsigned short y, int* z, USTRUCT_MAPINFO* mi );

int mapdump( int argc, char* argv[] )
{
    u16 wxl = 5485, wxh = 5500, wyl=0, wyh=30;

    if (argc >= 4)
    {
        wxl = wxh = atoi( argv[2] );
        wyl = wyh = atoi( argv[3] );
    }
    if (argc >= 6)
    {
        wxh = atoi( argv[4] );
        wyh = atoi( argv[5] );
    }

    open_uo_data_files();
    read_uo_data();

    ofstream ofs( "mapdump.html" );

    ofs << "<table border=1 cellpadding=5 cellspacing=0>" << endl;
    ofs << "<tr><td>&nbsp;</td>";
    for( u16 x = wxl; x <= wxh; ++x )
    {
        ofs << "<td align=center>" << x << "</td>";
    }
    ofs << "</tr>" << endl;
    for( u16 y = wyl; y <= wyh; ++y )
    {
        ofs << "<tr><td valign=center>" << y << "</td>" << endl;
        for( u16 x = wxl; x <= wxh; ++x )
        {
            ofs << "<td align=left valign=top>";
            int z;
            USTRUCT_MAPINFO mi;
            safe_getmapinfo( x, y, &z, &mi );
            USTRUCT_LAND_TILE landtile;
            readlandtile( mi.landtile, &landtile );
            ofs << "z=" << z << "<br>";
            ofs << "landtile=" << hexint(mi.landtile) << " " << landtile.name << "<br>";
            ofs << "&nbsp;flags=" << hexint( landtile.flags ) << "<br>";
            ofs << "mapz=" << (int)mi.z << "<br>";
            
            StaticList statics;
            readallstatics( statics, x, y );
            if (!statics.empty())
            {
                ofs << "<table border=1 cellpadding=5 cellspacing=0>" << endl;
                ofs << "<tr><td>graphic</td><td>z</td><td>flags</td><td>ht</td>" << endl;
                for( unsigned i = 0; i < statics.size(); ++i )
                {
                    ofs << "<tr>";
                    StaticRec& rec = statics[i];
                    ofs << "<td>" << hexint( rec.graphic ) << "</td>";
                    ofs << "<td>" << int(rec.z) << "</td>";
                    ofs << "<td>" << hexint( rec.flags ) << "</td>";
                    ofs << "<td>" << int(rec.height) << "</td>";
                    ofs << "</tr>" << endl;
                }
                ofs << "</table>" << endl;
            }
            ofs << "</td>" << endl;
        }
        ofs << "</tr>" << endl;
    }
    ofs << "</table>" << endl;

    return 0;
}

struct MapContour
{
    signed char z[6144][4096];
};

int contour( int argc, char **argv )
{
    open_uo_data_files();
    read_uo_data();

    MapContour* mc = new MapContour;
    for( u16 y = 0; y < 4095; ++y )
    {
        for( u16 x = 0; x < 6143; ++x )
        {
            static StaticList vec;
    
            vec.clear();
    
            readstatics( vec, x, y );

            bool result;
            int newz;
            standheight(MOVEMODE_LAND, vec, x, y, 127, 
                        &result, &newz);
            if (result)
            {
                mc->z[x][y] = static_cast<signed char>(newz);
            }
            else
            {
                mc->z[x][y] = 127;
            }
        }
    }

    ofstream ofs( "contour.dat", ios::trunc|ios::out|ios::binary );
    ofs.write( reinterpret_cast<const char*>(mc), sizeof(MapContour) );

    return 0;
}

int findlandtile( int argc, char **argv )
{
    int landtile = strtoul( argv[1], NULL, 0 );
    open_uo_data_files();
    read_uo_data();

    for( u16 y = 0; y < 4095; ++y )
    {
        for( u16 x = 0; x < 6143; ++x )
        {
            int z;
            USTRUCT_MAPINFO mi;
            safe_getmapinfo( x, y, &z, &mi );
            if (mi.landtile == landtile)
            {
                cout << x << "," << y << "," << (int) mi.z;
                if (mi.z != z)
                    cout << " (" << z << ")";
                cout << endl;
            }
        }
    }


    return 0;
}

int findgraphic( int argc, char **argv )
{
    int graphic = strtoul( argv[1], NULL, 0 );
    cout << "Searching map for statics with graphic=" << hexint(graphic) << endl;

    open_uo_data_files();
    read_uo_data();

    for( u16 y = 0; y < 4095; ++y )
    {
        for( u16 x = 0; x < 6143; ++x )
        {
            StaticList statics;
            readallstatics( statics, x, y );
            for( unsigned i = 0; i < statics.size(); ++i )
            {
                StaticRec& rec = statics[i];
                if (rec.graphic == graphic)
                {
                    cout << x << "," << y << "," << rec.z << endl;
                }
            }
        }
    }

    return 0;
}

int findlandtileflags( int argc, char **argv )
{
    unsigned long flags = strtoul( argv[1], NULL, 0 );
    open_uo_data_files();
    read_uo_data();

    for( u16 y = 0; y < 4095; ++y )
    {
        for( u16 x = 0; x < 6143; ++x )
        {
            int z;
            USTRUCT_MAPINFO mi;
            safe_getmapinfo( x, y, &z, &mi );
            if (landtile_uoflags( mi.landtile ) & flags)
            {
                cout << x << "," << y << "," << (int) mi.z;
                cout << ": landtile " << hexint(mi.landtile)
                     << ", flags " << hexint( landtile_uoflags( mi.landtile ) );
                cout << endl;
            }
        }
    }


    return 0;
}


int main( int argc, char **argv )
{
    ConfigFile cf( "pol.cfg" );
    ConfigElem elem;

    cf.readraw( elem );

    config.uo_datafile_root = elem.remove_string( "UoDataFileRoot" );
    config.uo_datafile_root = normalized_dir_form( config.uo_datafile_root );
	if (argc <= 1)
		return Usage(1);

    if (argv[1][0] == '/' || argv[1][1] == ':')
    {
        config.uo_datafile_root = argv[1];
        --argc;
        ++argv;
    }

	if (stricmp( argv[1], "tiledump" ) == 0)
	{
		return tiledump( argc, argv );
	}
	else if (stricmp( argv[1], "vertile" ) == 0)
	{
		return vertile( argc, argv );
	}
    else if (stricmp( argv[1], "verlandtile" ) == 0)
    {
        return verlandtile( argc, argv );
    }
    else if (stricmp( argv[1], "landtilehist" ) == 0)
    {
        return landtilehist( argc, argv );
    }
	else if (stricmp( argv[1], "flagsearch" ) == 0)
	{
		return flagsearch( argc, argv );
	}
	else if (stricmp( argv[1], "landtileflagsearch" ) == 0)
	{
		return landtileflagsearch( argc, argv );
	}
    else if (stricmp( argv[1], "loschange" ) == 0)
    {
        return loschange( argc, argv );
    }
	else if (stricmp( argv[1], "rawdump" ) == 0)
	{
		return rawdump( argc, argv );
	}
    else if (stricmp( argv[1], "ctable" ) == 0)
    {
        return print_ctable();
    }
    else if (stricmp( argv[1], "sndlist" ) == 0)
    {
        return print_sndlist(argc, argv);
    }
    else if (stricmp( argv[1], "statics" ) == 0)
    {
        return print_statics();
    }
    else if (stricmp( argv[1], "verdata" ) == 0)
    {
        return print_verdata_info();
    }
    else if (stricmp( argv[1], "multis" ) == 0)
    {
        return print_multis();
    }
    else if (stricmp( argv[1], "water" ) == 0)
    {
        return print_water_data();
    }
    else if (stricmp( argv[1], "newstatics" ) == 0)
    {
        return write_pol_static_files( "main" );
    }
    else if (stricmp( argv[1], "staticsmax" ) == 0)
    {
        void staticsmax();
        open_uo_data_files();
        staticsmax();
        return 0;
    }
    else if (stricmp( argv[1], "watersearch" ) == 0)
    {
        return water_search(argc,argv);
    }
    else if (stricmp( argv[1], "zhist" ) == 0)
    {
        return z_histogram();
    }
    else if (stricmp( argv[1], "staticshist" ) == 0)
    {
        return statics_histogram();
    }
    else if (stricmp( argv[1], "writedungmap" ) == 0)
    {
        return write_polmap();
    }
    else if (stricmp( argv[1], "writekeys" ) == 0)
    {
        printf( "Keys written to current.key" );
        return 0;
    }
    else if (stricmp( argv[1], "mapdump" ) == 0)
    {
        return mapdump( argc, argv );
    }
    else if (stricmp( argv[1], "contour" ) == 0)
    {
        return contour( argc, argv );
    }
    else if (stricmp( argv[1], "findlandtile" ) == 0)
    {
        return findlandtile( argc-1, argv+1 );
    }
	else if (stricmp( argv[1], "findlandtileflags" ) == 0)
	{
		return findlandtileflags( argc-1, argv+1 );
	}
    else if (stricmp( argv[1], "findgraphic" ) == 0)
    {
        return findgraphic( argc-1, argv+1 );
    }
    
	return 0;
}
