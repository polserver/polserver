/*
History
=======
2005/01/23 Shinigami: check_item_integrity & check_character_integrity - fix for multi realm support (had used WGRID_X & WGRID_Y)
                      ClrCharacterWorldPosition - Tokuno MapDimension doesn't fit blocks of 64x64 (WGRID_SIZE)
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2012/02/06 MuadDib:   Added Old Serial for debug on orphaned items that make it to remove_item_from_world.

Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/endian.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"

#include "../plib/realm.h"
#include "mobile/charactr.h"
#include "item/item.h"
#include "multi/multi.h"
#include "realms.h"
#include "uvars.h"
#include "uworld.h"
namespace Pol {
  namespace Core {
	void add_item_to_world( Items::Item* item )
	{
	  Zone& zone = getzone( item->x, item->y, item->realm );

      passert( std::find( zone.items.begin(), zone.items.end(), item ) == zone.items.end() );

	  ++item->realm->toplevel_item_count;
	  zone.items.push_back( item );
	}

	void remove_item_from_world( Items::Item* item )
	{
	  Zone& zone = getzone( item->x, item->y, item->realm );

      ZoneItems::iterator itr = std::find( zone.items.begin(), zone.items.end(), item );
	  if ( itr == zone.items.end() )
	  {
        POLLOG_ERROR.Format( "remove_item_from_world: item 0x{:X} at {},{} does not exist in world zone ( Old Serial: 0x{:X} )\n" )
          << item->serial << item->x << item->y << ( cfBEu32( item->serial_ext ) );

		passert( itr != zone.items.end() );
	  }

	  --item->realm->toplevel_item_count;
	  zone.items.erase( itr );
	}

	void add_multi_to_world( Multi::UMulti* multi )
	{
	  Zone& zone = getzone( multi->x, multi->y, multi->realm );
	  zone.multis.push_back( multi );
	}

	void remove_multi_from_world( Multi::UMulti* multi )
	{
	  Zone& zone = getzone( multi->x, multi->y, multi->realm );
      ZoneMultis::iterator itr = std::find( zone.multis.begin(), zone.multis.end(), multi );

	  passert( itr != zone.multis.end() );

	  zone.multis.erase( itr );
	}

	void move_multi_in_world( unsigned short oldx, unsigned short oldy,
							  unsigned short newx, unsigned short newy,
							  Multi::UMulti* multi, Plib::Realm* oldrealm )
	{
	  Zone& oldzone = getzone( oldx, oldy, oldrealm );
	  Zone& newzone = getzone( newx, newy, multi->realm );

	  if ( &oldzone != &newzone )
	  {
        ZoneMultis::iterator itr = std::find( oldzone.multis.begin( ), oldzone.multis.end( ), multi );
		passert( itr != oldzone.multis.end() );
		oldzone.multis.erase( itr );

		newzone.multis.push_back( multi );
	  }
	}

	int get_toplevel_item_count()
	{
	  int count = 0;
	  for ( const auto &realm : *Realms)
        count += realm->toplevel_item_count;
	  return count;
	}

	int get_mobile_count()
	{
	  int count = 0;
      for ( const auto &realm : *Realms )
        count += realm->mobile_count;
	  return count;
	}

	//4-17-04 Rac destroyed the world! in favor of splitting its duties amongst the realms
	//World world;

    void SetCharacterWorldPosition( Mobile::Character* chr )
    {
      Zone& zone = getzone( chr->x, chr->y, chr->realm );

      auto set_pos = [&]( ZoneCharacters &set )
      {
        passert( std::find( set.begin(), set.end(), chr ) == set.end() );
        ++chr->realm->mobile_count;
        set.push_back( chr );
      };

      if ( chr->isa( Core::UObject::CLASS_NPC ) )
        set_pos( zone.npcs );
      else
        set_pos( zone.characters );
    }

	void ClrCharacterWorldPosition( Mobile::Character* chr, const char* reason )
	{
	  Zone& zone = getzone( chr->x, chr->y, chr->realm );
	  unsigned wgridx = chr->realm->width() / WGRID_SIZE;
	  unsigned wgridy = chr->realm->height() / WGRID_SIZE;

	  // Tokuno-Fix
	  if ( wgridx * WGRID_SIZE < chr->realm->width() )
		wgridx++;
	  if ( wgridy * WGRID_SIZE < chr->realm->height() )
		wgridy++;

      auto clear_pos = [&]( ZoneCharacters &set )
      {
        auto itr = std::find( set.begin(), set.end(), chr );
        if ( itr == set.end() )
        {
          POLLOG_ERROR.Format( "ClrCharacterWorldPosition({}): mob (0x{:X},0x{:X}) supposedly at ({},{}) isn't in correct zone\n" )
            << reason << chr->serial << chr->serial_ext << chr->x << chr->y;
          bool is_npc = chr->isa( Core::UObject::CLASS_NPC );
          for ( unsigned zonex = 0; zonex < wgridx; ++zonex )
          {
            for ( unsigned zoney = 0; zoney < wgridy; ++zoney )
            {
              bool found = false;
              if ( is_npc )
              {
                auto _z = chr->realm->zone[zonex][zoney].npcs;
                found = std::find( _z.begin(), _z.end(), chr ) != _z.end();
              }
              else
              {
                auto _z = chr->realm->zone[zonex][zoney].characters;
                found = std::find( _z.begin(), _z.end(), chr ) != _z.end();
              }
              if ( found )
                POLLOG_ERROR.Format( "ClrCharacterWorldPosition: Found mob in zone ({},{})\n" )
                << zonex << zoney;
            }
          }
          passert( itr != set.end() );
        }
        --chr->realm->mobile_count;
        set.erase( itr );
      };

      if ( !chr->isa( Core::UObject::CLASS_NPC ) )
        clear_pos( zone.characters );
      else
        clear_pos( zone.npcs );
	}

	void MoveCharacterWorldPosition( unsigned short oldx, unsigned short oldy,
									 unsigned short newx, unsigned short newy,
                                     Mobile::Character* chr, Plib::Realm* oldrealm )
	{
	  if ( oldrealm == NULL )
		oldrealm = chr->realm;
	  Zone& oldzone = getzone( oldx, oldy, oldrealm );
	  Zone& newzone = getzone( newx, newy, chr->realm );

	  if ( &oldzone != &newzone )
	  {
        auto move_pos = [&]( ZoneCharacters &oldset, ZoneCharacters &newset )
        {
          passert( std::find( oldset.begin(), oldset.end(), chr ) != oldset.end() );
          passert( std::find( newset.begin(), newset.end(), chr ) == newset.end() );

          oldset.erase( std::find( oldset.begin(), oldset.end(), chr ) );

          newset.push_back( chr );
          if ( chr->realm != oldrealm )
          {
            --oldrealm->mobile_count;
            ++chr->realm->mobile_count;
          }
        };
        if ( !chr->isa( Core::UObject::CLASS_NPC ) )
          move_pos( oldzone.characters, newzone.characters );
        else
          move_pos( oldzone.npcs, newzone.npcs );
	  }
	}

	void MoveItemWorldPosition( unsigned short oldx, unsigned short oldy,
                                Items::Item* item, Plib::Realm* oldrealm )
	{
	  if ( oldrealm == NULL )
		oldrealm = item->realm;
	  Zone& oldzone = getzone( oldx, oldy, oldrealm );
	  Zone& newzone = getzone( item->x, item->y, item->realm );

	  if ( &oldzone != &newzone )
	  {
        ZoneItems::iterator itr = std::find( oldzone.items.begin(), oldzone.items.end(), item );

		if ( itr == oldzone.items.end() )
		{
          POLLOG_ERROR.Format( "MoveItemWorldPosition: item 0x{:X} at old-x/y({},{} - {}) new-x/y({},{} - {}) does not exist in world zone. \n" )
            << item->serial << oldx << oldy << oldrealm->name() << item->x << item->y << item->realm->name();

		  passert( itr != oldzone.items.end() );
		}

		oldzone.items.erase( itr );

        passert( std::find( newzone.items.begin(), newzone.items.end(), item ) == newzone.items.end() );
		newzone.items.push_back( item );
	  }
	}

	// Dave added this for debugging a single zone

	bool check_single_zone_item_integrity( int x, int y, Plib::Realm* realm )
	{
	  try
	  {
		ZoneItems& witem = realm->zone[x][y].items;

        for ( const auto &item : witem )
        {
          unsigned short wx, wy;
          zone_convert( item->x, item->y, wx, wy, realm );
          if ( wx != x || wy != y )
          {
            POLLOG_ERROR.Format( "Item 0x{:X} in zone ({},{}) but location is ({},{}) (zone {},{})\n" )
              << item->serial
              << x << y
              << item->x << item->y
              << wx << wy;
            return false;
          }
        }
	  }
	  catch ( ... )
	  {
        POLLOG_ERROR.Format( "item integ problem at zone ({},{})\n" ) << x << y;
		return false;
	  }
	  return true;
	}


	bool check_item_integrity()
	{
	  bool ok = true;
	  for ( auto &realm : *Realms )
	  {
		unsigned int gridwidth = realm->width() / WGRID_SIZE;
		unsigned int gridheight = realm->height() / WGRID_SIZE;

		// Tokuno-Fix
		if ( gridwidth * WGRID_SIZE < realm->width() )
		  gridwidth++;
		if ( gridheight * WGRID_SIZE < realm->height() )
		  gridheight++;

		for ( unsigned x = 0; x < gridwidth; ++x )
		{
		  for ( unsigned y = 0; y < gridheight; ++y )
		  {
			if ( !check_single_zone_item_integrity( x, y, realm ) )
			  ok = false;
		  }
		}
	  }
	  return ok;
	}

	void check_character_integrity()
	{
	  // TODO: iterate through the object hash?
	  //for( unsigned i = 0; i < characters.size(); ++i )
	  //{
	  //    Character* chr = characters[i];
	  //    unsigned short wx, wy;
	  //    w_convert( chr->x, chr->y, wx, wy );
	  //    if (!world.zone[wx][wy].characters.count(chr))
	  //    {
	  //        cout << "Character " << chr->serial << " at " << chr->x << "," << chr->y << " is not in its zone." << endl;
	  //    }
	  //}
	  for ( auto &realm : *Realms )
	  {
		unsigned int gridwidth = realm->width() / WGRID_SIZE;
		unsigned int gridheight = realm->height() / WGRID_SIZE;

		// Tokuno-Fix
		if ( gridwidth * WGRID_SIZE < realm->width() )
		  gridwidth++;
		if ( gridheight * WGRID_SIZE < realm->height() )
		  gridheight++;

        auto check_zone = []( Mobile::Character* chr, unsigned y, unsigned x )
        {
          unsigned short wx, wy;
          zone_convert( chr->x, chr->y, wx, wy, chr->realm );
          if ( wx != x || wy != y )
            INFO_PRINT << "Character 0x" << fmt::hexu( chr->serial ) << " in a zone, but elsewhere\n";
        };

		for ( unsigned x = 0; x < gridwidth; ++x )
		{
		  for ( unsigned y = 0; y < gridheight; ++y )
		  {
            for ( const auto &chr : realm->zone[x][y].characters )
              check_zone( chr, y, x );
            for ( const auto &chr : realm->zone[x][y].npcs )
              check_zone( chr, y, x );
		  }
		}
	  }
	}

    // reallocates all vectors to fit the current size
    void optimize_zones( )
    {
      for ( auto &realm : *Realms )
      {
        unsigned int gridwidth = realm->width( ) / WGRID_SIZE;
        unsigned int gridheight = realm->height( ) / WGRID_SIZE;

        // Tokuno-Fix
        if ( gridwidth * WGRID_SIZE < realm->width( ) )
          gridwidth++;
        if ( gridheight * WGRID_SIZE < realm->height( ) )
          gridheight++;

        for ( unsigned x = 0; x < gridwidth; ++x )
        {
          for ( unsigned y = 0; y < gridheight; ++y )
          {
            realm->zone[x][y].characters.shrink_to_fit();
            realm->zone[x][y].npcs.shrink_to_fit();
            realm->zone[x][y].items.shrink_to_fit();
            realm->zone[x][y].multis.shrink_to_fit();
          }
        }
      }
    }
  }
}