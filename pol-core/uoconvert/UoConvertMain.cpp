#include "UoConvertMain.h"

#include <stdio.h>
#include <string.h>
#include <string>

#include "../clib/Program/ProgramMain.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/fileutil.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"
#include "../clib/timer.h"
#include "../plib/clidata.h"
#include "../plib/mapcell.h"
#include "../plib/mapfunc.h"
#include "../plib/mapshape.h"
#include "../plib/mapsolid.h"
#include "../plib/maptile.h"
#include "../plib/mapwriter.h"
#include "../plib/polfile.h"
#include "../plib/realmdescriptor.h"
#include "../plib/systemstate.h"
#include "../plib/udatfile.h"
#include "../plib/uofile.h"
#include "../plib/uofilei.h"
#include "../plib/uoinstallfinder.h"
#include "../plib/uopreader/uop.h"
#include "../plib/uopreader/uophash.h"
#include "../plib/ustruct.h"
#include "../pol/landtile.h"
#include "../pol/objtype.h"


namespace Pol
{
namespace UoConvert
{
using namespace std;
using namespace Pol::Core;
using namespace Pol::Plib;

///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

UoConvertMain::UoConvertMain()
    : Pol::Clib::ProgramMain(), cfg_use_no_shoot( false ), cfg_LOS_through_windows( false )
{
}
UoConvertMain::~UoConvertMain() {}
///////////////////////////////////////////////////////////////////////////////

void UoConvertMain::showHelp()
{
  ERROR_PRINT << "Usage:\n"
              << "    \n"
              << "  UOCONVERT command [options ...]\n"
              << "    \n"
              << "  Commands: \n"
              << "    map {uodata=Dir} {maxtileid=0x3FFF/0x7FFF} {realm=realmname} {width=Width}"
              << "        {height=Height} {mapid=0} {readuop=1} {x=X} {y=Y}\n"
              << "    statics {uodata=Dir} {maxtileid=0x3FFF/0x7FFF} {realm=realmname}\n"
              << "    maptile {uodata=Dir} {maxtileid=0x3FFF/0x7FFF} {realm=realmname}\n"
              << "    multis {uodata=Dir} {maxtileid=0x3FFF/0x7FFF} {outdir=dir}\n"
              << "    tiles {uodata=Dir} {maxtileid=0x3FFF/0x7FFF} {outdir=dir}\n"
              << "    landtiles {uodata=Dir} {maxtileid=0x3FFF/0x7FFF} {outdir=dir}\n";
}

using namespace Core;
using namespace Plib;

void UoConvertMain::display_flags()
{
  for ( unsigned blocking = 0; blocking <= 1; ++blocking )
  {
    for ( unsigned platform = 0; platform <= 1; ++platform )
    {
      for ( unsigned walk = 0; walk <= 1; ++walk )
      {
        for ( unsigned wall = 0; wall <= 1; ++wall )
        {
          for ( unsigned half = 0; half <= 1; ++half )
          {
            for ( unsigned floor = 0; floor <= 1; ++floor )
            {
              unsigned flags = 0;
              if ( blocking )
                flags |= USTRUCT_TILE::FLAG_BLOCKING;
              if ( platform )
                flags |= USTRUCT_TILE::FLAG_PLATFORM;
              if ( walk )
                flags |= USTRUCT_TILE::FLAG__WALK;
              if ( wall )
                flags |= USTRUCT_TILE::FLAG_WALL;
              if ( half )
                flags |= USTRUCT_TILE::FLAG_HALF_HEIGHT;
              if ( floor )
                flags |= USTRUCT_TILE::FLAG_FLOOR;

              unsigned int polflags = Plib::polflags_from_tileflags(
                  0x4000, flags, cfg_use_no_shoot, cfg_LOS_through_windows );
              unsigned moveland = ( polflags & Plib::FLAG::MOVELAND ) ? 1 : 0;
              INFO_PRINT.Format( "{} {} {} {} {} {}: {}\n" )
                  << blocking << platform << walk << wall << half << floor << moveland;
            }
          }
        }
      }
    }
  }
}

void UoConvertMain::create_maptile( const std::string& realmname )
{
  Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realmname );
  uo_map_height = static_cast<unsigned short>( descriptor.height );
  uo_map_width = static_cast<unsigned short>( descriptor.width );

  INFO_PRINT << "Creating maptile file.\n"
             << "  Realm: " << realmname << "\n"
             << "  Map ID: " << descriptor.uomapid << "\n"
             << "  Use Dif files: " << ( descriptor.uodif ? "Yes" : "No" ) << "\n"
             << "  Size: " << uo_map_width << "x" << uo_map_height << "\n";

  auto writer = new Plib::MapWriter();
  writer->OpenExistingFiles( realmname );

  for ( unsigned short y_base = 0; y_base < uo_map_height; y_base += Plib::MAPTILE_CHUNK )
  {
    for ( unsigned short x_base = 0; x_base < uo_map_width; x_base += Plib::MAPTILE_CHUNK )
    {
      for ( unsigned short x_add = 0; x_add < Plib::MAPTILE_CHUNK; ++x_add )
      {
        for ( unsigned short y_add = 0; y_add < Plib::MAPTILE_CHUNK; ++y_add )
        {
          unsigned short x = x_base + x_add;
          unsigned short y = y_base + y_add;

          short z;
          USTRUCT_MAPINFO mi;

          safe_getmapinfo( x, y, &z, &mi );

          if ( mi.landtile > 0x3FFF )
            INFO_PRINT.Format( "Tile 0x{:X} at ({},{},{}) is an invalid ID!\n" )
                << mi.landtile << x << y << z;

          // for water, don't average with surrounding tiles.
          if ( Plib::landtile_uoflags_read( mi.landtile ) & Plib::USTRUCT_TILE::FLAG_LIQUID )
            z = mi.z;

          Plib::MAPTILE_CELL cell;
          cell.landtile = mi.landtile;
          cell.z = static_cast<signed char>( z );
          writer->SetMapTile( x, y, cell );
        }
      }
    }
    INFO_PRINT << "\rConverting: " << y_base * 100 / uo_map_height << "%";
  }
  writer->Flush();
  delete writer;
  INFO_PRINT << "\rConversion complete.\n";
}

class StaticsByZ
{
public:
  bool operator()( const StaticRec& a, const StaticRec& b )
  {
    return ( a.z < b.z ) || ( ( a.z == b.z && a.height < b.height ) );
  }
};


bool flags_match( unsigned int f1, unsigned int f2, unsigned char bits_compare )
{
  return ( f1 & bits_compare ) == ( f2 & bits_compare );
}

/*
bool otherflags_match( unsigned char f1, unsigned char f2, unsigned char bits_exclude )
{
  return ( f1 & ~bits_exclude ) == ( f2 & ~bits_exclude );
}
bool differby_exactly( unsigned char f1, unsigned char f2, unsigned char bits )
{
  return ( ( f1 ^ f2 ) == bits );
}*/

void UoConvertMain::update_map( const std::string& realm, unsigned short x, unsigned short y )
{
  auto mapwriter = new MapWriter();
  mapwriter->OpenExistingFiles( realm );
  rawmapfullread();
  rawstaticfullread();
  unsigned short x_base = x / SOLIDX_X_SIZE * SOLIDX_X_SIZE;
  unsigned short y_base = y / SOLIDX_Y_SIZE * SOLIDX_Y_SIZE;

  ProcessSolidBlock( x_base, y_base, *mapwriter );
  delete mapwriter;
  INFO_PRINT << "empty=" << empty << ", nonempty=" << nonempty << "\n"
             << "with more_solids: " << with_more_solids << "\n"
             << "total statics=" << total_statics << "\n";
}

void UoConvertMain::create_map( const std::string& realm, unsigned short width,
                                unsigned short height )
{
  auto mapwriter = new MapWriter();
  INFO_PRINT << "Creating map base and solids files.\n"
             << "  Realm: " << realm << "\n"
             << "  Map ID: " << uo_mapid << "\n"
             << "  Reading UOP file: " << ( uo_readuop ? "Yes" : "No" ) << "\n"
             << "  Use Dif files: " << ( uo_usedif ? "Yes" : "No" ) << "\n"
             << "  Size: " << uo_map_width << "x" << uo_map_height << "\n"
             << "Initializing files: ";
  mapwriter->CreateNewFiles( realm, width, height );
  INFO_PRINT << "Done.\n";
  Tools::Timer<> timer;
  rawmapfullread();
  rawstaticfullread();
  INFO_PRINT << "  Reading mapfiles time: " << timer.ellapsed() << " ms.\n";
  for ( unsigned short y_base = 0; y_base < height; y_base += SOLIDX_Y_SIZE )
  {
    for ( unsigned short x_base = 0; x_base < width; x_base += SOLIDX_X_SIZE )
    {
      ProcessSolidBlock( x_base, y_base, *mapwriter );
    }
    INFO_PRINT << "\rConverting: " << y_base * 100 / height << "%";
  }
  timer.stop();

  mapwriter->WriteConfigFile();
  delete mapwriter;

  INFO_PRINT << "\rConversion complete.              \n"
             << "Conversion details:\n"
             << "  Total blocks: " << empty + nonempty << "\n"
             << "  Blocks with solids: " << nonempty << " ("
             << ( nonempty * 100 / ( empty + nonempty ) ) << "%)"
             << "\n"
             << "  Blocks without solids: " << empty << " ("
             << ( empty * 100 / ( empty + nonempty ) ) << "%)"
             << "\n"
             << "  Locations with solids: " << with_more_solids << "\n"
             << "  Total number of solids: " << total_statics << "\n"
             << "  Elapsed time: " << timer.ellapsed() << " ms.\n";
}

static bool is_no_draw( USTRUCT_MAPINFO& mi )
{
  return ( mi.landtile == 0x2 );
}

static bool is_cave_exit( USTRUCT_MAPINFO& mi )
{
  return ( mi.landtile == 0x7ec || mi.landtile == 0x7ed || mi.landtile == 0x7ee ||
           mi.landtile == 0x7ef || mi.landtile == 0x7f0 || mi.landtile == 0x7f1 ||
           mi.landtile == 0x834 || mi.landtile == 0x835 || mi.landtile == 0x836 ||
           mi.landtile == 0x837 || mi.landtile == 0x838 || mi.landtile == 0x839 ||
           mi.landtile == 0x1d3 || mi.landtile == 0x1d4 || mi.landtile == 0x1d5 ||
           mi.landtile == 0x1d6 || mi.landtile == 0x1d7 || mi.landtile == 0x1d8 ||
           mi.landtile == 0x1d9 || mi.landtile == 0x1da );
}

static bool is_cave_shadow( USTRUCT_MAPINFO& mi )
{
  return ( mi.landtile == 0x1db ||  // shadows above caves
           mi.landtile == 0x1ae ||  // more shadows above caves
           mi.landtile == 0x1af || mi.landtile == 0x1b0 || mi.landtile == 0x1b1 ||
           mi.landtile == 0x1b2 || mi.landtile == 0x1b3 || mi.landtile == 0x1b4 ||
           mi.landtile == 0x1b5 );
}

short get_lowestadjacentz( unsigned short x, unsigned short y, short z )
{
  USTRUCT_MAPINFO mi;
  short z0;
  short lowest_z = z;
  bool cave_override = false;

  if ( ( x - 1 >= 0 ) && ( y - 1 >= 0 ) )
  {
    safe_getmapinfo( x - 1, y - 1, &z0, &mi );

    if ( is_cave_shadow( mi ) || is_cave_exit( mi ) )
      z0 = z;

    if ( is_no_draw( mi ) )
      cave_override = true;

    if ( z0 < lowest_z )
    {
      lowest_z = z0;
    }
  }

  if ( x - 1 >= 0 )
  {
    safe_getmapinfo( x - 1, y, &z0, &mi );

    if ( is_cave_shadow( mi ) || is_cave_exit( mi ) )
      z0 = z;

    if ( is_no_draw( mi ) )
      cave_override = true;

    if ( z0 < lowest_z )
    {
      lowest_z = z0;
    }
  }

  if ( ( x - 1 >= 0 ) && ( y + 1 < uo_map_height ) )
  {
    safe_getmapinfo( x - 1, y + 1, &z0, &mi );

    if ( is_cave_shadow( mi ) || is_cave_exit( mi ) )
      z0 = z;

    if ( is_no_draw( mi ) )
      cave_override = true;

    if ( z0 < lowest_z )
    {
      lowest_z = z0;
    }
  }

  if ( y - 1 >= 0 )
  {
    safe_getmapinfo( x, y - 1, &z0, &mi );

    if ( is_cave_shadow( mi ) || is_cave_exit( mi ) )
      z0 = z;

    if ( is_no_draw( mi ) )
      cave_override = true;

    if ( z0 < lowest_z )
    {
      lowest_z = z0;
    }
  }

  if ( ( y - 1 >= 0 ) && ( x + 1 < uo_map_width ) )
  {
    safe_getmapinfo( x + 1, y - 1, &z0, &mi );

    if ( is_cave_shadow( mi ) || is_cave_exit( mi ) )
      z0 = z;

    if ( is_no_draw( mi ) )
      cave_override = true;

    if ( z0 < lowest_z )
    {
      lowest_z = z0;
    }
  }

  if ( x + 1 < uo_map_width )
  {
    safe_getmapinfo( x + 1, y, &z0, &mi );

    if ( is_cave_shadow( mi ) || is_cave_exit( mi ) )
      z0 = z;

    if ( is_no_draw( mi ) )
      cave_override = true;

    if ( z0 < lowest_z )
    {
      lowest_z = z0;
    }
  }

  if ( ( x + 1 < uo_map_width ) && ( y + 1 < uo_map_height ) )
  {
    safe_getmapinfo( x + 1, y + 1, &z0, &mi );

    if ( is_cave_shadow( mi ) || is_cave_exit( mi ) )
      z0 = z;

    if ( is_no_draw( mi ) )
      cave_override = true;

    if ( z0 < lowest_z )
    {
      lowest_z = z0;
    }
  }

  if ( y + 1 < uo_map_height )
  {
    safe_getmapinfo( x, y + 1, &z0, &mi );

    if ( is_cave_shadow( mi ) || is_cave_exit( mi ) )
      z0 = z;

    if ( is_no_draw( mi ) )
      cave_override = true;

    if ( z0 < lowest_z )
    {
      lowest_z = z0;
    }
  }

  if ( cave_override )
    return z;
  else
    return lowest_z;
}

void UoConvertMain::ProcessSolidBlock( unsigned short x_base, unsigned short y_base,
                                       MapWriter& mapwriter )
{
  unsigned int idx2_offset = 0;
  SOLIDX2_ELEM idx2_elem;
  memset( &idx2_elem, 0, sizeof idx2_elem );
  idx2_elem.baseindex = mapwriter.NextSolidIndex();

  unsigned short x_add_max = SOLIDX_X_SIZE, y_add_max = SOLIDX_Y_SIZE;
  if ( x_base + x_add_max > uo_map_width )
    x_add_max = uo_map_width - x_base;
  if ( y_base + y_add_max > uo_map_height )
    y_add_max = uo_map_height - y_base;

  for ( unsigned short x_add = 0; x_add < x_add_max; ++x_add )
  {
    for ( unsigned short y_add = 0; y_add < y_add_max; ++y_add )
    {
      unsigned short x = x_base + x_add;
      unsigned short y = y_base + y_add;

      StaticList statics;

      // read the map, and treat it like a static.
      short z;
      USTRUCT_MAPINFO mi;

      safe_getmapinfo( x, y, &z, &mi );

      if ( mi.landtile > 0x3FFF )
        INFO_PRINT.Format( "Tile 0x{:X} at ({},{},{}) is an invalid ID!\n" )
            << mi.landtile << x << y << z;

      // for water, don't average with surrounding tiles.
      if ( Plib::landtile_uoflags_read( mi.landtile ) & USTRUCT_TILE::FLAG_LIQUID )
        z = mi.z;
      short low_z = get_lowestadjacentz( x, y, z );

      short lt_height = z - low_z;
      z = low_z;

      if ( mi.landtile > 0x3FFF )
        INFO_PRINT.Format( "Tile 0x{:X} at ({},{},{}) is an invalid ID!\n" )
            << mi.landtile << x << y << z;

      unsigned int lt_flags = Plib::landtile_uoflags_read( mi.landtile );
      if ( ~lt_flags & USTRUCT_TILE::FLAG_BLOCKING )
      {  // this seems to be the default.
        lt_flags |= USTRUCT_TILE::FLAG_PLATFORM;
      }
      lt_flags |=
          USTRUCT_TILE::FLAG_NO_SHOOT;  // added to make sure people using noshoot will have shapes
      // generated by this tile in future block LOS, shouldn't
      // affect people using old LOS method one way or another.
      lt_flags |= USTRUCT_TILE::FLAG_FLOOR;
      lt_flags |= USTRUCT_TILE::FLAG_HALF_HEIGHT;  // the entire map is this way

      if ( lt_flags & USTRUCT_TILE::FLAG_WALL )
        lt_height = 20;

      readstatics( statics, x, y,
                   USTRUCT_TILE::FLAG_BLOCKING | USTRUCT_TILE::FLAG_PLATFORM |
                       USTRUCT_TILE::FLAG_HALF_HEIGHT | USTRUCT_TILE::FLAG_LIQUID |
                       USTRUCT_TILE::FLAG_HOVEROVER
                   // USTRUCT_TILE::FLAG__WALK
      );

      for ( unsigned i = 0; i < statics.size(); ++i )
      {
        StaticRec srec = statics[i];

        unsigned int polflags = polflags_from_tileflags( srec.graphic, srec.flags, cfg_use_no_shoot,
                                                         cfg_LOS_through_windows );

        if ( ( ~polflags & FLAG::MOVELAND ) && ( ~polflags & FLAG::MOVESEA ) &&
             ( ~polflags & FLAG::BLOCKSIGHT ) && ( ~polflags & FLAG::BLOCKING ) &&
             ( ~polflags & FLAG::OVERFLIGHT ) )
        {
          // remove it.  we'll re-sort later.
          statics.erase( statics.begin() + i );
          --i;  // do-over
        }
        if ( ( ~srec.flags & USTRUCT_TILE::FLAG_BLOCKING ) &&
             ( ~srec.flags & USTRUCT_TILE::FLAG_PLATFORM ) &&
             ( ~srec.flags & USTRUCT_TILE::FLAG_HALF_HEIGHT ) &&
             ( ~srec.flags & USTRUCT_TILE::FLAG_LIQUID ) &&
             ( ~srec.flags & USTRUCT_TILE::FLAG_HOVEROVER ) )
        /*(~srec.flags & USTRUCT_TILE::FLAG__WALK)*/
        {
          // remove it.  we'll re-sort later.
          statics.erase( statics.begin() + i );
          --i;  // do-over
        }
      }

      bool addMap = true;

      for ( const auto& srec : statics )
      {
        // Look for water tiles.  If there are any, discard the map (which is usually at -15 anyway)
        if ( z + lt_height <= srec.z &&
             ( ( srec.z - ( z + lt_height ) ) <=
               10 ) &&  // only where the map is below or same Z as the static
             srec.graphic >= 0x1796 &&
             srec.graphic <= 0x17B2 )  // FIXME hardcoded
        {
          // arr, there be water here
          addMap = false;
        }

        // if there's a static on top of one of these "wall" landtiles, make it override.
        if ( ( lt_flags & USTRUCT_TILE::FLAG_WALL ) &&  // wall?
             z <= srec.z && srec.z - z <= lt_height )
        {
          lt_height = srec.z - z;
        }
      }
      // shadows above caves
      if ( is_cave_shadow( mi ) && !statics.empty() )
      {
        addMap = false;
      }


      // If the map is a NODRAW tile, and there are statics, discard the map tile
      if ( mi.landtile == 2 && !statics.empty() )
        addMap = false;

      if ( addMap )
        statics.push_back( StaticRec( 0, static_cast<signed char>( z ), lt_flags,
                                      static_cast<char>( lt_height ) ) );

      sort( statics.begin(), statics.end(), StaticsByZ() );
      reverse( statics.begin(), statics.end() );

      std::vector<MapShape> shapes;

      // try to consolidate like shapes, and discard ones we don't care about.
      while ( !statics.empty() )
      {
        StaticRec srec = statics.back();
        statics.pop_back();

        unsigned int polflags = polflags_from_tileflags( srec.graphic, srec.flags, cfg_use_no_shoot,
                                                         cfg_LOS_through_windows );
        if ( ( ~polflags & FLAG::MOVELAND ) && ( ~polflags & FLAG::MOVESEA ) &&
             ( ~polflags & FLAG::BLOCKSIGHT ) && ( ~polflags & FLAG::BLOCKING ) &&
             ( ~polflags & FLAG::OVERFLIGHT ) )
        {
          passert_always( 0 );
          continue;
        }
        if ( ( ~srec.flags & USTRUCT_TILE::FLAG_BLOCKING ) &&
             ( ~srec.flags & USTRUCT_TILE::FLAG_PLATFORM ) &&
             ( ~srec.flags & USTRUCT_TILE::FLAG_HALF_HEIGHT ) &&
             ( ~srec.flags & USTRUCT_TILE::FLAG_LIQUID ) &&
             ( ~srec.flags & USTRUCT_TILE::FLAG_HOVEROVER ) )
        /*(~srec.flags & USTRUCT_TILE::FLAG__WALK)*/
        {
          passert_always( 0 );
          continue;
        }

        if ( shapes.empty() )
        {
          // this, whatever it is, is the map base.
          // TODO: look for water statics and use THOSE as the map.
          MapShape shape;
          shape.z = srec.z;  // these will be converted below to
          shape.height = 0;  // make the map "solid"
          shape.flags = static_cast<unsigned char>( polflags );
          // no matter what, the lowest level is gradual
          shape.flags |= FLAG::GRADUAL;
          shapes.push_back( shape );

          // for wall flag - map tile always height 0, at bottom. if map tile has height, add it as
          // a static
          if ( srec.height != 0 )
          {
            MapShape _shape;
            _shape.z = srec.z;
            _shape.height = srec.height;
            _shape.flags = polflags;
            shapes.push_back( _shape );
          }
          continue;
        }

        MapShape& prev = shapes.back();
        // we're adding it.
        MapShape shape;
        shape.z = srec.z;
        shape.height = srec.height;
        shape.flags = polflags;

        // always add the map shape seperately
        if ( shapes.size() == 1 )
        {
          shapes.push_back( shape );
          continue;
        }

        if ( shape.z < prev.z + prev.height )
        {
          // things can't exist in the same place.
          // shrink the bottom part of this shape.
          // if that would give it negative height, then skip it.
          short height_remove = prev.z + prev.height - shape.z;
          if ( height_remove <= shape.height )
          {
            shape.z += height_remove;
            shape.height -= height_remove;
          }
          else
          {  // example: 5530, 14
            continue;
          }
        }

        // sometimes water has "sand" a couple z-coords above it.
        // We'll try to detect this (really, anything that is up to 4 dist from water)
        // and extend the thing above downward.
        if ( ( prev.flags & FLAG::MOVESEA ) && ( shape.z > prev.z + prev.height ) &&
             ( shape.z <= prev.z + prev.height + 4 ) )
        {
          short height_add = shape.z - prev.z - prev.height;
          shape.z -= height_add;
          shape.height += height_add;
        }
        if ( ( prev.flags & FLAG::MOVESEA ) && ( prev.z + prev.height == -5 ) &&
             ( shape.flags & FLAG::MOVESEA ) && ( shape.z == 25 ) )
        {
          // oddly, there are some water tiles at z=25 in some places...I don't get it
          continue;
        }

        // string prevflags_s = flagstr(prev.flags);
        // const char* prevflags = prevflags_s.c_str();
        // string shapeflags_s = flagstr(shape.flags);
        // const char* shapeflags = shapeflags_s.c_str();

        if ( shape.z > prev.z + prev.height )
        {
          //
          // elevated above what's below, must include separately
          //

          shapes.push_back( shape );
          continue;
        }

        passert_always( shape.z == prev.z + prev.height );

        if ( shape.z == prev.z + prev.height )
        {
          //
          // sitting right on top of the previous solid
          //

          // standable atop non-standable: standable
          // nonstandable atop standable: nonstandable
          // etc
          bool can_combine =
              flags_match( prev.flags, shape.flags, FLAG::BLOCKSIGHT | FLAG::BLOCKING );
          if ( prev.flags & FLAG::MOVELAND && ~shape.flags & FLAG::BLOCKING &&
               ~shape.flags & FLAG::MOVELAND )
          {
            can_combine = false;
          }

          if ( can_combine )
          {
            prev.flags = shape.flags;
            prev.height += shape.height;
          }
          else  // if one blocks LOS, but not the other, they can't be combined this way.
          {
            shapes.push_back( shape );
            continue;
          }
        }
      }

      // the first StaticShape is the map base.
      MapShape base = shapes[0];
      shapes.erase( shapes.begin() );
      MAPCELL cell;
      passert_always( base.height == 0 );
      cell.z = static_cast<signed char>(
          base.z );  // assume now map has height=1. a static was already added if it was >0
      cell.flags = static_cast<u8>( base.flags );
      if ( !shapes.empty() )
        cell.flags |= FLAG::MORE_SOLIDS;

      mapwriter.SetMapCell( x, y, cell );

      if ( !shapes.empty() )
      {
        ++with_more_solids;
        total_statics += static_cast<unsigned int>( shapes.size() );
        if ( idx2_offset == 0 )
          idx2_offset = mapwriter.NextSolidx2Offset();

        unsigned int addindex = mapwriter.NextSolidIndex() - idx2_elem.baseindex;
        if ( addindex > std::numeric_limits<unsigned short>::max() )
          throw std::runtime_error( "addoffset overflow" );
        idx2_elem.addindex[x_add][y_add] = static_cast<unsigned short>( addindex );
        int count = static_cast<int>( shapes.size() );
        for ( int j = 0; j < count; ++j )
        {
          MapShape shape = shapes[j];
          char _z, height, flags;
          _z = static_cast<char>( shapes[j].z );
          height = static_cast<char>( shape.height );
          flags = static_cast<u8>( shape.flags );
          if ( !height )  // make 0 height solid
          {
            --_z;
            ++height;
          }

          if ( j != count - 1 )
            flags |= FLAG::MORE_SOLIDS;
          SOLIDS_ELEM solid;
          solid.z = _z;
          solid.height = height;
          solid.flags = flags;
          mapwriter.AppendSolid( solid );
        }
      }
    }
  }
  if ( idx2_offset )
  {
    ++nonempty;
    mapwriter.AppendSolidx2Elem( idx2_elem );
  }
  else
  {
    ++empty;
  }
  mapwriter.SetSolidx2Offset( x_base, y_base, idx2_offset );
}

void UoConvertMain::write_multi( FILE* multis_cfg, unsigned id, FILE* multi_mul,
                                 unsigned int offset, unsigned int length )
{
  USTRUCT_MULTI_ELEMENT elem;
  unsigned int count;
  if ( cfg_use_new_hsa_format )
    count = length / sizeof( USTRUCT_MULTI_ELEMENT_HSA );
  else
    count = length / sizeof elem;

  std::string type, mytype;
  if ( BoatTypes.count( id ) )
    type = "Boat";
  else if ( HouseTypes.count( id ) )
    type = "House";
  else if ( StairTypes.count( id ) )
    type = "Stairs";
  else
  {
    ERROR_PRINT << "Type 0x" << fmt::hexu( id )
                << " not found in uoconvert.cfg, assuming \"House\" type.\n";
    type = "House";
  }
  mytype = type;

  fprintf( multis_cfg, "%s 0x%x\n", type.c_str(), id );
  fprintf( multis_cfg, "{\n" );

  if ( fseek( multi_mul, offset, SEEK_SET ) != 0 )
  {
    throw std::runtime_error( "write_multi(): fseek() failed" );
  }


  bool first = true;
  while ( count-- )
  {
    if ( fread( &elem, sizeof elem, 1, multi_mul ) != 1 )
    {
      throw std::runtime_error( "write_multi(): fread() failed" );
    }

    if ( cfg_use_new_hsa_format )
    {
      if ( fseek( multi_mul, 4, SEEK_CUR ) != 0 )
        throw std::runtime_error( "write_multi(): fseek() failed" );
    }

    if ( elem.graphic == GRAPHIC_NODRAW )
      continue;

    if ( elem.flags )
      type = "static";
    else
      type = "dynamic";

    // boats typically have as their first element the "mast", but flagged as dynamic.
    if ( mytype == "Boat" )
    {
      if ( first && elem.graphic != 1 )
        type = "static";
    }
    std::string comment;
    if ( cfg_use_new_hsa_format )
    {
      USTRUCT_TILE_HSA tile;
      readtile( elem.graphic, &tile );
      comment.assign( tile.name, sizeof( tile.name ) );
    }
    else
    {
      USTRUCT_TILE tile;
      readtile( elem.graphic, &tile );
      comment.assign( tile.name, sizeof( tile.name ) );
    }
    fprintf( multis_cfg, "    %-7s 0x%04x %4d %4d %4d   // %s\n", type.c_str(), elem.graphic,
             elem.x, elem.y, elem.z, comment.c_str() );
    first = false;
  }
  fprintf( multis_cfg, "}\n" );
  fprintf( multis_cfg, "\n" );
}

void UoConvertMain::create_multis_cfg( FILE* multi_idx, FILE* multi_mul, FILE* multis_cfg )
{
  if ( fseek( multi_idx, 0, SEEK_SET ) != 0 )
    throw std::runtime_error( "create_multis_cfg: fseek failed" );
  unsigned count = 0;
  USTRUCT_IDX idxrec;
  for ( int i = 0; fread( &idxrec, sizeof idxrec, 1, multi_idx ) == 1; ++i )
  {
    const USTRUCT_VERSION* vrec = nullptr;

    if ( check_verdata( VERFILE_MULTI_MUL, i, vrec ) )
    {
      write_multi( multis_cfg, i, verfile, vrec->filepos, vrec->length );
      ++count;
    }
    else
    {
      if ( idxrec.offset == 0xFFffFFffLu )
        continue;

      write_multi( multis_cfg, i, multi_mul, idxrec.offset, idxrec.length );
      ++count;
    }
  }
  INFO_PRINT << count << " multi definitions written to multis.cfg\n";
}

void UoConvertMain::create_multis_cfg()
{
  FILE* multi_idx = open_uo_file( "multi.idx" );
  FILE* multi_mul = open_uo_file( "multi.mul" );

  std::string outdir = programArgsFindEquals( "outdir=", "." );
  FILE* multis_cfg = fopen( ( outdir + "/multis.cfg" ).c_str(), "wt" );

  create_multis_cfg( multi_idx, multi_mul, multis_cfg );

  fclose( multis_cfg );

  fclose( multi_idx );
  fclose( multi_mul );
}
void UoConvertMain::write_flags( FILE* fp, unsigned int flags )
{
  if ( flags & FLAG::MOVELAND )
    fprintf( fp, "    MoveLand 1\n" );
  if ( flags & FLAG::MOVESEA )
    fprintf( fp, "    MoveSea 1\n" );
  if ( flags & FLAG::BLOCKSIGHT )
    fprintf( fp, "    BlockSight 1\n" );
  if ( ~flags & FLAG::OVERFLIGHT )
    fprintf( fp, "    OverFlight 0\n" );
  if ( flags & FLAG::ALLOWDROPON )
    fprintf( fp, "    AllowDropOn 1\n" );
  if ( flags & FLAG::GRADUAL )
    fprintf( fp, "    Gradual 1\n" );
  if ( flags & FLAG::STACKABLE )
    fprintf( fp, "    Stackable 1\n" );
  if ( flags & FLAG::BLOCKING )
    fprintf( fp, "    Blocking 1\n" );
  if ( flags & FLAG::MOVABLE )
    fprintf( fp, "    Movable 1\n" );
  if ( flags & FLAG::EQUIPPABLE )
    fprintf( fp, "    Equippable 1\n" );
  if ( flags & FLAG::DESC_PREPEND_A )
    fprintf( fp, "    DescPrependA 1\n" );
  if ( flags & FLAG::DESC_PREPEND_AN )
    fprintf( fp, "    DescPrependAn 1\n" );
}

void UoConvertMain::create_tiles_cfg()
{
  std::string outdir = programArgsFindEquals( "outdir=", "." );
  FILE* fp = fopen( ( outdir + "/tiles.cfg" ).c_str(), "wt" );
  int mountCount;
  char name[21];

  unsigned count = 0;
  for ( unsigned int graphic_i = 0; graphic_i <= Plib::systemstate.config.max_tile_id; ++graphic_i )
  {
    u16 graphic = static_cast<u16>( graphic_i );
    USTRUCT_TILE tile;
    if ( cfg_use_new_hsa_format )
    {
      USTRUCT_TILE_HSA newtile;
      read_objinfo( graphic, newtile );
      tile.anim = newtile.anim;
      tile.flags = newtile.flags;
      tile.height = newtile.height;
      tile.layer = newtile.layer;
      memcpy( tile.name, newtile.name, sizeof tile.name );
      tile.unk14 = newtile.unk14;
      tile.unk15 = newtile.unk15;
      tile.unk6 = newtile.unk6;
      tile.unk7 = newtile.unk7;
      tile.unk8 = newtile.unk8;
      tile.unk9 = newtile.unk9;
      tile.weight = newtile.weight;
    }
    else
      read_objinfo( graphic, tile );
    mountCount = static_cast<int>( MountTypes.count( graphic ) );

    if ( tile.name[0] == '\0' && tile.flags == 0 && tile.layer == 0 && tile.height == 0 &&
         mountCount == 0 )
    {
      continue;
    }
    unsigned int flags =
        polflags_from_tileflags( graphic, tile.flags, cfg_use_no_shoot, cfg_LOS_through_windows );
    if ( mountCount != 0 )
    {
      tile.layer = 25;
      flags |= FLAG::EQUIPPABLE;
    }

    // unsigned long uoflagslong = static_cast<unsigned long>( tile.flags );
    memset( name, 0, sizeof name );
    memcpy( name, tile.name, sizeof tile.name );

    fprintf( fp, "tile 0x%x\n", graphic );
    fprintf( fp, "{\n" );
    fprintf( fp, "    Desc %s\n", name );
    fprintf( fp, "    UoFlags 0x%08lx\n", static_cast<unsigned long>( tile.flags ) );
    if ( tile.layer )
      fprintf( fp, "    Layer %u\n", tile.layer );
    if ( flags & FLAG::EQUIPPABLE )
      fprintf( fp, "    AnimID %u\n", tile.anim );
    if ( static_cast<unsigned long>( tile.flags ) & USTRUCT_TILE::FLAG_PARTIAL_HUE )
      fprintf( fp, "    PartialHue 1\n" );
    fprintf( fp, "    Height %u\n", tile.height );
    fprintf( fp, "    Weight %u\n", tile.weight );
    write_flags( fp, flags );
    fprintf( fp, "}\n" );
    fprintf( fp, "\n" );
    ++count;
  }
  fclose( fp );

  INFO_PRINT << count << " tile definitions written to tiles.cfg\n";
}

void UoConvertMain::create_landtiles_cfg()
{
  std::string outdir = programArgsFindEquals( "outdir=", "." );
  FILE* fp = fopen( ( outdir + "/landtiles.cfg" ).c_str(), "wt" );
  unsigned count = 0;

  for ( u16 i = 0; i <= 0x3FFF; ++i )
  {
    USTRUCT_LAND_TILE landtile;
    if ( cfg_use_new_hsa_format )
    {
      USTRUCT_LAND_TILE_HSA newlandtile;
      readlandtile( i, &newlandtile );
      landtile.flags = newlandtile.flags;
      landtile.unk = newlandtile.unk;
      memcpy( landtile.name, newlandtile.name, sizeof landtile.name );
    }
    else
      readlandtile( i, &landtile );

    if ( landtile.name[0] || landtile.flags )
    {
      fprintf( fp, "landtile 0x%x\n", i );
      fprintf( fp, "{\n" );
      fprintf( fp, "    Name %s\n", landtile.name );
      fprintf( fp, "    UoFlags 0x%08lx\n", static_cast<unsigned long>( landtile.flags ) );

      unsigned int flags = polflags_from_landtileflags( i, landtile.flags );
      flags &= ~FLAG::MOVABLE;  // movable makes no sense for landtiles
      write_flags( fp, flags );
      fprintf( fp, "}\n" );
      fprintf( fp, "\n" );
      ++count;
    }
  }
  fclose( fp );

  INFO_PRINT << count << " landtile definitions written to landtiles.cfg\n";
}

int UoConvertMain::main()
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

  // Setups uoconvert by finding the path of uo files and max tiles from pol.cfg or command
  // line arguments. Also loads parameters from uoconvert.cfg.
  setup_uoconvert();

  std::string command = binArgs[1];
  if ( command == "uoptomul" )
  {
    if ( !convert_uop_to_mul() )
      return 1;
  }
  else if ( command == "map" )
  {
    UoConvert::uo_mapid = programArgsFindEquals( "mapid=", 0, false );
    UoConvert::uo_usedif = programArgsFindEquals( "usedif=", 0, false );
    UoConvert::uo_readuop = (bool)programArgsFindEquals( "readuop=", 1, false );

    std::string realm = programArgsFindEquals( "realm=", "britannia" );
    int default_width = 6144;
    int default_height = 4096;
    switch ( UoConvert::uo_mapid )
    {
    case 0:
    case 1:
      break;
    case 2:  // ilshenar:
      default_width = 2304;
      default_height = 1600;
      break;
    case 3:  // malas
      default_width = 2560;
      default_height = 2048;
      break;
    case 4:  // tokuno
      default_width = 1448;
      default_height = 1448;
      break;
    case 5:  // termur
      default_width = 1280;
      default_height = 4096;
      break;
    }
    int width = programArgsFindEquals( "width=", default_width, false );
    int height = programArgsFindEquals( "height=", default_height, false );
    UoConvert::uo_map_width = static_cast<unsigned short>( width );
    UoConvert::uo_map_height = static_cast<unsigned short>( height );

    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();

    int x = programArgsFindEquals( "x=", -1, false );
    int y = programArgsFindEquals( "y=", -1, false );

    // britannia: realm=main mapid=0 width=6144 height=4096
    // ilshenar: realm=ilshenar mapid=2 width=2304 height=1600
    // malas: realm=malas mapid=3 width=2560 height=2048
    // tokuno: realm=tokuno mapid=4 width=1448 height=1448
    // termur: realm=termur mapid=5 width=1280 height=4096

    if ( x >= 0 && y >= 0 )
    {
      UoConvertMain::update_map( realm, static_cast<u16>( x ), static_cast<u16>( y ) );
    }
    else
    {
      UoConvertMain::create_map( realm, static_cast<unsigned short>( width ),
                                 static_cast<unsigned short>( height ) );
    }
  }
  else if ( command == "statics" )
  {
    std::string realm = programArgsFindEquals( "realm=", "britannia" );
    Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realm );

    UoConvert::uo_mapid = descriptor.uomapid;
    UoConvert::uo_usedif = descriptor.uodif;
    UoConvert::uo_map_width = static_cast<unsigned short>( descriptor.width );
    UoConvert::uo_map_height = static_cast<unsigned short>( descriptor.height );

    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();

    write_pol_static_files( realm );
  }
  else if ( command == "multis" )
  {
    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();
    UoConvertMain::create_multis_cfg();
  }
  else if ( command == "tiles" )
  {
    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();
    UoConvertMain::create_tiles_cfg();
  }
  else if ( command == "landtiles" )
  {
    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();
    UoConvertMain::create_landtiles_cfg();
  }
  else if ( command == "maptile" )
  {
    std::string realm = programArgsFindEquals( "realm=", "britannia" );
    Plib::RealmDescriptor descriptor = Plib::RealmDescriptor::Load( realm );

    UoConvert::uo_mapid = descriptor.uomapid;
    UoConvert::uo_usedif = descriptor.uodif;
    UoConvert::uo_map_width = static_cast<unsigned short>( descriptor.width );
    UoConvert::uo_map_height = static_cast<unsigned short>( descriptor.height );

    UoConvert::open_uo_data_files();
    UoConvert::read_uo_data();

    UoConvertMain::create_maptile( realm );
  }
  else if ( command == "flags" )
  {
    UoConvertMain::display_flags();
  }
  else  // unknown option
  {
    showHelp();
    return 1;
  }
  UoConvert::clear_tiledata();
  return 0;
}
bool UoConvertMain::convert_uop_to_mul()
{
  // this is kludgy and doesn't take into account the UODataPath. Mostly a proof of concept now.
  UoConvert::uo_mapid = programArgsFindEquals( "mapid=", 0, false );

  std::string mul_mapfile = "map" + to_string( uo_mapid ) + ".mul";
  std::string uop_mapfile = "map" + to_string( uo_mapid ) + "LegacyMUL.uop";

  auto maphash = []( int mapid, size_t chunkidx ) {
    fmt::Writer tmp;
    tmp << "build/map" << mapid << "legacymul/" << fmt::pad( chunkidx, 8, '0' ) << ".dat";
    return HashLittle2( tmp.str() );
  };

  std::ifstream ifs( uop_mapfile, std::ifstream::binary );
  if ( !ifs )
  {
    ERROR_PRINT << "Error when opening mapfile: " << uop_mapfile << '\n';
    return false;
  }

  kaitai::kstream ks( &ifs );
  uop_t uopfile( &ks );

  // TODO: read all blocks
  std::map<uint64_t, uop_t::file_t*> filemap;
  uop_t::block_addr_t* currentblock = uopfile.header()->firstblock();
  for ( auto file : *currentblock->block_body()->files() )
  {
    if ( file == nullptr )
      continue;
    if ( file->decompressed_size() == 0 )
      continue;
    filemap[file->filehash()] = file;
  }

  if ( uopfile.header()->nfiles() != filemap.size() )
    INFO_PRINT << "Warning: not all chunks read (" << filemap.size() << "/"
               << uopfile.header()->nfiles() << ")\n";

  std::ofstream ofs( mul_mapfile, std::ofstream::binary );
  for ( size_t i = 0; i < filemap.size(); i++ )
  {
    auto fileitr = filemap.find( maphash( uo_mapid, i ) );
    if ( fileitr == filemap.end() )
    {
      INFO_PRINT << "Couldn't find file hash: " << maphash( uo_mapid, i );
      continue;
    }

    auto file = fileitr->second;
    ofs << file->data()->filebytes();
    INFO_PRINT << "Wrote: " << i + 1 << "/" << filemap.size() << '\n';
  }
  INFO_PRINT << "Done converting.\n";

  return true;
}
void UoConvertMain::setup_uoconvert()
{
  std::string uodata_root = programArgsFindEquals( "uodata=", "" );
  unsigned short max_tile =
      static_cast<unsigned short>( programArgsFindEquals( "maxtileid=", 0x0, true ) );

  // if any of the two is missing, read from pol.cfg
  if ( uodata_root.empty() || !max_tile )
  {
    INFO_PRINT << "Reading pol.cfg.\n";
    Clib::ConfigFile cf( "pol.cfg" );
    Clib::ConfigElem elem;

    cf.readraw( elem );

    if ( uodata_root.empty() )
      uodata_root = Plib::UOInstallFinder::remove_elem( elem );

    if ( !max_tile )
      max_tile = elem.remove_ushort( "MaxTileID", 0x0 );
  }

  if ( max_tile != UOBJ_DEFAULT_MAX && max_tile != UOBJ_SA_MAX && max_tile != UOBJ_HSA_MAX )
    max_tile = UOBJ_DEFAULT_MAX;

  // Save the parameters into this ugly global state we have
  Plib::systemstate.config.max_tile_id = max_tile;
  Plib::systemstate.config.uo_datafile_root = Clib::normalized_dir_form( uodata_root );

  // Load parameters from uoconvert.cfg (multi types, mounts, etc)
  load_uoconvert_cfg();
}
void UoConvertMain::load_uoconvert_cfg()
{
  std::string main_cfg = "uoconvert.cfg";
  if ( Clib::FileExists( main_cfg ) )
  {
    std::string temp;
    Clib::ConfigElem elem;
    INFO_PRINT << "Reading uoconvert.cfg.\n";
    Clib::ConfigFile cf_main( main_cfg );
    while ( cf_main.read( elem ) )
    {
      if ( elem.type_is( "MultiTypes" ) )
      {
        temp = elem.remove_string( "Boats" );
        ISTRINGSTREAM is_boats( temp );
        std::string graphicnum;
        while ( is_boats >> graphicnum )
          BoatTypes.insert( strtoul( graphicnum.c_str(), nullptr, 0 ) );

        temp = elem.remove_string( "Houses" );
        ISTRINGSTREAM is_houses( temp );
        while ( is_houses >> graphicnum )
          HouseTypes.insert( strtoul( graphicnum.c_str(), nullptr, 0 ) );

        temp = elem.remove_string( "Stairs" );
        ISTRINGSTREAM is_stairs( temp );
        while ( is_stairs >> graphicnum )
          StairTypes.insert( strtoul( graphicnum.c_str(), nullptr, 0 ) );
      }
      else if ( elem.type_is( "LOSOptions" ) )
      {
        if ( elem.has_prop( "UseNoShoot" ) )
          UoConvertMain::cfg_use_no_shoot = elem.remove_bool( "UseNoShoot" );

        if ( elem.has_prop( "LOSThroughWindows" ) )
          UoConvertMain::cfg_LOS_through_windows = elem.remove_bool( "LOSThroughWindows" );
      }
      else if ( elem.type_is( "Mounts" ) )
      {
        std::string graphicnum;
        temp = elem.remove_string( "Tiles" );
        ISTRINGSTREAM is_mounts( temp );
        while ( is_mounts >> graphicnum )
        {
          MountTypes.insert( strtoul( graphicnum.c_str(), nullptr, 0 ) );
        }
      }
      else if ( elem.type_is( "StaticOptions" ) )
      {
        if ( elem.has_prop( "MaxStaticsPerBlock" ) )
        {
          UoConvert::cfg_max_statics_per_block = elem.remove_int( "MaxStaticsPerBlock" );

          if ( UoConvert::cfg_max_statics_per_block > MAX_STATICS_PER_BLOCK )
          {
            UoConvert::cfg_max_statics_per_block = MAX_STATICS_PER_BLOCK;
            INFO_PRINT << "max. Statics per Block limited to "
                       << UoConvert::cfg_max_statics_per_block << " Items\n";
          }
          else if ( UoConvert::cfg_max_statics_per_block < 0 )
            UoConvert::cfg_max_statics_per_block = 1000;
        }

        if ( elem.has_prop( "WarningStaticsPerBlock" ) )
        {
          UoConvert::cfg_warning_statics_per_block = elem.remove_int( "WarningStaticsPerBlock" );

          if ( UoConvert::cfg_warning_statics_per_block > MAX_STATICS_PER_BLOCK )
          {
            UoConvert::cfg_warning_statics_per_block = MAX_STATICS_PER_BLOCK;
            INFO_PRINT << "max. Statics per Block for Warning limited to "
                       << UoConvert::cfg_warning_statics_per_block << " Items\n";
          }
          else if ( UoConvert::cfg_warning_statics_per_block < 0 )
            UoConvert::cfg_warning_statics_per_block = 1000;
        }

        if ( elem.has_prop( "ShowIllegalGraphicWarning" ) )
          UoConvert::cfg_show_illegal_graphic_warning =
              elem.remove_bool( "ShowIllegalGraphicWarning" );
      }
      else if ( elem.type_is( "TileOptions" ) )
      {
        if ( elem.has_prop( "ShowRoofAndPlatformWarning" ) )
          cfg_show_roof_and_platform_warning = elem.remove_bool( "ShowRoofAndPlatformWarning" );
      }
      else if ( elem.type_is( "ClientOptions" ) )
      {
        if ( elem.has_prop( "UseNewHSAFormat" ) )
          UoConvert::cfg_use_new_hsa_format = elem.remove_bool( "UseNewHSAFormat" );
      }
    }
  }
}
}  // namespace UoConvert
}  // namespace Pol

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
  Pol::UoConvert::UoConvertMain* UoConvertMain = new Pol::UoConvert::UoConvertMain();
  UoConvertMain->start( argc, argv );
}
