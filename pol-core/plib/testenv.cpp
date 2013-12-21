/*
History
=======
2009/09/03 MuadDib:   Relocation of multi related cpp/h

Notes
=======

*/

#include "../clib/stl_inc.h"

#include <assert.h>

#include "../clib/cfgfile.h"
#include "../clib/cfgelem.h"
#include "../clib/passert.h"
#include "../plib/realm.h"

#include "../pol/item/item.h"
#include "../pol/item/itemdesc.h"
#include "../pol/multi/multi.h"
#include "../pol/npc.h"
#include "../pol/objecthash.h"
#include "../pol/realms.h"
#include "../pol/ufunc.h"
#include "../pol/uvars.h"
#include "../pol/uworld.h"
namespace Pol {
  namespace Core {
    bool FindNpcTemplate( const char *template_name,
                          Clib::ConfigFile& cfile,
                          Clib::ConfigElem& elem );
  }
  namespace Plib {
    using namespace Core;
    using namespace Items;
	NPC* test_banker;
	NPC* test_banker2;
	NPC* test_banker3;

	Item* test_guard_door;
	NPC* test_water_elemental;
	NPC* test_seaserpent;
	Item* test_chest1;
	Item* test_chest2;
	NPC* test_orclord;

	Item* add_item( unsigned int objtype, unsigned short x, unsigned short y, short z )
	{
	  Item* item;
	  item = Item::create( objtype );
	  passert( item );
	  item->x = x;
	  item->y = y;
	  item->z = static_cast<s8>( z );
	  item->realm = find_realm( string( "britannia" ) );
	  add_item_to_world( item );
	  return item;
	}

	void add_multi( unsigned int objtype, unsigned short x, unsigned short y, short z )
	{
	  Multi::UMulti* multi = Multi::UMulti::create( find_itemdesc( objtype ) );
	  passert( multi );
	  multi->x = x;
	  multi->y = y;
	  multi->z = static_cast<s8>( z );
	  multi->realm = find_realm( string( "britannia" ) );
	  add_multi_to_world( multi );
	}
	void add_multi( unsigned int objtype, unsigned short x, unsigned short y, short z, int flags )
	{
	  Realm* realm = find_realm( string( "britannia" ) );
	  Bscript::BObject obj( Multi::UMulti::scripted_create( find_itemdesc( objtype ), x, y, static_cast<s8>( z ), realm, flags ) );
	}

	NPC* add_npc( const char* npctype, unsigned short x, unsigned short y, short z )
	{
	  Clib::ConfigFile cfile;
	  Clib::ConfigElem elem;
	  if ( !Core::FindNpcTemplate( npctype, cfile, elem ) )
	  {
		throw runtime_error( string( "NPC template '" ) + npctype + "' not found" );
	  }

	  auto  npc = new NPC( elem.remove_ushort( "OBJTYPE" ), elem );
	  elem.clear_prop( "Serial" );
	  elem.clear_prop( "X" );
	  elem.clear_prop( "Y" );
	  elem.clear_prop( "Z" );

	  elem.add_prop( "Serial", GetNextSerialNumber() );
	  elem.add_prop( "X", x );
	  elem.add_prop( "Y", y );
	  elem.add_prop( "Z", z );
	  npc->readPropertiesForNewNPC( elem );

	  objecthash.Insert( npc );

	  SetCharacterWorldPosition( npc );
	  return npc;
	}

	void create_test_environment()
	{
	  // a backpack on a lower level, for testing walking around on the level above.
	  add_item( 0xe75, 1347, 1657, 50 );

	  // a section of portcullis
	  add_item( 0x6f6, 1398, 1625, 29 );

	  //  black pearl on a table in Castle Britannia
	  add_item( 0xf7a, 1328, 1644, 78 );

	  // a house way NE of britain
	  add_multi( 0x6064, 2016, 1327, 0 );

	  // a "stone step" outside on the ground.
	  add_item( 0x915, 1309, 1630, 30 );

	  // a dungeon ramp outside on the ground.
	  add_item( 0x11bd, 1311, 1636, 30 );

	  // a stone arch we should be able to walk under
	  add_item( 0x002c, 1311, 1649, 30 );

	  // a secret door we should be able to walk through
	  add_item( 0x00f5, 1316, 1648, 30 );

	  // a weird door we should be able to walk through
	  add_item( 0x022f, 1313, 1654, 30 );

	  // a dungeon arch
	  add_item( 0x0247, 1313, 1657, 30 );

	  // a black pearl
	  add_item( 0x0f7a, 1315, 1658, 30 );

	  // a boat in britain harbor
	  add_multi( 0x6040, 1487, 1768, -5, Multi::CRMULTI_FACING_NORTH );
	  // the call above creates the planks, hold, and tillerman.  hm, how to extend the port gangplank, though...
	  //add_item( 0x3ED5, 1485, 1768, -5 ); // port gangplank, extended
	  //add_item( 0x3eb2, 1489, 1768, -5 ); // starboard gangplank, retracted
	  //add_item( 0xF013, 1487, 1764, -5 ); // hold
	  //add_item( 0x3e4e, 1488, 1772, -5 ); // tillerman

	  // a boat out in deeper water (map water, that is)
	  add_multi( 0x6040, 1497, 1779, -5 );
	  add_item( 0x3ed5, 1495, 1779, -5 ); // its gangplank

	  // a long boat in deep water
	  add_multi( 0x6044, 1495, 1817, -5, Multi::CRMULTI_FACING_WEST );
	  // FIXME: was 4013, so needs turned.


	  test_guard_door = add_item( 0x6e7, 1412, 1621, 30 ); // a door in a guard house at LB's castle

	  test_banker = add_npc( "banker", 1412, 1624, 25 );
	  test_water_elemental = add_npc( "waterelemental", 1482, 1780, -2 );
	  test_seaserpent = add_npc( "seaserpent", 1479, 1763, -5 );

	  // a house in green acres
	  add_multi( 0x6064, 5987, 1998, 0 );

	  // a dungeon teleporter
	  add_item( 0x6200, 1175, 2635, 0 );

	  // two chests at the same location, for LOS testing
	  test_chest1 = add_item( 0xe42, 1479, 1755, -2 );
	  test_chest2 = add_item( 0xe43, 1479, 1755, -2 );
	  test_banker2 = add_npc( "banker", 1479, 1754, -2 );

	  // an orc underneath buc's den
	  test_orclord = add_npc( "orclord", 2648, 2193, -20 );
	  test_banker3 = add_npc( "banker", 2648, 2193, 4 );

	  // couple of black pearls used for LOS checking
	  add_item( 0xf7a, 1618, 3351, 3 );
	  add_item( 0xf7a, 1618, 3340, 4 );
	}
  }
}