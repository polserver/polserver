/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2009/11/23 Turley:    added staticdefrag to defrag/remove duplicate statics
2009/12/02 Turley:    added config.max_tile_id - Tomi

Notes
=======

*/

#include "../pol/uofile.h"
#include "../pol/polcfg.h"
#include "../pol/polfile.h"
#include "../pol/uofilei.h"
#include "../pol/multi/multidef.h"
#include "../pol/globals/multidefs.h"
#include "../pol/objtype.h"
#include "../pol/poltype.h"

#include "../plib/realmdescriptor.h"
#include "../plib/staticblock.h"
#include "../plib/systemstate.h"

#include "../clib/strutil.h"
#include "../clib/stlutil.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/cmdargs.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/fdump.h"
#include "../clib/passert.h"

#ifdef _MSC_VER
#pragma warning(disable:4996) // deprecation warning for fopen, sprintf, stricmp
#endif

namespace Pol {
  namespace Multi {
    void read_multidefs();
    bool BoatShapeExists( unsigned short /*graphic*/ )
    {
      return true;
    }
  }
  namespace Core {
    void safe_getmapinfo( unsigned short x, unsigned short y, short* z, USTRUCT_MAPINFO* mi );
    void staticsmax();
    void readwater();
  }
  namespace Uotool {

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
	};

	/* transforms table above into:
		{ nbits, bits_reversed [[ LSB .. MSB ]] },
		*/
	int print_ctable()
	{
      INFO_PRINT << "#include \"ctable.h\"\n\n"
        << "SVR_KEYDESC keydesc[ 257 ] = {\n";
      fmt::Writer _tmp;
	  for ( int i = 0; i <= 256; i++ )
	  {
		int nbits = keyid[i] & 0x0F;
		unsigned short inmask = 0x10;
		const unsigned short val = static_cast<const unsigned short>( keyid[i] );
		unsigned short valout = 0;
		while ( nbits-- )
		{
		  valout <<= 1;
		  if ( val & inmask )
			valout |= 1;
		  inmask <<= 1;
		}
        _tmp.Format( "    {{ {:>2}, 0x{:04X}, 0x{:04X}}},\n" )
          << ( keyid[i] & 0x0F )
          << ( keyid[i] >> 4 )
          << valout;
	  }
      _tmp << "};\n";
      INFO_PRINT << _tmp.str();
	  return 0;
	}

	unsigned char buffer[10000];

	int Usage( int ret )
	{
      ERROR_PRINT << "Usage: uotool func [params ...]\n"
        << "  Functions:\n"
        << "    tiledump filename        Dump object information\n"
        << "    vertile                  Dump updated object info\n"
        << "    rawdump filename hdrlen reclen\n"
        << "    ctable                   Print Server encryption table\n"
        << "    findgraphic graphic      finds coords of statics with graphic\n"
        << "    findlandtileflags flags  finds landtiles with flags\n"
        << "    contour                  write binary file 6144x4096 z heights\n"
        << "    mapdump x1 y1 [x2 y2]    dumps map+statics info to html tables\n"
        << "    landtileflagsearch flags [notflags]  prints landtiles matching args\n"
        << "    flagsearch flags [notflags]          prints tiles matching flags\n"
        << "    landtilehist             prints landtile histogram\n"
        << "    staticshist              histogram of number of statics at a location\n"
        << "    zhist                    histogram of map z\n"
        << "    multis                   prints multi definitions \n"
        << "    verdata                  prints verdata info\n"
        << "    statics                  prints statics info\n"
        << "    sndlist                  prints sound list info\n"
        << "    verlandtile              prints verdata landtile info\n"
        << "    loschange                prints differences in LOS handling \n"
        << "    staticdefrag [realm]     recreates static files {default britannia} \n"
        << "    formatdesc name          prints plural and singular form of name \n"
        << "    checkmultis              prints infos about multi center items \n";
	  return ret;
	}
#define TILES_START 0x68800

	void display_tileinfo( unsigned short objtype, const Core::USTRUCT_TILE& tile )
	{
      fmt::Writer _tmp;
      _tmp.Format( "objtype:  0x{:04X}\n" ) << objtype;
      _tmp << "  name:   " << tile.name << "\n";
      if ( tile.flags )  _tmp.Format( "  flags:  0x{:08X}\n" ) << static_cast<unsigned long>( tile.flags );
      if ( tile.weight ) _tmp.Format( "  weight: 0x{:02X}\n" ) << (int)tile.weight;
      if ( tile.layer )  _tmp.Format( "  layer:  0x{:02X}\n" ) << (int)tile.layer;
      if ( tile.unk6 )   _tmp.Format( "  unk6:   0x{:02X}\n" ) << (int)tile.unk6;
      if ( tile.unk7 )   _tmp.Format( "  unk7:   0x{:02X}\n" ) << (int)tile.unk7;
      if ( tile.unk8 )   _tmp.Format( "  unk8:   0x{:02X}\n" ) << (int)tile.unk8;
      if ( tile.unk9 )   _tmp.Format( "  unk9:   0x{:02X}\n" ) << (int)tile.unk9;
      if ( tile.anim )   _tmp.Format( "  anim:   0x{:08X}\n" ) << static_cast<unsigned long>( tile.anim );
      if ( tile.height ) _tmp.Format( "  height: 0x{:02X}\n" ) << (int)tile.height;
      if ( tile.unk14 )  _tmp.Format( "  unk14:  0x{:02X}\n" ) << (int)tile.unk14;
      if ( tile.unk15 )  _tmp.Format( "  unk15:  0x{:02X}\n" ) << (int)tile.unk15;
	}

	int tiledump( int argc, char **argv )
	{
	  Core::USTRUCT_TILE tile;
	  u32 version;
	  if ( argc != 3 )
		return Usage( 1 );

	  FILE *fp = fopen( argv[2], "rb" );
	  fseek( fp, TILES_START, SEEK_SET );
	  int recnum = 0;
	  unsigned short objtype = 0;
	  for ( ;; )
	  {
		if ( recnum == 0 )
		{
		  if ( fread( &version, sizeof version, 1, fp ) != 1 )
			break;
          INFO_PRINT.Format( "Block Version: {:08X}\n" ) << static_cast<unsigned long>( version );
		}
		if ( fread( &tile, sizeof tile, 1, fp ) != 1 )
		  break;
		display_tileinfo( objtype, tile );


		++objtype;
		++recnum;
		if ( recnum == 32 )
		  recnum = 0;

	  }
	  fclose( fp );
	  return 0;
	}

	int vertile( int /*argc*/, char ** /*argv*/ )
	{
	  Core::USTRUCT_TILE tile;

	  Core::open_uo_data_files();
      Core::read_uo_data( );

	  int i;
	  for ( i = 0; i <= 0xFFFF; i++ )
	  {
		unsigned short objtype = (unsigned short)i;
		read_objinfo( objtype, tile );
		display_tileinfo( objtype, tile );
	  }
      Core::clear_tiledata( );
	  return 0;
	}

	int verlandtile( int /*argc*/, char ** /*argv*/ )
	{
	  Core::USTRUCT_LAND_TILE landtile;

      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  int i;
	  for ( i = 0; i <= 0x3FFF; i++ )
	  {
		unsigned short objtype = (unsigned short)i;
		readlandtile( objtype, &landtile );
		if ( landtile.flags || landtile.unk || landtile.name[0] )
		{
          INFO_PRINT << "Land Tile: 0x" << fmt::hexu(objtype) << "\n"
          << "\tflags: 0x" << fmt::hexu( landtile.flags ) << "\n"
          << "\t  unk: 0x" << fmt::hexu( landtile.unk ) << "\n"
          << "\t name: " << landtile.name << "\n";
		}
	  }
      Core::clear_tiledata( );
	  return 0;
	}

	int landtilehist( int /*argc*/, char** /*argv*/ )
	{
	  Core::USTRUCT_LAND_TILE landtile;

      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  typedef std::map<std::string, unsigned> M;
	  M tilecount;

	  int i;
	  for ( i = 0; i < 0x4000; i++ )
	  {
		unsigned short objtype = (unsigned short)i;
		readlandtile( objtype, &landtile );
		tilecount[landtile.name] = tilecount[landtile.name] + 1;
	  }

      fmt::Writer tmp;
	  for ( const auto &elem : tilecount )
	  {
        tmp << elem.first << ": " << elem.second << "\n";
	  }
      INFO_PRINT << tmp.str();
      Core::clear_tiledata( );
	  return 0;
	}

	int landtilecfg( int /*argc*/, char** /*argv*/ )
	{
	  Core::USTRUCT_LAND_TILE landtile;

      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  typedef std::map<std::string, unsigned> M;
	  M tilecount;

	  int i;
	  for ( i = 0; i < 0x4000; i++ )
	  {
		unsigned short objtype = (unsigned short)i;
		readlandtile( objtype, &landtile );
		tilecount[landtile.name] = tilecount[landtile.name] + 1;
	  }

      fmt::Writer tmp;
	  for ( const auto &elem : tilecount )
	  {
		tmp << elem.first << ": " << elem.second << "\n";
	  }
      INFO_PRINT << tmp.str();
      Core::clear_tiledata( );
	  return 0;
	}

	int flagsearch( int argc, char **argv )
	{
	  Core::USTRUCT_TILE tile;

      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  if ( argc < 3 ) return 1;

	  unsigned int flags = strtoul( argv[2], NULL, 0 );
	  unsigned int notflags = 0;
	  if ( argc >= 4 )
		notflags = strtoul( argv[3], NULL, 0 );

	  int i;
	  for ( i = 0; i <= 0xFFFF; i++ )
	  {
		unsigned short objtype = (unsigned short)i;
		read_objinfo( objtype, tile );
		if ( ( ( tile.flags & flags ) == flags ) &&
			 ( ( ~tile.flags & notflags ) == notflags ) )
		{
		  display_tileinfo( objtype, tile );
		}
	  }
      Core::clear_tiledata( );
	  return 0;
	}

	int landtileflagsearch( int argc, char **argv )
	{

      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  if ( argc < 3 ) return 1;

	  unsigned int flags = strtoul( argv[2], NULL, 0 );
	  unsigned int notflags = 0;
	  if ( argc >= 4 )
		notflags = strtoul( argv[3], NULL, 0 );

	  Core::USTRUCT_LAND_TILE landtile;

	  int i;
	  for ( i = 0; i < 0x4000; i++ )
	  {
		unsigned short objtype = (unsigned short)i;
		readlandtile( objtype, &landtile );
		if ( ( landtile.flags & flags ) == flags &&
			 ( ~landtile.flags & notflags ) == notflags )
		{
          INFO_PRINT << "Land Tile: 0x" << fmt::hexu( objtype ) << "\n"
            << "\tflags: 0x" << fmt::hexu( landtile.flags ) << "\n"
            << "\t  unk: 0x" << fmt::hexu( landtile.unk ) << "\n"
            << "\t name: " << landtile.name << "\n";
		}
	  }
      Core::clear_tiledata( );
	  return 0;
	}

	int loschange( int /*argc*/, char** /*argv*/ )
	{
	  Core::USTRUCT_TILE tile;

      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  for ( int i = 0; i <= 0xFFFF; i++ )
	  {
		unsigned short objtype = (unsigned short)i;
		read_objinfo( objtype, tile );

		bool old_lostest = ( tile.flags & Core::USTRUCT_TILE::FLAG_WALKBLOCK ) != 0;

		bool new_lostest = ( tile.flags & ( Core::USTRUCT_TILE::FLAG_WALKBLOCK | Core::USTRUCT_TILE::FLAG_NO_SHOOT ) ) != 0;


		if ( old_lostest != new_lostest )
		{
		  display_tileinfo( objtype, tile );
          INFO_PRINT.Format( " Old LOS: %s\n New LOS: %s\n" ) << ( old_lostest ? "true" : "false" )
            << ( new_lostest ? "true" : "false" );

		}

	  }
      Core::clear_tiledata( );
	  return 0;
	}

	int print_verdata_info()
	{
      Core::open_uo_data_files( );
      Core::read_uo_data( );
	  int num_version_records;
	  Core::USTRUCT_VERSION vrec;

	  // FIXME: should read this once per run, per file.
      fseek( Core::verfile, 0, SEEK_SET );
      fread( &num_version_records, sizeof num_version_records, 1, Core::verfile ); // ENDIAN-BROKEN

      INFO_PRINT << "There are " << num_version_records << " version records.\n";

	  int filecount[32];
	  int inv_filecount = 0;
	  memset( filecount, 0, sizeof filecount );

	  for ( int i = 0; i < num_version_records; i++ )
	  {
        fread( &vrec, sizeof vrec, 1, Core::verfile );
		if ( vrec.file < 32 )
		  ++filecount[vrec.file];
		else
		  ++inv_filecount;
	  }
	  for ( int i = 0; i < 32; ++i )
	  {
        if ( filecount[i] )
          INFO_PRINT << "File 0x" << fmt::hexu( i ) << ": " << filecount[i] << " versioned blocks.\n";
	  }
      if ( inv_filecount )
        INFO_PRINT << inv_filecount << " invalid file indexes\n";
      Core::clear_tiledata( );
	  return 0;
	}

	int print_statics()
	{
      INFO_PRINT << "Reading UO data..\n";
      Core::open_uo_data_files( );
      Core::read_uo_data( );
	  int water = 0;
	  int strange_water = 0;
	  int cnt = 0;
	  for ( u16 y = 30; y < 50; y++ )
	  {
		for ( u16 x = 5900; x < 5940; x++ )
		{
          std::vector<Core::StaticRec> vec;
          Core::readallstatics( vec, x, y );

		  if ( !vec.empty() )
		  {
			bool hdrshown = false;
			for ( const auto &elem : vec )
			{
              int height = Core::tileheight( elem.graphic );
			  if ( elem.graphic >= 0x1796 && elem.graphic <= 0x17b2 )
			  {
				if ( elem.z == -5 && height == 0 )
				  water++;
				else
				  strange_water++;
				continue;
			  }
              if ( !hdrshown )
                INFO_PRINT << x << "," << y << ":\n";
			  hdrshown = true;
              INFO_PRINT << "\tOBJT= 0x" << fmt::hexu( elem.graphic )
                << "  Z=" << int( elem.z ) << "  HT=" << height
                << "  FLAGS=0x" << fmt::hexu( Core::tile_uoflags( elem.graphic ) ) << "\n";
			  ++cnt;
			}
		  }
		}
	  }
      INFO_PRINT << cnt << " statics exist.\n"
        << water << " water tiles exist.\n"
        << strange_water << " strange water tiles exist.\n";
      Core::clear_tiledata( );
	  return 0;
	}



    void elimdupes( Core::StaticList& list )
	{
	  bool any = true;
	  do
	  {
		any = false;
		for ( unsigned i = 0; !any && ( i < list.size() ); ++i )
		{
		  for ( unsigned j = 0; !any && ( j < list.size() ); ++j )
		  {
			if ( i == j )
			  continue;
            if ( Core::tile_uoflags( list[i].graphic ) == Core::tile_uoflags( list[j].graphic ) &&
                 Core::tileheight( list[i].graphic ) == Core::tileheight( list[j].graphic ) &&
				 list[i].z == list[j].z )
			{
			  list[i] = list.back();
			  list.pop_back();
			  any = true;
			}
		  }
		}
	  } while ( any );
	}

#if 0
	int test_new_statics()
	{
      INFO_PRINT << "Reading UO data..\n";
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
              INFO_PRINT << "elem not found: x=" << x << ", y=" << y << ",i=" << i
				<< ", r1.graphic=" << r1.graphic << ", r1.z=" << int(r1.z)
				<< "\n";
              INFO_PRINT << "list1:\n";
			  for( unsigned j = 0; j < list1.size(); ++j )
			  {
                INFO_PRINT << "gid=" << list1[j].graphic 
				  << " z=" << int(list1[j].z) 
				  << " ht=" << int(tileheight(list1[j].graphic))
				  << "\n";
			  }
              INFO_PRINT << "list2:\n";
			  for( unsigned j = 0; j < list2.size(); ++j )
			  {
                INFO_PRINT << "gid=" << list2[j].graphic 
				  << " z=" << int(list2[j].z) 
				  << " ht=" << int(tileheight(list2[j].graphic))
				  << "\n";
			  }
			}
		  }
		}
	  }
	  clear_tiledata();
	  return 0;
	}
#endif

	int rawdump( int argc, char **argv )
	{
	  if ( argc != 5 )
		return Usage( 1 );

	  FILE *fp = fopen( argv[2], "rb" );
	  int hdrlen = atoi( argv[3] );
	  int reclen = atoi( argv[4] );
	  int recnum = 0;
	  if ( !fp )
		return 1;

	  if ( hdrlen )
	  {
		if ( fread( buffer, hdrlen, 1, fp ) != 1 )
		{
		  fclose( fp );
		  return 1;
		}
        fmt::Writer tmp;
        tmp << "Header:\n";
        Clib::fdump( tmp, buffer, hdrlen );
        INFO_PRINT << tmp.str() << "\n";
	  }

	  while ( fread( buffer, reclen, 1, fp ) == 1 )
	  {
        fmt::Writer tmp;
        tmp.Format( "Record {} (0x{:X}):\n" ) << recnum << recnum;
        Clib::fdump( tmp, buffer, reclen );
        INFO_PRINT << tmp.str( ) << "\n";

		++recnum;
	  }
      INFO_PRINT << recnum << " records read.\n";
	  fclose( fp );
	  return 0;
	}

	unsigned int read_ulong( std::istream& is )
	{
	  unsigned char a[4];
	  is.read( (char *)a, sizeof a );
	  return ( a[3] << 24 ) | ( a[2] << 16 ) | ( a[1] << 8 ) | a[0];
	}

	int print_sndlist( int /*argc*/, char** /*argv*/ )
	{
	  unsigned int offset;
	  unsigned int length;
	  unsigned int serial;
	  char filename[15];

	  std::string soundidxname = Plib::systemstate.config.uo_datafile_root + "soundidx.mul";
      std::string soundname = Plib::systemstate.config.uo_datafile_root + "sound.mul";
      std::ifstream soundidx(soundidxname.c_str(), std::ios::in | std::ios::binary);
      std::ifstream sound(soundname.c_str(), std::ios::in | std::ios::binary);
	  int i;
	  i = 0;
	  while ( soundidx.good() )
	  {
		++i;
		offset = read_ulong( soundidx );
		length = read_ulong( soundidx );
		serial = read_ulong( soundidx );
		if ( !soundidx.good() )
		  break;

		if ( offset == 0xFFffFFffLu )
		  continue;

        sound.seekg(offset, std::ios::beg);
		if ( !sound.good() )
		  break;

		sound.read( filename, sizeof filename );
        INFO_PRINT << "0x" << fmt::hexu( i ) << ", 0x" << fmt::hexu( serial ) << ": " << filename << "\n"
          << "len " << length << "\n";
	  }
	  return 0;
	}

	void print_multihull( u16 i, Multi::MultiDef* multi )
	{
	  if ( multi->hull.empty() )
		return;

	  Core::USTRUCT_TILE tile;
      read_objinfo( static_cast<u16>( i + ( Plib::systemstate.config.max_tile_id + 1 ) ), tile );
      INFO_PRINT << "Multi 0x" << fmt::hexu (i + i + ( Plib::systemstate.config.max_tile_id + 1 ) )
		<< " -- " << tile.name << ":\n";
      fmt::Writer tmp;
	  for ( short y = multi->minry; y <= multi->maxry; ++y )
	  {
		for ( short x = multi->minrx; x <= multi->maxrx; ++x )
		{
		  unsigned short key = multi->getkey( x, y );
		  bool external = multi->hull2.count( key ) != 0;
		  bool internal = multi->internal_hull2.count( key ) != 0;
		  bool origin = ( x == 0 && y == 0 );

		  if ( external && !internal )
            tmp << 'H';
		  else if ( !external && internal )
            tmp << 'i';
		  else if ( external && internal )
            tmp << 'I';
		  else if ( origin )
            tmp << '*';
		  else
            tmp << ' ';
		}
        tmp << "\n";
	  }
      tmp << "\n";
      INFO_PRINT << tmp.str( );
	}

	void print_widedata( u16 i, Multi::MultiDef* multi )
	{
	  if ( multi->hull.empty() )
		return;

	  Core::USTRUCT_TILE tile;
      read_objinfo( static_cast<u16>( i + ( Plib::systemstate.config.max_tile_id + 1 ) ), tile );
      INFO_PRINT << "Multi 0x" << fmt::hexu( i + ( Plib::systemstate.config.max_tile_id + 1 ) )
		<< " -- " << tile.name << ":\n";
      fmt::Writer tmp;
	  for ( short y = multi->minry; y <= multi->maxry; ++y )
	  {
		for ( short x = multi->minrx; x <= multi->maxrx; ++x )
		{
		  const Multi::MULTI_ELEM* elem = multi->find_component( x, y );
		  if ( elem != NULL )
		  {
            tmp << "0x" << fmt::pad( fmt::hexu( elem->is_static ), 6 ) << ":";
		  }
		  else
		  {
            tmp << "        " << ":";
		  }
		}
        tmp << "\n";
	  }
      tmp << "\n";
      INFO_PRINT << tmp.str();
	}

	void print_multidata( u16 i, Multi::MultiDef* multi )
	{
	  if ( multi->hull.empty() )
		return;

	  Core::USTRUCT_TILE tile;
      read_objinfo( static_cast<u16>( i + ( Plib::systemstate.config.max_tile_id + 1 ) ), tile );
      INFO_PRINT << "Multi 0x" << fmt::hexu( i + ( Plib::systemstate.config.max_tile_id + 1 ) )
        << " -- " << tile.name << ":\n";
      fmt::Writer tmp;
	  for ( const auto &_itr : multi->components )
	  {
		const Multi::MULTI_ELEM* elem = _itr.second;
        tmp << "0x" << fmt::hexu( elem->objtype)
		  << " 0x" << fmt::hexu((int)elem->is_static)
		  << ":"  << elem->x << "," << elem->y << "," << elem->z << "\n";
	  }
      INFO_PRINT << tmp.str();
	}

	int print_multis( void )
	{
      INFO_PRINT << "Reading UO data..\n";
      Core::open_uo_data_files( );
      Core::read_uo_data( );
	  Multi::read_multidefs();

	  for ( u16 i = 0; i < 0x1000; ++i )
	  {
		if ( Multi::multidef_buffer.multidefs_by_multiid[i] )
		{
		  print_multihull( i, Multi::multidef_buffer.multidefs_by_multiid[i] );
		  print_multidata( i, Multi::multidef_buffer.multidefs_by_multiid[i] );
		}
	  }
      Core::clear_tiledata( );
	  return 0;

	}

	int z_histogram()
	{
	  unsigned int zcount[256];
      memset(zcount, 0, sizeof(zcount));

      INFO_PRINT << "Reading UO data..\n";
      Core::open_uo_data_files( );
      Core::read_uo_data( );
	  for ( u16 x = 0; x < 6143; ++x )
	  {
        INFO_PRINT << ".";
		for ( u16 y = 0; y < 4095; ++y )
		{
		  Core::USTRUCT_MAPINFO mi;
		  short z;
		  getmapinfo( x, y, &z, &mi );
		  assert( z >= Core::ZCOORD_MIN && z <= Core::ZCOORD_MAX );
		  ++zcount[z + 128];
		}
	  }
      INFO_PRINT << "\n";
	  for ( int i = 0; i < 256; ++i )
	  {
		if ( zcount[i] )
          INFO_PRINT << i - 128 << ": " << zcount[i] << "\n";
	  }
      Core::clear_tiledata( );
	  return 0;
	}

	int statics_histogram()
	{
	  unsigned int counts[1000];
	  memset( counts, 0, sizeof counts );
      INFO_PRINT << "Reading UO data..\n";
      Core::open_uo_data_files( );
      Core::read_uo_data( );
	  for ( u16 x = 0; x < 6143; x += 8 )
	  {
        INFO_PRINT << ".";
		for ( u16 y = 0; y < 4095; y += 8 )
		{
		  std::vector<Core::USTRUCT_STATIC> p;
		  int count;

		  readstaticblock( &p, &count, x, y );
		  if ( count < 1000 )
              ++counts[count];
          else
              ERROR_PRINT << "doh: count=" << count << "\n";
		}
	  }
      INFO_PRINT << "\n";
	  for ( int i = 0; i < 1000; ++i )
	  {
		if ( counts[i] )
          INFO_PRINT << i << ": " << counts[i] << "\n";
	  }
      Core::clear_tiledata( );
	  return 0;
	}

	int write_polmap( const char* filename, unsigned short xbegin, unsigned short xend )
	{
      INFO_PRINT << "Writing " << filename << "\n";
	  FILE* fp = fopen( filename, "wb" );
	  int num = xend + 1 - xbegin;
	  for ( u16 xs = xbegin; xs < xend; xs += 8 )
	  {
		int percent = ( xs - xbegin ) * 100 / num;
        INFO_PRINT << "\r" << percent << "%";
		for ( u16 ys = 0; ys < 4096; ys += 8 )
		{
		  short z;
		  Core::USTRUCT_POL_MAPINFO_BLOCK blk;
		  memset( &blk, 0, sizeof blk );
		  for ( u16 xi = 0; xi < 8; ++xi )
		  {
			for ( u16 yi = 0; yi < 8; ++yi )
			{
			  u16 x = xs + xi;
			  u16 y = ys + yi;
			  if ( x == 6143 ) x = 6142;
			  if ( y == 4095 ) y = 4094;
              bool walkok = Core::groundheight( x, y, &z );
			  blk.z[xi][yi] = static_cast<signed char>( z );
			  if ( walkok )
				blk.walkok[xi] |= ( 1 << yi );
			}
		  }
		  fwrite( &blk, sizeof blk, 1, fp );
		}
	  }
	  fclose( fp );
	  return 0;
	}

	int write_polmap()
	{
      INFO_PRINT << "Reading UO data..\n";
      Core::open_uo_data_files( );
      Core::read_uo_data( );
	  write_polmap( "dngnmap0.pol", 5120, 6144 );
	  write_polmap( "map0.pol", 0, 6144 );
      Core::clear_tiledata( );
	  return 0;
	}

	int print_water_data()
	{
      Core::open_uo_data_files( );
      Core::readwater( );
	  return 0;
	}

	inline bool is_water( u16 objtype )
	{
	  return ( objtype >= 0x1796 && objtype <= 0x17b2 );
	}
	bool has_water( u16 x, u16 y )
	{
      Core::StaticList vec;
	  vec.clear();
      Core::readstatics( vec, x, y );
	  for ( const auto &rec : vec )
	  {
		if ( is_water( rec.graphic ) )
		  return true;
	  }
	  return false;
	}

	int water_search( int /*argc*/, char** /*argv*/ )
	{
	  u16 wxl = 1420, wxh = 1480, wyl = 1760, wyh = 1780;
      Core::open_uo_data_files( );
	  for ( u16 y = wyl; y < wyh; y++ )
	  {
		for ( u16 x = wxl; x < wxh; x++ )
		{
		  if ( has_water( x, y ) )
            INFO_PRINT << "W";
		  else
            INFO_PRINT << ".";
		}
        INFO_PRINT << "\n";
	  }
      INFO_PRINT << "\n";
	  return 0;
	}

	int mapdump( int argc, char* argv[] )
	{
	  u16 wxl = 5485, wxh = 5500, wyl = 0, wyh = 30;

	  if ( argc >= 4 )
	  {
		wxl = wxh = static_cast<u16>( atoi( argv[2] ) );
		wyl = wyh = static_cast<u16>( atoi( argv[3] ) );
	  }
	  if ( argc >= 6 )
	  {
		wxh = static_cast<u16>( atoi( argv[4] ) );
		wyh = static_cast<u16>( atoi( argv[5] ) );
	  }

      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  std::ofstream ofs( "mapdump.html" );

	  ofs << "<table border=1 cellpadding=5 cellspacing=0>" << std::endl;
	  ofs << "<tr><td>&nbsp;</td>";
	  for ( u16 x = wxl; x <= wxh; ++x )
	  {
		ofs << "<td align=center>" << x << "</td>";
	  }
      ofs << "</tr>" << std::endl;
	  for ( u16 y = wyl; y <= wyh; ++y )
	  {
          ofs << "<tr><td valign=center>" << y << "</td>" << std::endl;
		for ( u16 x = wxl; x <= wxh; ++x )
		{
		  ofs << "<td align=left valign=top>";
		  short z;
		  Core::USTRUCT_MAPINFO mi;
		  safe_getmapinfo( x, y, &z, &mi );
		  Core::USTRUCT_LAND_TILE landtile;
		  readlandtile( mi.landtile, &landtile );
		  ofs << "z=" << z << "<br>";
		  ofs << "landtile=" << Clib::hexint( mi.landtile ) << " " << landtile.name << "<br>";
		  ofs << "&nbsp;flags=" << Clib::hexint( landtile.flags ) << "<br>";
		  ofs << "mapz=" << (int)mi.z << "<br>";

          Core::StaticList statics;
		  readallstatics( statics, x, y );
		  if ( !statics.empty() )
		  {
              ofs << "<table border=1 cellpadding=5 cellspacing=0>" << std::endl;
              ofs << "<tr><td>graphic</td><td>z</td><td>flags</td><td>ht</td>" << std::endl;
			for ( const auto &rec : statics )
			{
			  ofs << "<tr>";
			  ofs << "<td>" << Clib::hexint( rec.graphic ) << "</td>";
			  ofs << "<td>" << int( rec.z ) << "</td>";
			  ofs << "<td>" << Clib::hexint( rec.flags ) << "</td>";
			  ofs << "<td>" << int( rec.height ) << "</td>";
			  ofs << "</tr>" << std::endl;
			}
			ofs << "</table>" << std::endl;
		  }
		  ofs << "</td>" << std::endl;
		}
		ofs << "</tr>" << std::endl;
	  }
	  ofs << "</table>" << std::endl;
      Core::clear_tiledata( );
	  return 0;
	}

	struct MapContour
	{
	  signed char z[6144][4096];
	};

	int contour( int /*argc*/, char** /*argv*/ )
	{
      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  auto mc = new MapContour;
	  for ( u16 y = 0; y < 4095; ++y )
	  {
		for ( u16 x = 0; x < 6143; ++x )
		{
          static Core::StaticList vec;

		  vec.clear();

		  readstatics( vec, x, y );

		  bool result;
		  short newz;
          standheight( Core::MOVEMODE_LAND, vec, x, y, 127,
					   &result, &newz );
		  if ( result )
		  {
			mc->z[x][y] = static_cast<signed char>( newz );
		  }
		  else
		  {
			mc->z[x][y] = 127;
		  }
		}
	  }

      std::ofstream ofs("contour.dat", std::ios::trunc | std::ios::out | std::ios::binary);
	  ofs.write( reinterpret_cast<const char*>( mc ), sizeof( MapContour ) );
      Core::clear_tiledata( );
	  delete mc;
	  return 0;
	}

	int findlandtile( int /*argc*/, char **argv )
	{
	  int landtile = strtoul( argv[1], NULL, 0 );
      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  for ( u16 y = 0; y < 4095; ++y )
	  {
		for ( u16 x = 0; x < 6143; ++x )
		{
		  short z;
		  Core::USTRUCT_MAPINFO mi;
		  safe_getmapinfo( x, y, &z, &mi );
		  if ( mi.landtile == landtile )
		  {
            INFO_PRINT << x << "," << y << "," << (int)mi.z;
			if ( mi.z != z )
              INFO_PRINT << " (" << z << ")";
            INFO_PRINT << "\n";
		  }
		}
	  }

      Core::clear_tiledata( );
	  return 0;
	}

	int findgraphic( int /*argc*/, char **argv )
	{
	  int graphic = strtoul( argv[1], NULL, 0 );
      INFO_PRINT << "Searching map for statics with graphic=0x" << fmt::hexu( graphic ) << "\n";

      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  for ( u16 y = 0; y < 4095; ++y )
	  {
		for ( u16 x = 0; x < 6143; ++x )
		{
          Core::StaticList statics;
		  readallstatics( statics, x, y );
		  for ( const auto &rec : statics )
		  {
			if ( rec.graphic == graphic )
			{
              INFO_PRINT << x << "," << y << "," << rec.z << "\n";
			}
		  }
		}
	  }
      Core::clear_tiledata( );
	  return 0;
	}

	int findlandtileflags( int /*argc*/, char **argv )
	{
	  unsigned int flags = strtoul( argv[1], NULL, 0 );
      Core::open_uo_data_files( );
      Core::read_uo_data( );

	  for ( u16 y = 0; y < 4095; ++y )
	  {
		for ( u16 x = 0; x < 6143; ++x )
		{
		  short z;
		  Core::USTRUCT_MAPINFO mi;
		  safe_getmapinfo( x, y, &z, &mi );
          if ( Core::landtile_uoflags( mi.landtile ) & flags )
		  {
            INFO_PRINT << x << "," << y << "," << (int)mi.z
              << ": landtile 0x" << fmt::hexu( mi.landtile )
              << ", flags 0x" << fmt::hexu( Core::landtile_uoflags( mi.landtile ) )
              << "\n";
		  }
		}
	  }

      Core::clear_tiledata( );
	  return 0;
	}

	int defragstatics( int argc, char **argv )
	{
	  const char* realm;
	  if ( argc < 2 )
		realm = "britannia";
	  else
		realm = argv[1];

	  Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realm );

      Core::uo_mapid = descriptor.uomapid;
      Core::uo_usedif = descriptor.uodif;
      Core::uo_map_width = static_cast<unsigned short>( descriptor.width );
      Core::uo_map_height = static_cast<unsigned short>( descriptor.height );

      Core::open_uo_data_files( );
      Core::read_uo_data( );

      std::string statidx = "staidx" + Clib::tostring(Core::uo_mapid) + ".mul";
      std::string statics = "statics" + Clib::tostring(Core::uo_mapid) + ".mul";
	  Clib::RemoveFile( statidx );
	  Clib::RemoveFile( statics );

	  FILE* fidx = fopen( statidx.c_str(), "wb" );
	  FILE* fmul = fopen( statics.c_str(), "wb" );

	  int lastprogress = -1;
	  for ( u16 x = 0; x < descriptor.width; x += Plib::STATICBLOCK_CHUNK )
	  {
		int progress = x * 100L / descriptor.width;
		if ( progress != lastprogress )
		{
          INFO_PRINT << "\rRewriting statics files: " << progress << "%";
		  lastprogress = progress;
		}
		for ( u16 y = 0; y < descriptor.height; y += Plib::STATICBLOCK_CHUNK )
		{
            std::vector<Core::USTRUCT_STATIC> pstat;
		  int num;
          std::vector<Core::USTRUCT_STATIC> tilelist;
		  readstaticblock( &pstat, &num, x, y );
		  if ( num>0 )
		  {
			int currwritepos = ftell( fmul );
			for ( int i = 0; i < num; ++i )
			{
			  Core::USTRUCT_STATIC& tile = pstat[i];
			  if ( tile.graphic < 0x4000 )
			  {
				bool first = true;
				for ( const auto &stile : tilelist )
				{
				  if ( ( tile.graphic == stile.graphic )
					   && ( tile.x_offset == stile.x_offset )
					   && ( tile.y_offset == stile.y_offset )
					   && ( tile.z == stile.z )
					   && ( tile.hue == stile.hue ) )
				  {
					first = false;
					break;
				  }
				}
				if ( first )
				{
				  Core::USTRUCT_STATIC newtile;
				  newtile.graphic = tile.graphic;
				  newtile.x_offset = tile.x_offset;
				  newtile.y_offset = tile.y_offset;
				  newtile.z = tile.z;
				  newtile.hue = tile.hue;
				  tilelist.push_back( newtile );
				}
			  }
			}
			Core::USTRUCT_IDX idx;
			idx.offset = ~0u;
			idx.length = ~0u;
			idx.unknown = ~0u;
			if ( !tilelist.empty() )
			{
			  idx.offset = currwritepos;
			  for ( const auto &elem : tilelist )
			  {
				fwrite( &elem, sizeof( Core::USTRUCT_STATIC ), 1, fmul );
			  }
			  currwritepos = ftell( fmul ) - currwritepos;
			  idx.length = currwritepos;
			  idx.unknown = 0;
			  tilelist.clear();
			}
			fwrite( &idx, sizeof idx, 1, fidx );
		  }
		  else
		  {
			Core::USTRUCT_IDX idx;
			idx.offset = ~0u;
			idx.length = ~0u;
			idx.unknown = ~0u;
			fwrite( &idx, sizeof idx, 1, fidx );
		  }
		}
	  }

      INFO_PRINT << "\rRewriting statics files: Complete\n";
	  fclose( fidx );
	  fclose( fmul );
	  return 0;
	}

	int format_description( int argc, char **argv )
	{
	  std::string name = "";
	  for ( int i = 1; i < argc; ++i )
	  {
		name.append( argv[i] );
		if ( i < ( argc - 1 ) )
		  name.append( " " );
	  }
	  if ( name.length() == 0 )
		return Usage( 1 );
	  for ( unsigned short amount = 1; amount <= 2; ++amount )
	  {
		const char *src = name.c_str();

        std::string desc;

		if ( amount != 1 )
		{
		  char s[15];
		  sprintf( s, "%hu ", amount );
		  desc = s;
		}

		int singular = ( amount == 1 );
		int plural_handled = 0;
		int phase = 0; /* 0= first part, 1=plural part, 2=singular part, 3=rest */
		char ch;
		while ( '\0' != ( ch = *src ) )
		{
		  if ( phase == 0 )
		  {
			if ( ch == '%' )
			{
			  plural_handled = 1;
			  phase = 1;
			}
			else
			{
			  desc += ch;
			}
		  }
		  else if ( phase == 1 )
		  {
			if ( ch == '%' )
			  phase = 3;
			else if ( ch == '/' )
			  phase = 2;
			else if ( !singular )
			  desc += ch;
		  }
		  else if ( phase == 2 )
		  {
			if ( ch == '%' )
			  phase = 3;
			else if ( singular )
			  desc += ch;
		  }
		  // if phase == 3 that means there are more words to come, 
		  // lets loop through them to support singular/plural stuff in more than just the first word of the desc.
		  else
		  {
			desc += ch;
			phase = 0;
		  }
		  ++src;
		}

		if ( !singular && !plural_handled )
		  desc += 's';

		if ( amount == 1 )
          INFO_PRINT << "Singular: " << desc << "\n";
		else
          INFO_PRINT << "Plural: " << desc << "\n";
	  }
	  return 0;
	}

	int checkmultis()
	{
      FILE *multi_idx = Core::open_uo_file( "multi.idx" );
      FILE *multi_mul = Core::open_uo_file( "multi.mul" );
	  if ( fseek( multi_idx, 0, SEEK_SET ) != 0 )
	  {
		fclose( multi_idx );
		fclose( multi_mul );
        INFO_PRINT << "Failed seek check\n";
		return 0;
	  }
	  unsigned count = 0;
	  unsigned warnings = 0;
	  unsigned errors = 0;
	  unsigned invisitems = 0;
	  Core::USTRUCT_IDX idxrec;
	  for ( int i = 0; fread( &idxrec, sizeof idxrec, 1, multi_idx ) == 1; ++i )
	  {
		if ( idxrec.offset == 0xFFffFFffLu )
		  continue;
		fseek( multi_mul, idxrec.offset, SEEK_SET );
		Core::USTRUCT_MULTI_ELEMENT elem;
		fread( &elem, sizeof elem, 1, multi_mul );
		if ( elem.x != 0 || elem.y != 0 || elem.z != 0 )
		{
          INFO_PRINT << "ERROR: First tile not in center: " << elem.x << " " << elem.y << " " << elem.z << " (" << elem.flags << ") MultiID: 0x" << fmt::hexu( i ) << "\n";
		  ++errors;
		}
		else if ( elem.graphic == 0x0001 )
		{
		  ++invisitems;
		  unsigned int itemcount = idxrec.length / sizeof elem;
		  --itemcount;
		  while ( itemcount-- )
		  {
			fread( &elem, sizeof elem, 1, multi_mul );
			if ( elem.x == 0 && elem.y == 0 && elem.z == 0 && elem.graphic != 0x0001 && elem.flags )
			{
              INFO_PRINT << "Warning: Found invis tile as center, but could use 0x" << fmt::hexu( elem.graphic ) << " at 0 0 0 MultiID: 0x" << fmt::hexu( i ) << "\n";
			  ++warnings;
			  break;
			}
		  }
		}
		++count;
	  }
      INFO_PRINT << "Checked Multis: " << count << " with invis center: " << invisitems << " Warnings: " << warnings << " Errors: " << errors << "\n";
	  fclose( multi_idx );
	  fclose( multi_mul );
	  return 1;
	}

  }

  int xmain( int argc, char* argv[] )
  {
	Clib::StoreCmdArgs( argc, argv );
	Clib::ConfigFile cf( "pol.cfg" );
	Clib::ConfigElem elem;

	cf.readraw( elem );

    Plib::systemstate.config.uo_datafile_root = elem.remove_string( "UoDataFileRoot" );
    Plib::systemstate.config.uo_datafile_root = Clib::normalized_dir_form( Plib::systemstate.config.uo_datafile_root );

	unsigned short max_tile = elem.remove_ushort( "MaxTileID", UOBJ_DEFAULT_MAX );

    if ( max_tile != UOBJ_DEFAULT_MAX && max_tile != UOBJ_SA_MAX && max_tile != UOBJ_HSA_MAX )
      Plib::systemstate.config.max_tile_id = UOBJ_DEFAULT_MAX;
	else
      Plib::systemstate.config.max_tile_id = max_tile;

	if ( argc <= 1 )
	  return Uotool::Usage( 1 );

	if ( argv[1][0] == '/' || argv[1][1] == ':' )
	{
      Plib::systemstate.config.uo_datafile_root = argv[1];
	  --argc;
	  ++argv;
	}

	if ( stricmp( argv[1], "tiledump" ) == 0 )
	{
	  return Uotool::tiledump( argc, argv );
	}
	else if ( stricmp( argv[1], "vertile" ) == 0 )
	{
	  return Uotool::vertile( argc, argv );
	}
	else if ( stricmp( argv[1], "verlandtile" ) == 0 )
	{
	  return Uotool::verlandtile( argc, argv );
	}
	else if ( stricmp( argv[1], "landtilehist" ) == 0 )
	{
	  return Uotool::landtilehist( argc, argv );
	}
	else if ( stricmp( argv[1], "flagsearch" ) == 0 )
	{
	  return Uotool::flagsearch( argc, argv );
	}
	else if ( stricmp( argv[1], "landtileflagsearch" ) == 0 )
	{
	  return Uotool::landtileflagsearch( argc, argv );
	}
	else if ( stricmp( argv[1], "loschange" ) == 0 )
	{
	  return Uotool::loschange( argc, argv );
	}
	else if ( stricmp( argv[1], "rawdump" ) == 0 )
	{
	  return Uotool::rawdump( argc, argv );
	}
	else if ( stricmp( argv[1], "ctable" ) == 0 )
	{
	  return Uotool::print_ctable();
	}
	else if ( stricmp( argv[1], "sndlist" ) == 0 )
	{
	  return Uotool::print_sndlist( argc, argv );
	}
	else if ( stricmp( argv[1], "statics" ) == 0 )
	{
	  return Uotool::print_statics();
	}
	else if ( stricmp( argv[1], "verdata" ) == 0 )
	{
	  return Uotool::print_verdata_info();
	}
	else if ( stricmp( argv[1], "multis" ) == 0 )
	{
	  return Uotool::print_multis();
	}
	else if ( stricmp( argv[1], "water" ) == 0 )
	{
	  return Uotool::print_water_data();
	}
	else if ( stricmp( argv[1], "newstatics" ) == 0 )
	{
	  return Core::write_pol_static_files( "main" );
	}
	else if ( stricmp( argv[1], "staticsmax" ) == 0 )
	{
	  Core::open_uo_data_files();
	  Core::staticsmax();
	  return 0;
	}
	else if ( stricmp( argv[1], "watersearch" ) == 0 )
	{
	  return Uotool::water_search( argc, argv );
	}
	else if ( stricmp( argv[1], "zhist" ) == 0 )
	{
	  return Uotool::z_histogram();
	}
	else if ( stricmp( argv[1], "staticshist" ) == 0 )
	{
	  return Uotool::statics_histogram();
	}
	else if ( stricmp( argv[1], "writedungmap" ) == 0 )
	{
	  return Uotool::write_polmap();
	}
	else if ( stricmp( argv[1], "writekeys" ) == 0 )
	{
      INFO_PRINT << "Keys written to current.key\n";
	  return 0;
	}
	else if ( stricmp( argv[1], "mapdump" ) == 0 )
	{
	  return Uotool::mapdump( argc, argv );
	}
	else if ( stricmp( argv[1], "contour" ) == 0 )
	{
	  return Uotool::contour( argc, argv );
	}
	else if ( stricmp( argv[1], "findlandtile" ) == 0 )
	{
	  return Uotool::findlandtile( argc - 1, argv + 1 );
	}
	else if ( stricmp( argv[1], "findlandtileflags" ) == 0 )
	{
	  return Uotool::findlandtileflags( argc - 1, argv + 1 );
	}
	else if ( stricmp( argv[1], "findgraphic" ) == 0 )
	{
	  return Uotool::findgraphic( argc - 1, argv + 1 );
	}
	else if ( stricmp( argv[1], "defragstatics" ) == 0 )
	{
	  return Uotool::defragstatics( argc - 1, argv + 1 );
	}
	else if ( stricmp( argv[1], "formatdesc" ) == 0 )
	{
	  return Uotool::format_description( argc - 1, argv + 1 );
	}
	else if ( stricmp( argv[1], "checkmultis" ) == 0 )
	{
	  return Uotool::checkmultis();
	}

	return Uotool::Usage( 0 );
  }
}