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

	template <class A>
	void ForEachMobileInRange( u16 x, u16 y, Plib::Realm* realm, unsigned range,
							   void( *f )( Mobile::Character*, A* staticdata ), A* staticdata )
	{
      if ( realm == nullptr )
        return;
	  unsigned short wxL, wyL, wxH, wyH;

	  zone_convert_clip( x - range, y - range, realm, wxL, wyL );
	  zone_convert_clip( x + range, y + range, realm, wxH, wyH );
	  passert( wxL <= wxH );
	  passert( wyL <= wyH );
      int xL = x - range;
      if ( xL < 0 )
        xL = 0;
      int yL = y - range;
      if ( yL < 0 )
        yL = 0;
      int xH = x + range;
      int yH = y + range;
	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          for (auto &chr : realm->zone[wx][wy].characters)
          {
            if ( chr->x >= xL && chr->x <= xH &&
                 chr->y >= yL && chr->y <= yH )
                 ( *f )( chr, staticdata );
          }
          for ( auto &chr : realm->zone[wx][wy].npcs )
          {
            if ( chr->x >= xL && chr->x <= xH &&
                 chr->y >= yL && chr->y <= yH )
                 ( *f )( chr, staticdata );
          }
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
      if ( realm == nullptr )
        return;
      unsigned short wxL, wyL, wxH, wyH;

      zone_convert_clip( x - range, y - range, realm, wxL, wyL );
      zone_convert_clip( x + range, y + range, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      int xL = x - range;
      if ( xL < 0 )
        xL = 0;
      int yL = y - range;
      if ( yL < 0 )
        yL = 0;
      int xH = x + range;
      int yH = y + range;
      for ( unsigned short wx = wxL; wx <= wxH; ++wx )
      {
        for ( unsigned short wy = wyL; wy <= wyH; ++wy )
        {
          for ( auto &item : realm->zone[wx][wy].items )
          {
            if ( item->x >= xL && item->x <= xH &&
                 item->y >= yL && item->y <= yH )
              ( *f )( item, staticdata );
          }
        }
      }
    }
    template <class A>
    void ForEachItemInVisualRange( const Mobile::Character* chr,
                                   void( *f )( Items::Item*, A* staticdata ), A* staticdata )
    {
      ForEachItemInRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, f, staticdata );
    }

    namespace { // hide the implementation helper functions
      template <typename F>
      void iter_chr( Core::Zone &zone, F &f, u16 xL, u16 yL, u16 xH, u16 yH )
      {
        for ( auto &chr : zone.characters )
        {
          if ( chr->x >= xL && chr->x <= xH &&
               chr->y >= yL && chr->y <= yH )
               f( chr );
        }
      }
      template <typename F>
      void iter_npc( Core::Zone &zone, F &f, u16 xL, u16 yL, u16 xH, u16 yH )
      {
        for ( auto &npc : zone.npcs )
        {
          if ( npc->x >= xL && npc->x <= xH &&
               npc->y >= yL && npc->y <= yH )
               f( npc );
        }
      }
      template <typename F>
      void iter_item( Core::Zone &zone, F &f, u16 xL, u16 yL, u16 xH, u16 yH )
      {
        for ( auto &item : zone.items )
        {
          if ( item->x >= xL && item->x <= xH &&
               item->y >= yL && item->y <= yH )
               f( item );
        }
      }
      template <typename F>
      void iter_multi( Core::Zone &zone, F &f, u16 xL, u16 yL, u16 xH, u16 yH )
      {
        for ( auto &multi : zone.multis )
        {
          if ( multi->x >= xL && multi->x <= xH &&
               multi->y >= yL && multi->y <= yH )
               f( multi );
        }
      }
      template <typename F, typename I>
      void _ForEach( I &&i, u16 wxL, u16 wyL, u16 wxH, u16 wyH,
                     const Plib::Realm* realm, F &&f,
                     u16 xL, u16 yL, u16 xH, u16 yH)
      {
        for ( u16 wx = wxL; wx <= wxH; ++wx )
        {
          for ( u16 wy = wyL; wy <= wyH; ++wy )
          {
            i( realm->zone[wx][wy], f, xL, yL, xH, yH );
          }
        }
      }
    }

    // Helper functions to iterator over world items in given realm an range
    // takes any function with only one open parameter
    // its recommended to use lambdas even if std::bind can also be used, but a small benchmark showed
    // that std::bind is a bit slower (compiler can optimize better lambdas)

    // iterate over all mobiles (player+npcs) at given loc with given range
    template <typename F>
    void ForEachMobileInRange( u16 x, u16 y, const Plib::Realm* realm, unsigned range, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x - range, y - range, realm, wxL, wyL );
      zone_convert_clip( x + range, y + range, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      int xL = x - range;
      if ( xL < 0 )
        xL = 0;
      int yL = y - range;
      if ( yL < 0 )
        yL = 0;
      int xH = x + range;
      int yH = y + range;
      _ForEach( iter_chr<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, xL, yL, xH, yH );
      _ForEach( iter_npc<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, xL, yL, xH, yH );
    }

    // iterate over all mobiles (player+npcs) around chr in visual range
    template <typename F>
    void ForEachMobileInVisualRange( const Mobile::Character* chr, F &&f )
    {
      ForEachMobileInRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, f );
    }
    // iterate over all mobiles (player+npcs) in given box
    template <typename F>
    void ForEachMobileInBox( u16 x1, u16 y1, u16 x2, u16 y2, const Plib::Realm* realm, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x1, y1, realm, wxL, wyL );
      zone_convert_clip( x2, y2, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      _ForEach( iter_chr<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, x1, y1, x2, y2 );
      _ForEach( iter_npc<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, x1, y1, x2, y2 );
    }

    // iterate over all players at given loc with given range
    template <typename F>
    void ForEachPlayerInRange( u16 x, u16 y, const Plib::Realm* realm, unsigned range, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x - range, y - range, realm, wxL, wyL );
      zone_convert_clip( x + range, y + range, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      int xL = x - range;
      if ( xL < 0 )
        xL = 0;
      int yL = y - range;
      if ( yL < 0 )
        yL = 0;
      int xH = x + range;
      int yH = y + range;
      _ForEach( iter_chr<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, xL, yL, xH, yH );
    }
    // iterate over all players around chr in visual range
    template <typename F>
    void ForEachPlayerInVisualRange( const UObject* chr, F &&f )
    {
      ForEachPlayerInRange( chr->toplevel_owner()->x, chr->toplevel_owner()->y, chr->toplevel_owner()->realm, RANGE_VISUAL, f );
    }
    // iterate over all players in given box
    template <typename F>
    void ForEachPlayerInBox( u16 x1, u16 y1, u16 x2, u16 y2, const Plib::Realm* realm, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x1, y1, realm, wxL, wyL );
      zone_convert_clip( x2, y2, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      _ForEach( iter_chr<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, x1, y1, x2, y2 );
    }

    // iterate over all npcs at given loc with given range
    template <typename F>
    void ForEachNPCInRange( u16 x, u16 y, const Plib::Realm* realm, unsigned range, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x - range, y - range, realm, wxL, wyL );
      zone_convert_clip( x + range, y + range, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      int xL = x - range;
      if ( xL < 0 )
        xL = 0;
      int yL = y - range;
      if ( yL < 0 )
        yL = 0;
      int xH = x + range;
      int yH = y + range;
      _ForEach( iter_npc<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, xL, yL, xH, yH );
    }
    // iterate over all npcs around chr in visual range
    template <typename F>
    void ForEachNPCInVisualRange( const Mobile::Character* chr, F &&f )
    {
      ForEachNPCInRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, f );
    }
    // iterate over all npcs in given box
    template <typename F>
    void ForEachNPCInBox( u16 x1, u16 y1, u16 x2, u16 y2, const Plib::Realm* realm, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x1, y1, realm, wxL, wyL );
      zone_convert_clip( x2, y2, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      _ForEach( iter_npc<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, x1, y1, x2, y2 );
    }
    // iterate over all items at given loc with given range
    template <typename F>
    void ForEachItemInRange( u16 x, u16 y, const Plib::Realm* realm, unsigned range, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x - range, y - range, realm, wxL, wyL );
      zone_convert_clip( x + range, y + range, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      int xL = x - range;
      if ( xL < 0 )
        xL = 0;
      int yL = y - range;
      if ( yL < 0 )
        yL = 0;
      int xH = x + range;
      int yH = y + range;
      _ForEach( iter_item<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, xL, yL, xH, yH );
    }
    // iterate over all items around chr in visual range
    template <typename F>
    void ForEachItemInVisualRange( const Mobile::Character* chr, F &&f )
    {
      ForEachItemInRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, f );
    }
    // iterate over all items in given box
    template <typename F>
    void ForEachItemInBox( u16 x1, u16 y1, u16 x2, u16 y2, const Plib::Realm* realm, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x1, y1, realm, wxL, wyL );
      zone_convert_clip( x2, y2, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      _ForEach( iter_item<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, x1, y1, x2, y2 );
    }

    // iterate over all multis at given loc with given range
    template <typename F>
    void ForEachMultiInRange( u16 x, u16 y, const Plib::Realm* realm, unsigned range, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x - range, y - range, realm, wxL, wyL );
      zone_convert_clip( x + range, y + range, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      int xL = x - range;
      if ( xL < 0 )
        xL = 0;
      int yL = y - range;
      if ( yL < 0 )
        yL = 0;
      int xH = x + range;
      int yH = y + range;
      _ForEach( iter_multi<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, xL, yL, xH, yH );
    }
    // iterate over all multis around chr in visual range
    template <typename F>
    void ForEachMultiInVisualRange( const Mobile::Character* chr, F &&f )
    {
      ForEachMultiInRange( chr->x, chr->y, chr->realm, RANGE_VISUAL, f );
    }
    // iterate over all multis in given box
    template <typename F>
    void ForEachMultiInBox( u16 x1, u16 y1, u16 x2, u16 y2, const Plib::Realm* realm, F &&f )
    {
      if ( realm == nullptr )
        return;
      u16 wxL, wyL, wxH, wyH;
      zone_convert_clip( x1, y1, realm, wxL, wyL );
      zone_convert_clip( x2, y2, realm, wxH, wyH );
      passert( wxL <= wxH );
      passert( wyL <= wyH );
      _ForEach( iter_multi<decltype( f )>, wxL, wyL, wxH, wyH, realm, f, x1, y1, x2, y2 );
    }
  }
}
#endif
