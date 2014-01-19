/*
History
=======
2006/09/27 Shinigami: GCC 3.4.x fix - added "#include "charactr.h" because of ForEachMobileInVisualRange
2009/10/17 Turley:    added ForEachItemInRange & ForEachItemInVisualRange

Notes
=======

*/


#ifndef UWORLD_H
#define UWORLD_H

#include <set>

#include <assert.h>

#include "../clib/passert.h"
#include "../plib/realm.h"
#include "mobile/charactr.h"
#include "uvars.h"
#include "zone.h"
namespace Pol {
  namespace Items {
	class Item;
  }
  namespace Multi {
	class UMulti;
  }
  namespace Plib {
	class Realm;
  }
  namespace Core {

	void add_item_to_world( Items::Item* item );
	void remove_item_from_world( Items::Item* item );

	void add_multi_to_world( Multi::UMulti* multi );
	void remove_multi_from_world( Multi::UMulti* multi );
	void move_multi_in_world( unsigned short oldx, unsigned short oldy,
							  unsigned short newx, unsigned short newy,
							  Multi::UMulti* multi, Plib::Realm* oldrealm );

	void SetCharacterWorldPosition( Mobile::Character* chr );
	void ClrCharacterWorldPosition( Mobile::Character* chr, const char* reason );
	void MoveCharacterWorldPosition( unsigned short oldx, unsigned short oldy,
									 unsigned short newx, unsigned short newy,
									 Mobile::Character* chr, Plib::Realm* oldrealm );

	void SetItemWorldPosition( Items::Item* item );
	void ClrItemWorldPosition( Items::Item* item );
	void MoveItemWorldPosition( unsigned short oldx, unsigned short oldy,
								Items::Item* item, Plib::Realm* oldrealm );

	int get_toplevel_item_count();
	int get_mobile_count();

    void optimize_zones();

	typedef std::vector<Mobile::Character*> ZoneCharacters;
	typedef std::vector<Multi::UMulti*> ZoneMultis;
	typedef std::vector<Items::Item*> ZoneItems;

	struct Zone
	{
	  ZoneCharacters characters;
      ZoneCharacters npcs;
	  ZoneItems items;
	  ZoneMultis multis;
	};

	const unsigned WGRID_SIZE = 64;
	const unsigned WGRID_SHIFT = 6;

	const unsigned WGRID_X = WORLD_X / WGRID_SIZE; // 6144 / 64 = 96
	const unsigned WGRID_Y = WORLD_Y / WGRID_SIZE; // 4096 / 64 = 64

    inline void zone_convert( unsigned short x, unsigned short y, unsigned short& wx, unsigned short& wy, const Plib::Realm* realm )
	{
	  passert( x < realm->width() );
	  passert( y < realm->height() );

	  wx = x >> WGRID_SHIFT;
	  wy = y >> WGRID_SHIFT;
	}

	inline void zone_convert_clip( int x, int y, const Plib::Realm* realm, unsigned short& wx, unsigned short& wy )
	{
	  if ( x < 0 )
		x = 0;
	  if ( y < 0 )
		y = 0;
	  if ( (unsigned)x >= realm->width() )
		x = realm->width() - 1;
	  if ( (unsigned)y >= realm->height() )
		y = realm->height() - 1;

	  wx = static_cast<unsigned short>( x >> WGRID_SHIFT );
	  wy = static_cast<unsigned short>( y >> WGRID_SHIFT );
	}

	inline Zone& getzone( unsigned short x, unsigned short y, Plib::Realm* realm )
	{
	  passert( x < realm->width() );
	  passert( y < realm->height() );

	  return realm->zone[x >> WGRID_SHIFT][y >> WGRID_SHIFT];
	}

    // Helper functions to iterator over world items in given realm an range
    // takes any function with only one open parameter
    // its recommended to use lambdas even if std::bind can also be used, but a small benchmark showed
    // that std::bind is a bit slower (compiler can optimize better lambdas)
    // Usage:
    // WorldIterator<PlayerFilter>::InRange(...)

    namespace {
      // template independent code
      struct WorldIteratorHelper
      {
        struct Coords
        {
          u16 wxL;
          u16 wyL;
          u16 wxH;
          u16 wyH;
          int xL;
          int yL;
          int xH;
          int yH;
        };
        static bool inRange( const UObject *obj, const Coords& coords )
        {
          return ( obj->x >= coords.xL && obj->x <= coords.xH &&
                   obj->y >= coords.yL && obj->y <= coords.yH );
        }

        static bool validateParams( u16 x, u16 y, const Plib::Realm* realm, unsigned range, Coords* coords )
        {
          if ( realm == nullptr )
            return false;
          zone_convert_clip( x - range, y - range, realm, coords->wxL, coords->wyL );
          zone_convert_clip( x + range, y + range, realm, coords->wxH, coords->wyH );
          passert( coords->wxL <= coords->wxH );
          passert( coords->wyL <= coords->wyH );
          coords->xL = x - range;
          if ( coords->xL < 0 )
            coords->xL = 0;
          coords->yL = y - range;
          if ( coords->yL < 0 )
            coords->yL = 0;
          coords->xH = x + range;
          coords->yH = y + range;
          return true;
        }

        static bool validateParams( u16 x1, u16 y1, u16 x2, u16 y2, const Plib::Realm* realm, Coords* coords )
        {
          if ( realm == nullptr )
            return false;
          zone_convert_clip( x1, y1, realm, coords->wxL, coords->wyL );
          zone_convert_clip( x2, y2, realm, coords->wxH, coords->wyH );
          passert( coords->wxL <= coords->wxH );
          passert( coords->wyL <= coords->wyH );
          coords->xL = x1;
          coords->yL = y1;
          coords->xH = x2;
          coords->yH = y2;
          return true;
        }
      };
    }


    // main struct, define as template param which filter to use
    template <class Filter>
    struct WorldIterator
    {
      template <typename F>
      static void InRange( u16 x, u16 y, const Plib::Realm* realm, unsigned range, F &&f )
      {
        WorldIteratorHelper::Coords coords;
        if ( !WorldIteratorHelper::validateParams( x, y, realm, range, &coords ) )
          return;
        _forEach( coords, realm, std::forward<F>( f ) );
      }
      template <typename F>
      static void InVisualRange( const UObject* chr, F &&f )
      {
        InRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, std::forward<F>( f ) );
      }
      template <typename F>
      static void InBox( u16 x1, u16 y1, u16 x2, u16 y2, const Plib::Realm* realm, F &&f )
      {
        WorldIteratorHelper::Coords coords;
        if ( !WorldIteratorHelper::validateParams( x1, y1, x2, y2, realm, &coords ) )
          return;
        _forEach( coords, realm, std::forward<F>( f ) );
      }
    protected:
      template <typename F>
      static void _forEach( const WorldIteratorHelper::Coords &coords,
                            const Plib::Realm* realm, F &&f )
      {
        for ( u16 wx = coords.wxL; wx <= coords.wxH; ++wx )
        {
          for ( u16 wy = coords.wyL; wy <= coords.wyH; ++wy )
          {
            Filter::call( realm->zone[wx][wy], coords, f );
          }
        }
      }
    };

    // iterator over npcs and players
    struct MobileFilter
    {
      friend struct WorldIterator<MobileFilter>;
    protected:
      template <typename F>
      static void call( Core::Zone &zone, const WorldIteratorHelper::Coords &coords, F &&f )
      {
        for ( auto &chr : zone.characters )
        {
          if ( WorldIteratorHelper::inRange( chr, coords ) )
            f( chr );
        }
        for ( auto &chr : zone.npcs )
        {
          if ( WorldIteratorHelper::inRange( chr, coords ) )
            f( chr );
        }
      }
    };

    // iterator over player
    struct PlayerFilter
    {
      friend struct WorldIterator<PlayerFilter>;
    protected:
      template <typename F>
      static void call( Core::Zone &zone, const WorldIteratorHelper::Coords &coords, F &&f )
      {
        for ( auto &chr : zone.characters )
        {
          if ( WorldIteratorHelper::inRange( chr, coords ) )
            f( chr );
        }
      }
    };

    // iterator over npcs 
    struct NPCFilter
    {
      friend struct WorldIterator<NPCFilter>;
    protected:
      template <typename F>
      static void call( Core::Zone &zone, const WorldIteratorHelper::Coords &coords, F &&f )
      {
        for ( auto &npc : zone.npcs )
        {
          if ( WorldIteratorHelper::inRange( npc, coords ) )
            f( npc );
        }
      }
    };

    // iterator over items
    struct ItemFilter
    {
      friend struct WorldIterator<ItemFilter>;
    protected:
      template <typename F>
      static void call( Core::Zone &zone, const WorldIteratorHelper::Coords &coords, F &&f )
      {
        for ( auto &item : zone.items )
        {
          if ( WorldIteratorHelper::inRange( item, coords ) )
            f( item );
        }
      }
    };

    // iterator over multis
    struct MultiFilter
    {
      friend struct WorldIterator<MultiFilter>;
    protected:
      template <typename F>
      static void call( Core::Zone &zone, const WorldIteratorHelper::Coords &coords, F &&f )
      {
        for ( auto &multi : zone.multis )
        {
          if ( WorldIteratorHelper::inRange( multi, coords ) )
            f( multi );
        }
      }
    };
  }
}
#endif
