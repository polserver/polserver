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

	typedef std::set<Mobile::Character*> ZoneCharacters;
	typedef std::vector<Multi::UMulti*> ZoneMultis;
	typedef std::vector<Items::Item*> ZoneItems;

	struct Zone
	{
	  ZoneCharacters characters;
	  ZoneItems items;
	  ZoneMultis multis;
	};

	const unsigned WGRID_SIZE = 64;
	const unsigned WGRID_SHIFT = 6;

	const unsigned WGRID_X = WORLD_X / WGRID_SIZE; // 6144 / 64 = 96
	const unsigned WGRID_Y = WORLD_Y / WGRID_SIZE; // 4096 / 64 = 64

	class World
	{
	public:
	  World();

	  //Items::iterator find_item( unsigned int serial );
	  //Items::iterator find_item( unsigned int serial, unsigned short x, unsigned short y );

	  Zone zone[WGRID_X][WGRID_Y];
	  unsigned int toplevel_item_count;
	  unsigned int mobile_count;
	};
	extern World world;

	inline void zone_convert( unsigned short x, unsigned short y, unsigned short& wx, unsigned short& wy, Plib::Realm* realm )
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

	template <class A>
	void ForEachMobileInRange( u16 x, u16 y, Plib::Realm* realm, unsigned range,
							   void( *f )( Mobile::Character*, A* staticdata ), A* staticdata )
	{
	  unsigned short wxL, wyL, wxH, wyH;

	  zone_convert_clip( x - range, y - range, realm, wxL, wyL );
	  zone_convert_clip( x + range, y + range, realm, wxH, wyH );
	  passert( wxL <= wxH );
	  passert( wyL <= wyH );
	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          for (auto &chr : realm->zone[wx][wy].characters)
			( *f )( chr, staticdata );
		}
	  }
	}
	template <class A>
	void ForEachMobileInVisualRange( const Mobile::Character* chr,
									 void( *f )( Mobile::Character*, A* staticdata ), A* staticdata )
	{
	  ForEachMobileInRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, f, staticdata );
	}

	template <class A>
	void ForEachItemInRange( u16 x, u16 y, Plib::Realm* realm, unsigned range,
							 void( *f )( Items::Item*, A* staticdata ), A* staticdata )
	{
	  unsigned short wxL, wyL, wxH, wyH;

	  zone_convert_clip( x - range, y - range, realm, wxL, wyL );
	  zone_convert_clip( x + range, y + range, realm, wxH, wyH );
	  passert( wxL <= wxH );
	  passert( wyL <= wyH );
	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          for (auto &item : realm->zone[wx][wy].items)
            ( *f )( item, staticdata );
		}
	  }
	}
	template <class A>
	void ForEachItemInVisualRange( const Mobile::Character* chr,
								   void( *f )( Items::Item*, A* staticdata ), A* staticdata )
	{
	  ForEachItemInRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, f, staticdata );
	}
  }
}
#endif
