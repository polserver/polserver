#include "UoToolMain.h"

#include <assert.h>
#include <iosfwd>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "../clib/Program/ProgramMain.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fdump.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../plib/clidata.h"
#include "../plib/polfile.h"
#include "../plib/poltype.h"
#include "../plib/realmdescriptor.h"
#include "../plib/staticblock.h"
#include "../plib/systemstate.h"
#include "../plib/uconst.h"
#include "../plib/udatfile.h"
#include "../plib/uofile.h"
#include "../plib/uofilei.h"
#include "../plib/uoinstallfinder.h"
#include "../plib/ustruct.h"
#include "../pol/globals/multidefs.h"
#include "../pol/multi/multidef.h"
#include "../pol/objtype.h"
#include <format/format.h>

namespace Pol
{
namespace Multi
{
void read_multidefs();
bool BoatShapeExists( unsigned short /*graphic*/ )
{
  return true;
}
}  // namespace Multi

namespace UoTool
{
using namespace std;
using namespace Pol::Core;
using namespace Pol::Plib;

///////////////////////////////////////////////////////////////////////////////

int keyid[] = {
    0x0002, 0x01f5, 0x0226, 0x0347, 0x0757, 0x0286, 0x03b6, 0x0327, 0x0e08, 0x0628, 0x0567, 0x0798,
    0x19d9, 0x0978, 0x02a6, 0x0577, 0x0718, 0x05b8, 0x1cc9, 0x0a78, 0x0257, 0x04f7, 0x0668, 0x07d8,
    0x1919, 0x1ce9, 0x03f7, 0x0909, 0x0598, 0x07b8, 0x0918, 0x0c68, 0x02d6, 0x1869, 0x06f8, 0x0939,
    0x1cca, 0x05a8, 0x1aea, 0x1c0a, 0x1489, 0x14a9, 0x0829, 0x19fa, 0x1719, 0x1209, 0x0e79, 0x1f3a,
    0x14b9, 0x1009, 0x1909, 0x0136, 0x1619, 0x1259, 0x1339, 0x1959, 0x1739, 0x1ca9, 0x0869, 0x1e99,
    0x0db9, 0x1ec9, 0x08b9, 0x0859, 0x00a5, 0x0968, 0x09c8, 0x1c39, 0x19c9, 0x08f9, 0x18f9, 0x0919,
    0x0879, 0x0c69, 0x1779, 0x0899, 0x0d69, 0x08c9, 0x1ee9, 0x1eb9, 0x0849, 0x1649, 0x1759, 0x1cd9,
    0x05e8, 0x0889, 0x12b9, 0x1729, 0x10a9, 0x08d9, 0x13a9, 0x11c9, 0x1e1a, 0x1e0a, 0x1879, 0x1dca,
    0x1dfa, 0x0747, 0x19f9, 0x08d8, 0x0e48, 0x0797, 0x0ea9, 0x0e19, 0x0408, 0x0417, 0x10b9, 0x0b09,
    0x06a8, 0x0c18, 0x0717, 0x0787, 0x0b18, 0x14c9, 0x0437, 0x0768, 0x0667, 0x04d7, 0x08a9, 0x02f6,
    0x0c98, 0x0ce9, 0x1499, 0x1609, 0x1baa, 0x19ea, 0x39fa, 0x0e59, 0x1949, 0x1849, 0x1269, 0x0307,
    0x06c8, 0x1219, 0x1e89, 0x1c1a, 0x11da, 0x163a, 0x385a, 0x3dba, 0x17da, 0x106a, 0x397a, 0x24ea,
    0x02e7, 0x0988, 0x33ca, 0x32ea, 0x1e9a, 0x0bf9, 0x3dfa, 0x1dda, 0x32da, 0x2eda, 0x30ba, 0x107a,
    0x2e8a, 0x3dea, 0x125a, 0x1e8a, 0x0e99, 0x1cda, 0x1b5a, 0x1659, 0x232a, 0x2e1a, 0x3aeb, 0x3c6b,
    0x3e2b, 0x205a, 0x29aa, 0x248a, 0x2cda, 0x23ba, 0x3c5b, 0x251a, 0x2e9a, 0x252a, 0x1ea9, 0x3a0b,
    0x391b, 0x23ca, 0x392b, 0x3d5b, 0x233a, 0x2cca, 0x390b, 0x1bba, 0x3a1b, 0x3c4b, 0x211a, 0x203a,
    0x12a9, 0x231a, 0x3e0b, 0x29ba, 0x3d7b, 0x202a, 0x3adb, 0x213a, 0x253a, 0x32ca, 0x23da, 0x23fa,
    0x32fa, 0x11ca, 0x384a, 0x31ca, 0x17ca, 0x30aa, 0x2e0a, 0x276a, 0x250a, 0x3e3b, 0x396a, 0x18fa,
    0x204a, 0x206a, 0x230a, 0x265a, 0x212a, 0x23ea, 0x3acb, 0x393b, 0x3e1b, 0x1dea, 0x3d6b, 0x31da,
    0x3e5b, 0x3e4b, 0x207a, 0x3c7b, 0x277a, 0x3d4b, 0x0c08, 0x162a, 0x3daa, 0x124a, 0x1b4a, 0x264a,
    0x33da, 0x1d1a, 0x1afa, 0x39ea, 0x24fa, 0x373b, 0x249a, 0x372b, 0x1679, 0x210a, 0x23aa, 0x1b8a,
    0x3afb, 0x18ea, 0x2eca, 0x0627, 0x00d4  // terminator
};

/* transforms table above into:
 { nbits, bits_reversed [[ LSB .. MSB ]] },
 */
static int print_ctable()
{
  INFO_PRINT << "#include \"ctable.h\"\n\n"
             << "SVR_KEYDESC keydesc[ 257 ] = {\n";
  fmt::Writer _tmp;
  for ( int i = 0; i <= 256; i++ )
  {
    int nbits = keyid[i] & 0x0F;
    unsigned short inmask = 0x10;
    const unsigned short val = static_cast<unsigned short>( keyid[i] );
    unsigned short valout = 0;
    while ( nbits-- )
    {
      valout <<= 1;
      if ( val & inmask )
        valout |= 1;
      inmask <<= 1;
    }
    _tmp.Format( "    {{ {:>2}, 0x{:04X}, 0x{:04X}}},\n" )
        << ( keyid[i] & 0x0F ) << ( keyid[i] >> 4 ) << valout;
  }
  _tmp << "};\n";
  INFO_PRINT << _tmp.str();
  return 0;
}

unsigned char buffer[10000];

#define TILES_START 0x68800

static void display_tileinfo( unsigned short objtype, const USTRUCT_TILE& tile )
{
  fmt::Writer _tmp;
  _tmp.Format( "objtype:  0x{:04X}\n" ) << objtype;
  _tmp << "  name:   " << tile.name << "\n";
  if ( tile.flags )
    _tmp.Format( "  flags:  0x{:08X}\n" ) << static_cast<unsigned long>( tile.flags );
  if ( tile.weight )
    _tmp.Format( "  weight: 0x{:02X}\n" ) << (int)tile.weight;
  if ( tile.layer )
    _tmp.Format( "  layer:  0x{:02X}\n" ) << (int)tile.layer;
  if ( tile.unk6 )
    _tmp.Format( "  unk6:   0x{:02X}\n" ) << (int)tile.unk6;
  if ( tile.unk7 )
    _tmp.Format( "  unk7:   0x{:02X}\n" ) << (int)tile.unk7;
  if ( tile.unk8 )
    _tmp.Format( "  unk8:   0x{:02X}\n" ) << (int)tile.unk8;
  if ( tile.unk9 )
    _tmp.Format( "  unk9:   0x{:02X}\n" ) << (int)tile.unk9;
  if ( tile.anim )
    _tmp.Format( "  anim:   0x{:08X}\n" ) << static_cast<unsigned long>( tile.anim );
  if ( tile.height )
    _tmp.Format( "  height: 0x{:02X}\n" ) << (int)tile.height;
  if ( tile.unk14 )
    _tmp.Format( "  unk14:  0x{:02X}\n" ) << (int)tile.unk14;
  if ( tile.unk15 )
    _tmp.Format( "  unk15:  0x{:02X}\n" ) << (int)tile.unk15;
}

static int tiledump( int argc, char** argv )
{
  USTRUCT_TILE tile;
  u32 version;
  if ( argc != 3 )
    return 1;

  FILE* fp = fopen( argv[2], "rb" );
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

static int vertile()
{
  USTRUCT_TILE tile;

  open_uo_data_files();
  read_uo_data();

  int i;
  for ( i = 0; i <= 0xFFFF; i++ )
  {
    unsigned short objtype = (unsigned short)i;
    read_objinfo( objtype, tile );
    display_tileinfo( objtype, tile );
  }
  clear_tiledata();
  return 0;
}

static int verlandtile()
{
  USTRUCT_LAND_TILE landtile;

  open_uo_data_files();
  read_uo_data();

  int i;
  for ( i = 0; i <= 0x3FFF; i++ )
  {
    unsigned short objtype = (unsigned short)i;
    readlandtile( objtype, &landtile );
    if ( landtile.flags || landtile.unk || landtile.name[0] )
    {
      INFO_PRINT << "Land Tile: 0x" << fmt::hexu( objtype ) << "\n"
                 << "\tflags: 0x" << fmt::hexu( landtile.flags ) << "\n"
                 << "\t  unk: 0x" << fmt::hexu( landtile.unk ) << "\n"
                 << "\t name: " << landtile.name << "\n";
    }
  }
  clear_tiledata();
  return 0;
}

static int landtilehist()
{
  USTRUCT_LAND_TILE landtile;

  open_uo_data_files();
  read_uo_data();

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
  for ( const auto& elem : tilecount )
  {
    tmp << elem.first << ": " << elem.second << "\n";
  }
  INFO_PRINT << tmp.str();
  clear_tiledata();
  return 0;
}

static int flagsearch( int argc, char** argv )
{
  USTRUCT_TILE tile;

  open_uo_data_files();
  read_uo_data();

  if ( argc < 3 )
    return 1;

  unsigned int flags = strtoul( argv[2], nullptr, 0 );
  unsigned int notflags = 0;
  if ( argc >= 4 )
    notflags = strtoul( argv[3], nullptr, 0 );

  int i;
  for ( i = 0; i <= 0xFFFF; i++ )
  {
    unsigned short objtype = (unsigned short)i;
    read_objinfo( objtype, tile );
    if ( ( ( tile.flags & flags ) == flags ) && ( ( ~tile.flags & notflags ) == notflags ) )
    {
      display_tileinfo( objtype, tile );
    }
  }
  clear_tiledata();
  return 0;
}

static int landtileflagsearch( int argc, char** argv )
{
  open_uo_data_files();
  read_uo_data();

  if ( argc < 3 )
    return 1;

  unsigned int flags = strtoul( argv[2], nullptr, 0 );
  unsigned int notflags = 0;
  if ( argc >= 4 )
    notflags = strtoul( argv[3], nullptr, 0 );

  USTRUCT_LAND_TILE landtile;

  int i;
  for ( i = 0; i < 0x4000; i++ )
  {
    unsigned short objtype = (unsigned short)i;
    readlandtile( objtype, &landtile );
    if ( ( landtile.flags & flags ) == flags && ( ~landtile.flags & notflags ) == notflags )
    {
      INFO_PRINT << "Land Tile: 0x" << fmt::hexu( objtype ) << "\n"
                 << "\tflags: 0x" << fmt::hexu( landtile.flags ) << "\n"
                 << "\t  unk: 0x" << fmt::hexu( landtile.unk ) << "\n"
                 << "\t name: " << landtile.name << "\n";
    }
  }
  clear_tiledata();
  return 0;
}

static int loschange( int /*argc*/, char** /*argv*/ )
{
  USTRUCT_TILE tile;

  open_uo_data_files();
  read_uo_data();

  for ( int i = 0; i <= 0xFFFF; i++ )
  {
    unsigned short objtype = (unsigned short)i;
    read_objinfo( objtype, tile );

    bool old_lostest = ( tile.flags & USTRUCT_TILE::FLAG_WALKBLOCK ) != 0;

    bool new_lostest =
        ( tile.flags & ( USTRUCT_TILE::FLAG_WALKBLOCK | USTRUCT_TILE::FLAG_NO_SHOOT ) ) != 0;

    if ( old_lostest != new_lostest )
    {
      display_tileinfo( objtype, tile );
      INFO_PRINT.Format( " Old LOS: %s\n New LOS: %s\n" )
          << ( old_lostest ? "true" : "false" ) << ( new_lostest ? "true" : "false" );
    }
  }
  clear_tiledata();
  return 0;
}

static int print_verdata_info()
{
  open_uo_data_files();
  read_uo_data();
  int num_version_records;
  USTRUCT_VERSION vrec;

  // FIXME: should read this once per run, per file.
  fseek( verfile, 0, SEEK_SET );
  if ( fread( &num_version_records, sizeof num_version_records, 1, verfile ) !=
       1 )  // ENDIAN-BROKEN
    throw std::runtime_error( "print_verdata_info: fread(num_version_records) failed." );

  INFO_PRINT << "There are " << num_version_records << " version records.\n";

  int filecount[32];
  int inv_filecount = 0;
  memset( filecount, 0, sizeof filecount );

  for ( int i = 0; i < num_version_records; i++ )
  {
    if ( fread( &vrec, sizeof vrec, 1, verfile ) != 1 )
      throw std::runtime_error( "print_verdata_info: fread(vrec) failed." );
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
  clear_tiledata();
  return 0;
}

static int print_statics()
{
  INFO_PRINT << "Reading UO data..\n";
  open_uo_data_files();
  read_uo_data();
  int water = 0;
  int strange_water = 0;
  int cnt = 0;
  for ( u16 y = 30; y < 50; y++ )
  {
    for ( u16 x = 5900; x < 5940; x++ )
    {
      std::vector<StaticRec> vec;
      readallstatics( vec, x, y );

      if ( !vec.empty() )
      {
        bool hdrshown = false;
        for ( const auto& elem : vec )
        {
          int height = Plib::tileheight_read( elem.graphic );
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
          INFO_PRINT << "\tOBJT= 0x" << fmt::hexu( elem.graphic ) << "  Z=" << int( elem.z )
                     << "  HT=" << height << "  FLAGS=0x"
                     << fmt::hexu( Plib::tile_uoflags_read( elem.graphic ) ) << "\n";
          ++cnt;
        }
      }
    }
  }
  INFO_PRINT << cnt << " statics exist.\n"
             << water << " water tiles exist.\n"
             << strange_water << " strange water tiles exist.\n";
  clear_tiledata();
  return 0;
}

static int rawdump( int argc, char** argv )
{
  if ( argc != 5 )
    return 1;

  FILE* fp = fopen( argv[2], "rb" );
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
    INFO_PRINT << tmp.str() << "\n";

    ++recnum;
  }
  INFO_PRINT << recnum << " records read.\n";
  fclose( fp );
  return 0;
}

static unsigned int read_ulong( std::istream& is )
{
  unsigned char a[4];
  is.read( (char*)a, sizeof a );
  return ( a[3] << 24 ) | ( a[2] << 16 ) | ( a[1] << 8 ) | a[0];
}

static int print_sndlist()
{
  unsigned int offset;
  unsigned int length;
  unsigned int serial;
  char filename[15];

  std::string soundidxname = Plib::systemstate.config.uo_datafile_root + "soundidx.mul";
  std::string soundname = Plib::systemstate.config.uo_datafile_root + "sound.mul";
  std::ifstream soundidx( soundidxname.c_str(), std::ios::in | std::ios::binary );
  std::ifstream sound( soundname.c_str(), std::ios::in | std::ios::binary );
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

    sound.seekg( offset, std::ios::beg );
    if ( !sound.good() )
      break;

    sound.read( filename, sizeof filename );
    INFO_PRINT << "0x" << fmt::hexu( i ) << ", 0x" << fmt::hexu( serial ) << ": " << filename
               << "\n"
               << "len " << length << "\n";
  }
  return 0;
}

static void print_multihull( u16 i, Multi::MultiDef* multi )
{
  if ( multi->hull.empty() )
    return;

  USTRUCT_TILE tile;
  read_objinfo( static_cast<u16>( i + ( Plib::systemstate.config.max_tile_id + 1 ) ), tile );
  INFO_PRINT << "Multi 0x" << fmt::hexu( i + i + ( Plib::systemstate.config.max_tile_id + 1 ) )
             << " -- " << tile.name << ":\n";
  fmt::Writer tmp;
  for ( short y = multi->minrxyz.y(); y <= multi->maxrxyz.y(); ++y )
  {
    for ( short x = multi->minrxyz.x(); x <= multi->maxrxyz.x(); ++x )
    {
      unsigned short key = multi->getkey( Core::Vec2d( x, y ) );
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
  INFO_PRINT << tmp.str();
}

static void print_multidata( u16 i, Multi::MultiDef* multi )
{
  if ( multi->hull.empty() )
    return;

  USTRUCT_TILE tile;
  read_objinfo( static_cast<u16>( i + ( Plib::systemstate.config.max_tile_id + 1 ) ), tile );
  INFO_PRINT << "Multi 0x" << fmt::hexu( i + ( Plib::systemstate.config.max_tile_id + 1 ) )
             << " -- " << tile.name << ":\n";
  fmt::Writer tmp;
  for ( const auto& _itr : multi->components )
  {
    const Multi::MULTI_ELEM* elem = _itr.second;
    tmp << "0x" << fmt::hexu( elem->objtype ) << " 0x" << fmt::hexu( (int)elem->is_static ) << ":"
        << elem->relpos << "\n";
  }
  INFO_PRINT << tmp.str();
}

static int print_multis()
{
  INFO_PRINT << "Reading UO data..\n";
  open_uo_data_files();
  read_uo_data();
  Multi::read_multidefs();

  for ( u16 i = 0; i < 0x1000; ++i )
  {
    if ( Multi::multidef_buffer.multidefs_by_multiid[i] )
    {
      print_multihull( i, Multi::multidef_buffer.multidefs_by_multiid[i] );
      print_multidata( i, Multi::multidef_buffer.multidefs_by_multiid[i] );
    }
  }
  clear_tiledata();
  return 0;
}

static int z_histogram()
{
  unsigned int zcount[256];
  memset( zcount, 0, sizeof( zcount ) );

  INFO_PRINT << "Reading UO data..\n";
  open_uo_data_files();
  read_uo_data();
  for ( u16 x = 0; x < 6143; ++x )
  {
    INFO_PRINT << ".";
    for ( u16 y = 0; y < 4095; ++y )
    {
      USTRUCT_MAPINFO mi;
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
  clear_tiledata();
  return 0;
}

static int statics_histogram()
{
  unsigned int counts[1000];
  memset( counts, 0, sizeof counts );
  INFO_PRINT << "Reading UO data..\n";
  open_uo_data_files();
  read_uo_data();
  for ( u16 x = 0; x < 6143; x += 8 )
  {
    INFO_PRINT << ".";
    for ( u16 y = 0; y < 4095; y += 8 )
    {
      std::vector<USTRUCT_STATIC> p;
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
  clear_tiledata();
  return 0;
}

static int write_polmap( const char* filename, unsigned short xbegin, unsigned short xend )
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
      USTRUCT_POL_MAPINFO_BLOCK blk;
      memset( &blk, 0, sizeof blk );
      for ( u16 xi = 0; xi < 8; ++xi )
      {
        for ( u16 yi = 0; yi < 8; ++yi )
        {
          u16 x = xs + xi;
          u16 y = ys + yi;
          if ( x == 6143 )
            x = 6142;
          if ( y == 4095 )
            y = 4094;
          bool walkok = groundheight_read( x, y, &z );
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

static int write_polmap()
{
  INFO_PRINT << "Reading UO data..\n";
  open_uo_data_files();
  read_uo_data();
  write_polmap( "dngnmap0.pol", 5120, 6144 );
  write_polmap( "map0.pol", 0, 6144 );
  clear_tiledata();
  return 0;
}

static int print_water_data()
{
  open_uo_data_files();
  readwater();
  return 0;
}

static bool has_water( u16 x, u16 y )
{
  StaticList vec;
  vec.clear();
  readstatics( vec, x, y );
  for ( const auto& rec : vec )
  {
    if ( iswater( rec.graphic ) )
      return true;
  }
  return false;
}

static int water_search()
{
  u16 wxl = 1420, wxh = 1480, wyl = 1760, wyh = 1780;
  open_uo_data_files();
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

static int mapdump( int argc, char* argv[] )
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

  open_uo_data_files();
  read_uo_data();

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
      USTRUCT_MAPINFO mi;
      safe_getmapinfo( x, y, &z, &mi );
      USTRUCT_LAND_TILE landtile;
      readlandtile( mi.landtile, &landtile );
      ofs << "z=" << z << "<br>";
      ofs << "landtile=" << Clib::hexint( mi.landtile ) << " " << landtile.name << "<br>";
      ofs << "&nbsp;flags=" << Clib::hexint( landtile.flags ) << "<br>";
      ofs << "mapz=" << (int)mi.z << "<br>";

      StaticList statics;
      readallstatics( statics, x, y );
      if ( !statics.empty() )
      {
        ofs << "<table border=1 cellpadding=5 cellspacing=0>" << std::endl;
        ofs << "<tr><td>graphic</td><td>z</td><td>flags</td><td>ht</td>" << std::endl;
        for ( const auto& rec : statics )
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
  clear_tiledata();
  return 0;
}

struct MapContour
{
  signed char z[6144][4096];
};

static int contour()
{
  open_uo_data_files();
  read_uo_data();

  auto mc = new MapContour;
  for ( u16 y = 0; y < 4095; ++y )
  {
    for ( u16 x = 0; x < 6143; ++x )
    {
      static StaticList vec;

      vec.clear();

      readstatics( vec, x, y );

      bool result;
      short newz;
      standheight_read( MOVEMODE_LAND, vec, x, y, 127, &result, &newz );
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

  std::ofstream ofs( "contour.dat", std::ios::trunc | std::ios::out | std::ios::binary );
  ofs.write( reinterpret_cast<const char*>( mc ), sizeof( MapContour ) );
  clear_tiledata();
  delete mc;
  return 0;
}

static int findlandtile( int /*argc*/, char** argv )
{
  int landtile = strtoul( argv[1], nullptr, 0 );
  open_uo_data_files();
  read_uo_data();

  for ( u16 y = 0; y < 4095; ++y )
  {
    for ( u16 x = 0; x < 6143; ++x )
    {
      short z;
      USTRUCT_MAPINFO mi;
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

  clear_tiledata();
  return 0;
}

static int findgraphic( int /*argc*/, char** argv )
{
  int graphic = strtoul( argv[1], nullptr, 0 );
  INFO_PRINT << "Searching map for statics with graphic=0x" << fmt::hexu( graphic ) << "\n";

  open_uo_data_files();
  read_uo_data();

  for ( u16 y = 0; y < 4095; ++y )
  {
    for ( u16 x = 0; x < 6143; ++x )
    {
      StaticList statics;
      readallstatics( statics, x, y );
      for ( const auto& rec : statics )
      {
        if ( rec.graphic == graphic )
        {
          INFO_PRINT << x << "," << y << "," << rec.z << "\n";
        }
      }
    }
  }
  clear_tiledata();
  return 0;
}

static int findlandtileflags( int /*argc*/, char** argv )
{
  unsigned int flags = strtoul( argv[1], nullptr, 0 );
  open_uo_data_files();
  read_uo_data();

  for ( u16 y = 0; y < 4095; ++y )
  {
    for ( u16 x = 0; x < 6143; ++x )
    {
      short z;
      USTRUCT_MAPINFO mi;
      safe_getmapinfo( x, y, &z, &mi );
      if ( Plib::landtile_uoflags_read( mi.landtile ) & flags )
      {
        INFO_PRINT << x << "," << y << "," << (int)mi.z << ": landtile 0x"
                   << fmt::hexu( mi.landtile ) << ", flags 0x"
                   << fmt::hexu( Plib::landtile_uoflags_read( mi.landtile ) ) << "\n";
      }
    }
  }

  clear_tiledata();
  return 0;
}

static int defragstatics( int argc, char** argv )
{
  const char* realm;
  if ( argc < 2 )
    realm = "britannia";
  else
    realm = argv[1];

  Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realm );

  uo_mapid = descriptor.uomapid;
  uo_usedif = descriptor.uodif;
  uo_map_width = static_cast<unsigned short>( descriptor.width );
  uo_map_height = static_cast<unsigned short>( descriptor.height );

  open_uo_data_files();
  read_uo_data();

  std::string statidx = "staidx" + Clib::tostring( uo_mapid ) + ".mul";
  std::string statics = "statics" + Clib::tostring( uo_mapid ) + ".mul";
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
      std::vector<USTRUCT_STATIC> pstat;
      int num;
      std::vector<USTRUCT_STATIC> tilelist;
      readstaticblock( &pstat, &num, x, y );
      if ( num > 0 )
      {
        int currwritepos = ftell( fmul );
        for ( int i = 0; i < num; ++i )
        {
          USTRUCT_STATIC& tile = pstat[i];
          if ( tile.graphic < 0x4000 )
          {
            bool first = true;
            for ( const auto& stile : tilelist )
            {
              if ( ( tile.graphic == stile.graphic ) && ( tile.x_offset == stile.x_offset ) &&
                   ( tile.y_offset == stile.y_offset ) && ( tile.z == stile.z ) &&
                   ( tile.hue == stile.hue ) )
              {
                first = false;
                break;
              }
            }
            if ( first )
            {
              USTRUCT_STATIC newtile;
              newtile.graphic = tile.graphic;
              newtile.x_offset = tile.x_offset;
              newtile.y_offset = tile.y_offset;
              newtile.z = tile.z;
              newtile.hue = tile.hue;
              tilelist.push_back( newtile );
            }
          }
        }
        USTRUCT_IDX idx;
        idx.offset = ~0u;
        idx.length = ~0u;
        idx.unknown = ~0u;
        if ( !tilelist.empty() )
        {
          idx.offset = currwritepos;
          for ( const auto& elem : tilelist )
          {
            fwrite( &elem, sizeof( USTRUCT_STATIC ), 1, fmul );
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
        USTRUCT_IDX idx;
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

static int format_description( int argc, char** argv )
{
  std::string name = "";
  for ( int i = 1; i < argc; ++i )
  {
    name.append( argv[i] );
    if ( i < ( argc - 1 ) )
      name.append( " " );
  }
  if ( name.length() == 0 )
    return 1;
  for ( unsigned short amount = 1; amount <= 2; ++amount )
  {
    const char* src = name.c_str();

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
      // lets loop through them to support singular/plural stuff in more than just the first word of
      // the desc.
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

static int checkmultis()
{
  FILE* multi_idx = open_uo_file( "multi.idx" );
  FILE* multi_mul = open_uo_file( "multi.mul" );
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
  USTRUCT_IDX idxrec;
  for ( int i = 0; fread( &idxrec, sizeof idxrec, 1, multi_idx ) == 1; ++i )
  {
    if ( idxrec.offset == 0xFFffFFffLu )
      continue;
    fseek( multi_mul, idxrec.offset, SEEK_SET );
    USTRUCT_MULTI_ELEMENT elem;
    if ( fread( &elem, sizeof elem, 1, multi_mul ) != 1 )
      throw std::runtime_error( "checkmultis: fread(elem) failed." );
    if ( elem.x != 0 || elem.y != 0 || elem.z != 0 )
    {
      INFO_PRINT << "ERROR: First tile not in center: " << elem.x << " " << elem.y << " " << elem.z
                 << " (" << elem.flags << ") MultiID: 0x" << fmt::hexu( i ) << "\n";
      ++errors;
    }
    else if ( elem.graphic == 0x0001 )
    {
      ++invisitems;
      unsigned int itemcount = idxrec.length / sizeof elem;
      --itemcount;
      while ( itemcount-- )
      {
        if ( fread( &elem, sizeof elem, 1, multi_mul ) != 1 )
          throw std::runtime_error( "checkmultis: fread(multi_mul) failed." );
        if ( elem.x == 0 && elem.y == 0 && elem.z == 0 && elem.graphic != 0x0001 && elem.flags )
        {
          INFO_PRINT << "Warning: Found invis tile as center, but could use 0x"
                     << fmt::hexu( elem.graphic ) << " at 0 0 0 MultiID: 0x" << fmt::hexu( i )
                     << "\n";
          ++warnings;
          break;
        }
      }
    }
    ++count;
  }
  INFO_PRINT << "Checked Multis: " << count << " with invis center: " << invisitems
             << " Warnings: " << warnings << " Errors: " << errors << "\n";
  fclose( multi_idx );
  fclose( multi_mul );
  return 1;
}


///////////////////////////////////////////////////////////////////////////////

UoToolMain::UoToolMain() : Pol::Clib::ProgramMain() {}
UoToolMain::~UoToolMain() {}
///////////////////////////////////////////////////////////////////////////////

void UoToolMain::showHelp()
{
  ERROR_PRINT << "Usage:\n"
              << "    \n"
              << "  UOTOOL command [options ...]\n"
              << "    \n"
              << "  Commands:\n"
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
}

static int s_argc;
static char** s_argv;

int UoToolMain::main()
{
  const std::vector<std::string>& binArgs = programArgs();

  /**********************************************
   * show help
   **********************************************/
  if ( binArgs.size() == 1 )
  {
    showHelp();
    return 0;  // return "okay"
  }

  /**********************************************
   * TODO: rework the following cruft from former uotool.cpp
   **********************************************/
  Clib::ConfigFile cf( "pol.cfg" );
  Clib::ConfigElem elem;

  cf.readraw( elem );

  Plib::systemstate.config.uo_datafile_root = Plib::UOInstallFinder::remove_elem( elem );
  Plib::systemstate.config.uo_datafile_root =
      Clib::normalized_dir_form( Plib::systemstate.config.uo_datafile_root );

  unsigned short max_tile = elem.remove_ushort( "MaxTileID", UOBJ_DEFAULT_MAX );

  if ( max_tile != UOBJ_DEFAULT_MAX && max_tile != UOBJ_SA_MAX && max_tile != UOBJ_HSA_MAX )
    Plib::systemstate.config.max_tile_id = UOBJ_DEFAULT_MAX;
  else
    Plib::systemstate.config.max_tile_id = max_tile;

  std::string argvalue = binArgs[1];
  if ( argvalue[0] == '/' || argvalue[0] == ':' )
  {
    Plib::systemstate.config.uo_datafile_root = argvalue;
    if ( binArgs.size() < 3 )
    {
      showHelp();
      return 0;
    }
    argvalue = binArgs[2];
  }

  std::transform( argvalue.begin(), argvalue.end(), argvalue.begin(),
                  []( char c ) { return static_cast<char>( ::tolower( c ) ); } );

  if ( argvalue == "tiledump" )
  {
    return UoTool::tiledump( s_argc, s_argv );
  }
  else if ( argvalue == "vertile" )
  {
    return UoTool::vertile();
  }
  else if ( argvalue == "verlandtile" )
  {
    return UoTool::verlandtile();
  }
  else if ( argvalue == "landtilehist" )
  {
    return UoTool::landtilehist();
  }
  else if ( argvalue == "flagsearch" )
  {
    return UoTool::flagsearch( s_argc, s_argv );
  }
  else if ( argvalue == "landtileflagsearch" )
  {
    return UoTool::landtileflagsearch( s_argc, s_argv );
  }
  else if ( argvalue == "loschange" )
  {
    return UoTool::loschange( s_argc, s_argv );
  }
  else if ( argvalue == "rawdump" )
  {
    return UoTool::rawdump( s_argc, s_argv );
  }
  else if ( argvalue == "ctable" )
  {
    return UoTool::print_ctable();
  }
  else if ( argvalue == "sndlist" )
  {
    return UoTool::print_sndlist();
  }
  else if ( argvalue == "statics" )
  {
    return UoTool::print_statics();
  }
  else if ( argvalue == "verdata" )
  {
    return UoTool::print_verdata_info();
  }
  else if ( argvalue == "multis" )
  {
    return UoTool::print_multis();
  }
  else if ( argvalue == "water" )
  {
    return UoTool::print_water_data();
  }
  else if ( argvalue == "newstatics" )
  {
    return write_pol_static_files( "main" );
  }
  else if ( argvalue == "staticsmax" )
  {
    open_uo_data_files();
    staticsmax();
    return 0;
  }
  else if ( argvalue == "watersearch" )
  {
    return UoTool::water_search();
  }
  else if ( argvalue == "zhist" )
  {
    return UoTool::z_histogram();
  }
  else if ( argvalue == "staticshist" )
  {
    return UoTool::statics_histogram();
  }
  else if ( argvalue == "writedungmap" )
  {
    return UoTool::write_polmap();
  }
  else if ( argvalue == "writekeys" )
  {
    INFO_PRINT << "Keys written to current.key\n";
    return 0;
  }
  else if ( argvalue == "mapdump" )
  {
    return UoTool::mapdump( s_argc, s_argv );
  }
  else if ( argvalue == "contour" )
  {
    return UoTool::contour();
  }
  else if ( argvalue == "findlandtile" )
  {
    return UoTool::findlandtile( s_argc - 1, s_argv + 1 );
  }
  else if ( argvalue == "findlandtileflags" )
  {
    return UoTool::findlandtileflags( s_argc - 1, s_argv + 1 );
  }
  else if ( argvalue == "findgraphic" )
  {
    return UoTool::findgraphic( s_argc - 1, s_argv + 1 );
  }
  else if ( argvalue == "defragstatics" )
  {
    return UoTool::defragstatics( s_argc - 1, s_argv + 1 );
  }
  else if ( argvalue == "formatdesc" )
  {
    return UoTool::format_description( s_argc - 1, s_argv + 1 );
  }
  else if ( argvalue == "checkmultis" )
  {
    return UoTool::checkmultis();
  }

  showHelp();
  return 0;
}
}  // namespace UoTool
}  // namespace Pol

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
  Pol::UoTool::s_argc = argc;
  Pol::UoTool::s_argv = argv;

  Pol::UoTool::UoToolMain* UoToolMain = new Pol::UoTool::UoToolMain();
  UoToolMain->start( argc, argv );
}
