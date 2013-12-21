/*
History
=======
2005/06/01 Shinigami: added getmapshapes - to get access to mapshapes
                      added getstatics - to fill a list with statics
2005/06/06 Shinigami: added readmultis derivative - to get a list of statics
2005/09/03 Shinigami: fixed: Realm::walkheight ignored param doors_block
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2009/11/13 Turley:    Rewrote MovementCode
2009/12/03 Turley:    basic gargoyle fly support
Notes
=======
In standingheight checks there is a nasty bug. Items NOT locked down
(unmovable), are NOT being checked correctly to block movement.

*/

#include "../clib/stl_inc.h"

#include "realm.h"
#include "realmdescriptor.h"
#include "mapcell.h"
#include "mapserver.h"
#include "staticserver.h"
#include "mapshape.h"
#include "maptileserver.h"

#include "../pol/tiles.h"
#include "../pol/mobile/charactr.h"
#include "../pol/network/cgdata.h"
#include "../pol/multi/customhouses.h"
#include "../pol/fnsearch.h"
#include "../pol/ssopt.h"
#include "../pol/uvars.h"
#include "../pol/uworld.h"
#include "../pol/item/item.h"
#include "../pol/item/itemdesc.h"
#include "../pol/item/itemdesc.h"
#include "../pol/multi/multi.h"
#include "../pol/multi/house.h"
#include "../pol/uconst.h"

#define HULL_HEIGHT_BUFFER 2
namespace Pol {
  namespace Core {
	Items::Item* find_walkon_item( Core::ItemsVector& ivec, short z );
	unsigned char flags_from_tileflags( unsigned int uoflags );
  }
  namespace Multi {
	class UMulti;
	UMulti* find_supporting_multi( Plib::MultiList& mvec, short z );
  }

  namespace Plib {
	bool Realm::lowest_standheight( unsigned short x, unsigned short y, short* z ) const
	{
	  static MapShapeList vec;
	  vec.clear();
	  getmapshapes( vec, x, y, FLAG::MOVELAND | FLAG::MOVESEA | FLAG::BLOCKING | FLAG::GRADUAL );

	  bool res = true;
	  lowest_standheight( Core::MOVEMODE_LAND, vec, *z, &res, z );
	  return res;
	}

	void Realm::standheight( Core::MOVEMODE movemode,
							 MapShapeList& shapes,
							 short oldz,
							 bool* result_out, short * newz_out, short* gradual_boost )
	{
	  static vector<const MapShape*> possible_shapes;
	  possible_shapes.clear();
	  bool land_ok = ( movemode & Core::MOVEMODE_LAND ) ? true : false;
	  bool sea_ok = ( movemode & Core::MOVEMODE_SEA ) ? true : false;
	  bool fly_ok = ( movemode & Core::MOVEMODE_FLY ) ? true : false;
	  short the_boost = 0;
	  short new_boost = 0;

	  if ( gradual_boost != NULL )
		the_boost = *gradual_boost;
	  if ( the_boost < 5 )
		the_boost = 5;

	  short newz = -200;

	  // first check only possible walkon shapes and build a list
	  for ( MapShapeList::const_iterator itr = shapes.begin(), itrend = shapes.end(); itr != itrend; ++itr )
	  {
		const MapShape& shape = ( *itr );
		unsigned int flags = shape.flags;
		short ztop = shape.z + shape.height;
#if ENABLE_POLTEST_OUTPUT
		if (static_debug_on)
		{
		  cout << "static: graphic=0x" << std::hex << srec.graphic << std::dec 
			<< ", z=" << int(srec.z) 
			<< ", ht=" << int(srec.height) << endl;
		}
#endif

		if ( ( land_ok && ( flags&FLAG::MOVELAND ) ) ||
			 ( sea_ok && ( flags&FLAG::MOVESEA ) ) ||
			 ( fly_ok && ( flags&FLAG::OVERFLIGHT ) ) )
		{
		  if ( ( ( ztop <= oldz + 2 + the_boost ) || ( ( flags&FLAG::GRADUAL ) && ( ztop <= oldz + 15 ) )   // not too high to step onto
			|| ( fly_ok && ( flags&FLAG::OVERFLIGHT ) && ( ztop <= oldz + 20 ) ) ) && // seems that flying allows higher steps
			( ztop >= newz ) )     // but above or same as the highest yet seen
		  {
#if ENABLE_POLTEST_OUTPUT
			if (static_debug_on) cout << "Setting Z to " << int(ztemp) << endl;
#endif
			possible_shapes.push_back( &shape );
			newz = ztop;
		  }
		}
		if ( newz < shape.z &&
			 shape.z < newz + PLAYER_CHARACTER_HEIGHT )  // space too small to stand?
		{
		  if ( !possible_shapes.empty() )
			possible_shapes.pop_back(); // remove the last pos_shape
		}
	  }

	  bool ret_result = false;
	  short ret_newz = -200;
	  if ( !possible_shapes.empty() )
	  {
		// loop through all possible shapes and test if other shape blocks
		for ( vector<const MapShape*>::const_iterator pos_itr = possible_shapes.begin(), pos_itr_end = possible_shapes.end(); pos_itr != pos_itr_end; ++pos_itr )
		{
		  bool result = true;
		  const MapShape* pos_shape = ( *pos_itr );
		  newz = pos_shape->z + pos_shape->height;
		  for ( MapShapeList::const_iterator itr = shapes.begin(), itrend = shapes.end(); itr != itrend; ++itr )
		  {
			const MapShape& shape = ( *itr );
			if ( ( shape.flags & ( FLAG::MOVELAND | FLAG::MOVESEA | FLAG::BLOCKING ) ) == 0 )
			  continue;
			int shape_top = shape.z + shape.height;

			if ( ( newz < shape.z &&
			  //        Must be left lower than 15 height like
			  //        other checks, it will block char from walking down
			  //        ladders in homes/dungeons if over 9.
			  shape.z <= oldz + 9 /*PLAYER_CHARACTER_HEIGHT*/ )
			  ||
			  ( shape.z < newz + PLAYER_CHARACTER_HEIGHT && shape_top > newz ) )
			{
#if ENABLE_POLTEST_OUTPUT
			  if (static_debug_on)
			  {
				cout << "static: objtype=0x" << std::hex << srec.graphic << std::dec 
				  << ", z=" << int(srec.z) 
				  << ", ht=" << int(srec.height) 
				  << " blocks movement to z=" << int(newz) << endl;
			  }
#endif

			  result = false;
			  break;
			}

		  }
		  if ( result )
		  {
			//if something was already found use the smallest step diff
			if ( ( ret_result ) && ( labs( oldz - ret_newz ) < labs( oldz - newz ) ) )
			  continue;
			ret_result = true;
			ret_newz = newz;
			if ( pos_shape->flags & FLAG::GRADUAL )
			  new_boost = pos_shape->height;
		  }
		}
	  }

	  *result_out = ret_result;
	  *newz_out = ret_newz;
	  if ( ret_result && ( gradual_boost != NULL ) )
	  {
		if ( new_boost > 11 )
		  *gradual_boost = 11;
		else
		  *gradual_boost = new_boost;
	  }
	}


	void Realm::lowest_standheight( Core::MOVEMODE movemode,
									MapShapeList& shapes,
									short minz,
									bool* result_out, short* newz_out, short* gradual_boost )
	{
	  bool land_ok = ( movemode & Core::MOVEMODE_LAND ) ? true : false;
	  bool sea_ok = ( movemode & Core::MOVEMODE_SEA ) ? true : false;
	  bool fly_ok = ( movemode & Core::MOVEMODE_FLY ) ? true : false;
	  short the_boost = 0;
	  short new_boost = 0;

	  if ( gradual_boost != NULL )
		the_boost = *gradual_boost;

	  if ( shapes.size() == 1 ) //map only
	  {
		const MapShape& shape = shapes.at( 0 );
		if ( land_ok && ( shape.flags & ( FLAG::MOVELAND ) ) )
		{
		  *result_out = true;
		  *newz_out = shape.z + shape.height;
		  return;
		}
	  }


	  short newz = 255;
	  bool result = false;

	  for ( MapShapeList::const_iterator itr = shapes.begin(), itrend = shapes.end(); itr != itrend; ++itr )
	  {
		const MapShape& shape = ( *itr );
		unsigned int flags = shape.flags;
		short ztop = shape.z + shape.height;
#if ENABLE_POLTEST_OUTPUT
		if (static_debug_on)
		{
		  cout << "static: graphic=0x" << std::hex << srec.graphic << std::dec 
			<< ", z=" << int(srec.z) 
			<< ", ht=" << int(srec.height) << endl;
		}
#endif

		if ( the_boost < 5 )
		  the_boost = 5;

		if ( ( land_ok && ( flags&FLAG::MOVELAND ) ) ||
			 ( sea_ok && ( flags&FLAG::MOVESEA ) ) ||
			 ( fly_ok && ( flags&FLAG::OVERFLIGHT ) ) )
		{
		  if ( ( ztop >= minz ) &&	// higher than our base
			   ( ztop < newz ) )		// lower than we've seen before
		  {
#if ENABLE_POLTEST_OUTPUT
			if (static_debug_on) cout << "Setting Z to " << int(ztemp) << endl;
#endif
			bool valid = true;
			// validate that its actually standable
			for ( MapShapeList::const_iterator itrCheck = shapes.begin(), itrCheckend = shapes.end(); itrCheck != itrCheckend; ++itrCheck )
			{
			  const MapShape& shapeCheck = ( *itrCheck );
			  if ( ( shapeCheck.flags & ( FLAG::MOVELAND | FLAG::MOVESEA | FLAG::BLOCKING ) ) == 0 )
				continue;

			  if ( ( ( ztop < shapeCheck.z ) &&		// if the check-shape is higher than the location
				( shapeCheck.z < ztop + 9 ) )	// but low enough to block standing (9 instead of 15 for ladders)
				||								// OR
				( ( shapeCheck.z < ztop + PLAYER_CHARACTER_HEIGHT ) &&		// if the check-shape is sitting below the standing space of my new location
				( shapeCheck.z + shapeCheck.height > ztop ) ) )			// and the top of the object is above my new location

			  {
#if ENABLE_POLTEST_OUTPUT
				if (static_debug_on)
				{
				  cout << "static: objtype=0x" << std::hex << srec.graphic << std::dec 
					<< ", z=" << int(srec.z) 
					<< ", ht=" << int(srec.height) 
					<< " blocks movement to z=" << int(newz) << endl;
				}
#endif
				valid = false;
				break;
			  }
			}

			// its the lowest and its legal
			if ( valid )
			{
			  newz = ztop;
			  result = true;
			  if ( flags&FLAG::GRADUAL )
				new_boost = shape.height;
			}
		  }
		}
	  }

	  *result_out = result;
	  *newz_out = newz;
	  if ( result && ( gradual_boost != NULL ) )
	  {
		if ( new_boost > 11 )
		  *gradual_boost = 11;
		else
		  *gradual_boost = new_boost;
	  }
	}

	

	void Realm::readdynamics( MapShapeList& vec, unsigned short x, unsigned short y, Core::ItemsVector& walkon_items, bool doors_block )
	{
	  Core::ZoneItems& witems = Core::getzone( x, y, this ).items;
	  for ( Core::ZoneItems::const_iterator itr = witems.begin( ), itrend = witems.end( ); itr != itrend; ++itr )
	  {
		Items::Item *item = *itr;
		if ( ( item->x == x ) && ( item->y == y ) )
		{
		  if ( Core::tile_flags( item->graphic ) & FLAG::WALKBLOCK )
		  {
			if ( doors_block || item->itemdesc().type != Items::ItemDesc::DOORDESC )
			{
			  MapShape shape;
			  shape.z = item->z;
			  shape.height = item->height;
			  shape.flags = Core::tile[item->graphic].flags;
			  vec.push_back( shape );
			}
		  }

		  if ( !item->itemdesc().walk_on_script.empty() )
		  {
			walkon_items.push_back( item );
		  }
		}
	  }
	}
	

	// new Z given new X, Y, and old Z.
	bool Realm::walkheight( unsigned short x, unsigned short y, short oldz,
							short * newz,
							Multi::UMulti** pmulti, Items::Item** pwalkon,
							bool doors_block,
							Core::MOVEMODE movemode,
							short* gradual_boost )
	{
	  if ( x >= _Descriptor().width ||
		   y >= _Descriptor().height )
	  {
		return false;
	  }

	  static MapShapeList shapes;
	  static MultiList mvec;
	  static Core::ItemsVector walkon_items;
	  shapes.clear();
	  mvec.clear();
	  walkon_items.clear();

	  readdynamics( shapes, x, y, walkon_items, doors_block /* true */ );
	  unsigned int flags = FLAG::MOVE_FLAGS;
	  if ( movemode & Core::MOVEMODE_FLY )
		flags |= FLAG::OVERFLIGHT;
	  readmultis( shapes, x, y, flags, mvec );
	  getmapshapes( shapes, x, y, flags );

	  bool result;
	  standheight( movemode, shapes, oldz,
				   &result, newz, gradual_boost );

	  if ( result && ( pwalkon != NULL ) )
	  {
		if ( !mvec.empty() )
		{
		  *pmulti = find_supporting_multi( mvec, *newz );
		}
		else
		{
		  *pmulti = NULL;
		}

		if ( !walkon_items.empty() )
		{
		  *pwalkon = Core::find_walkon_item( walkon_items, *newz );
		}
		else
		{
		  *pwalkon = NULL;
		}

	  }

	  return result;
	}

	// new Z given new X, Y, and old Z.
	//dave: todo: return false if walking onto a custom house and not in the list of editing players, and no cmdlevel
	bool Realm::walkheight( const Mobile::Character* chr, unsigned short x, unsigned short y, short oldz,
							short* newz,
							Multi::UMulti** pmulti, Items::Item** pwalkon, short* gradual_boost )
	{

	  if ( x >= _Descriptor().width || y >= _Descriptor().height )
	  {
		return false;
	  }

	  static MapShapeList shapes;
	  static MultiList mvec;
	  static Core::ItemsVector walkon_items;
	  shapes.clear();
	  mvec.clear();
	  walkon_items.clear();

	  readdynamics( shapes, x, y, walkon_items, chr->doors_block() );
	  unsigned int flags = FLAG::MOVE_FLAGS;
	  if ( chr->movemode & Core::MOVEMODE_FLY )
		flags |= FLAG::OVERFLIGHT;
	  readmultis( shapes, x, y, flags, mvec );
	  getmapshapes( shapes, x, y, flags );

	  bool result;
	  standheight( chr->movemode, shapes, oldz,
				   &result, newz, gradual_boost );

	  if ( result && ( pwalkon != NULL ) )
	  {
		if ( !mvec.empty() )
		{
		  *pmulti = find_supporting_multi( mvec, *newz );
		  if ( *pmulti != NULL )
		  {
			Multi::UHouse* house = ( *pmulti )->as_house();
			if ( house && house->editing && chr->client && ( chr->client->gd->custom_house_serial != house->serial ) )
			  result = false; //sendsysmessage to client ?
		  }
		}
		else
		{
		  *pmulti = NULL;
		  if ( chr->is_house_editing() )
		  {
			Multi::UMulti* multi = Core::system_find_multi( chr->client->gd->custom_house_serial );
			if ( multi != NULL )
			{
			  Multi::UHouse* house = multi->as_house();
			  if ( house != NULL )
			  {
				Multi::CustomHouseStopEditing( const_cast<Mobile::Character*>( chr ), house );
				Multi::CustomHousesSendFull( house, chr->client, Multi::HOUSE_DESIGN_CURRENT );
			  }
			}
		  }
		}

		if ( !walkon_items.empty() )
		{
		  *pwalkon = find_walkon_item( walkon_items, *newz );
		}
		else
		{
		  *pwalkon = NULL;
		}

	  }

	  return result;
	}


	bool Realm::lowest_walkheight( unsigned short x, unsigned short y, short oldz,
								   short* newz,
								   Multi::UMulti** pmulti, Items::Item** pwalkon,
								   bool doors_block,
								   Core::MOVEMODE movemode,
								   short* gradual_boost )
	{

	  if ( x >= _Descriptor().width ||
		   y >= _Descriptor().height )
	  {
		return false;
	  }

	  static MapShapeList shapes;
	  static MultiList mvec;
	  static Core::ItemsVector walkon_items;
	  shapes.clear();
	  mvec.clear();
	  walkon_items.clear();

	  readdynamics( shapes, x, y, walkon_items, doors_block /* true */ );
	  unsigned int flags = FLAG::MOVE_FLAGS;
	  if ( movemode & Core::MOVEMODE_FLY )
		flags |= FLAG::OVERFLIGHT;
	  readmultis( shapes, x, y, flags, mvec );
	  getmapshapes( shapes, x, y, flags );

	  bool result;
	  lowest_standheight( movemode, shapes, oldz, &result, newz, gradual_boost );

	  if ( result )
	  {
		if ( !mvec.empty() )
		{
		  *pmulti = find_supporting_multi( mvec, *newz );
		}
		else
		{
		  *pmulti = NULL;
		}

		if ( !walkon_items.empty() )
		{
		  *pwalkon = find_walkon_item( walkon_items, *newz );
		}
		else
		{
		  *pwalkon = NULL;
		}

	  }

	  return result;
	}


	bool Realm::dropheight( unsigned short dropx,
							unsigned short dropy,
							short dropz,
							short chrz,
							short* newz,
							Multi::UMulti** pmulti )
	{

	  if ( dropx >= _Descriptor().width || dropy >= _Descriptor().height )
	  {
		return false;
	  }

	  static MapShapeList shapes;
	  static MultiList mvec;
	  static Core::ItemsVector ivec;
	  shapes.clear();
	  mvec.clear();
	  ivec.clear();

	  readdynamics( shapes, dropx, dropy, ivec, true /* doors_block */ );
	  readmultis( shapes, dropx, dropy, FLAG::DROP_FLAGS, mvec );
	  getmapshapes( shapes, dropx, dropy, FLAG::DROP_FLAGS );

	  bool result = dropheight( shapes, dropz, chrz, newz );
	  if ( result )
	  {
		if ( !mvec.empty() )
		{
		  *pmulti = find_supporting_multi( mvec, *newz );
		}
		else
		{
		  *pmulti = NULL;
		}
	  }
	  return result;
	}

	// used to be statics_dropheight
	bool Realm::dropheight( MapShapeList& shapes,
							short dropz,
							short chrz,
							short* newz )
	{
	  short z = -128;
	  bool result = false;

	  for ( MapShapeList::const_iterator itr = shapes.begin(), itrend = shapes.end(); itr != itrend; ++itr )
	  {
		const MapShape& shape = ( *itr );
		unsigned int flags = shape.flags;
		short ztop = shape.z + shape.height;
#if ENABLE_POLTEST_OUTPUT
		if (static_debug_on)
		{
		  cout << "static: graphic=0x" << std::hex << srec.graphic << std::dec 
			<< ", z=" << int(srec.z) 
			<< ", ht=" << int(tileheight(srec.graphic)) << endl;
		}
#endif

		if ( flags & FLAG::ALLOWDROPON )
		{
		  if ( ( ztop <= chrz + PLAYER_CHARACTER_HEIGHT ) &&   // not too high to reach
			   ( ztop >= z ) )          // but above or same as the highest yet seen
		  {                           // NOTE, the >= here is so statics at ground height
			// will override a blocking map tile.
#if ENABLE_POLTEST_OUTPUT
			if (static_debug_on) cout << "Setting Z to " << int(ztemp) << endl;
#endif
			z = ztop;
			result = true;
		  }
		}
	  }

	  *newz = z;

	  if ( result )
	  {
		for ( MapShapeList::const_iterator itr = shapes.begin(), itrend = shapes.end(); itr != itrend; ++itr )
		{
		  const MapShape& shape = ( *itr );
		  if ( shape.flags & FLAG::BLOCKING )
		  {
			short ztop = shape.z + shape.height;

			if ( shape.z <= z &&        // location blocked by some object
				 z < ztop )
			{
#if ENABLE_POLTEST_OUTPUT
			  if (static_debug_on)
			  {
				cout << "static: objtype=0x" << std::hex << srec.graphic << std::dec 
				  << ", z=" << int(srec.z) 
				  << ", ht=" << int(tileheight(srec.graphic)) 
				  << " blocks movement to z=" << int(z) << endl;
			  }
#endif
			  result = false;
			  break;
			}
			else if ( z < ztop &&
					  ztop <= dropz )
			{
			  // a blocking item somewhere between where they _tried_ to drop it,
			  // and where we decided to place it.
			  result = false;
			  break;
			}
		  }
		}
	  }

	  return result;

	}

	void Realm::readmultis( MapShapeList& vec, unsigned short x, unsigned short y, unsigned int anyflags ) const
	{
	  unsigned short wxL, wyL, wxH, wyH;
	  Core::zone_convert_clip( x - 64, y - 64, this, wxL, wyL );
      Core::zone_convert_clip( x + 64, y + 64, this, wxH, wyH );

	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          Core::ZoneMultis& wmultis = zone[wx][wy].multis;

		  for ( auto multi : wmultis )
		  {
			Multi::UHouse* house = multi->as_house();
			if ( house != NULL && house->IsCustom() ) //readshapes switches to working design if the house is being edited, 
			  //everyone in the house would use it for walking...
			  multi->readshapes( vec, s16( x ) - multi->x, s16( y ) - multi->y, multi->z );
			else
			{
			  const Multi::MultiDef& def = multi->multidef();
			  def.readshapes( vec, s16( x ) - multi->x, s16( y ) - multi->y, multi->z, anyflags );
			}
		  }
		}
	  }
	}

	void Realm::readmultis( MapShapeList& vec, unsigned short x, unsigned short y, unsigned int anyflags, MultiList& mvec )
	{
	  unsigned short wxL, wyL, wxH, wyH;
      Core::zone_convert_clip( x - 64, y - 64, this, wxL, wyL );
      Core::zone_convert_clip( x + 64, y + 64, this, wxH, wyH );

	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          Core::ZoneMultis& wmultis = zone[wx][wy].multis;

		  for ( auto multi : wmultis )
		  {
			Multi::UHouse* house = multi->as_house();
			if ( house != NULL && house->IsCustom() )
			{
			  if ( multi->readshapes( vec, s16( x ) - multi->x, s16( y ) - multi->y, multi->z ) )
				mvec.push_back( multi );
			}
			else
			{
			  const Multi::MultiDef& def = multi->multidef();
			  if ( def.readshapes( vec, s16( x ) - multi->x, s16( y ) - multi->y, multi->z, anyflags ) )
			  {
				mvec.push_back( multi );
			  }
			}
		  }
		}
	  }
	}

	void Realm::readmultis( Core::StaticList& vec, unsigned short x, unsigned short y ) const
	{
	  unsigned short wxL, wyL, wxH, wyH;
      Core::zone_convert_clip( x - 64, y - 64, this, wxL, wyL );
      Core::zone_convert_clip( x + 64, y + 64, this, wxH, wyH );

	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          Core::ZoneMultis& wmultis = zone[wx][wy].multis;

		  for ( auto multi : wmultis )
		  {
			Multi::UHouse* house = multi->as_house();
			if ( house != NULL && house->IsCustom() ) //readshapes switches to working design if the house is being edited, 
			  //everyone in the house would use it for walking...
			  multi->readobjects( vec, int( x ) - multi->x, int( y ) - multi->y, multi->z );
			else
			{
			  const Multi::MultiDef& def = multi->multidef();
			  def.readobjects( vec, int( x ) - multi->x, int( y ) - multi->y, multi->z );
			}
		  }
		}
	  }
	}

	bool Realm::navigable( unsigned short x, unsigned short y, short z, short height = 0 ) const
	{

	  if ( !valid( x, y, z ) )
	  {
		return false;
	  }

	  bool onwater = false;

	  static MapShapeList shapes;
	  shapes.clear();

	  // possible: readdynamic, readmultis
	  getmapshapes( shapes, x, y, FLAG::ALL );

	  for ( MapShapeList::const_iterator itr = shapes.begin(), itrend = shapes.end(); itr != itrend; ++itr )
	  {
		const MapShape& shape = ( *itr );
		if ( shape.flags & FLAG::MOVESEA )
		{
		  onwater = true;
		}
		else
		{
		  if ( ( ( z + height ) >= ( shape.z - HULL_HEIGHT_BUFFER ) ) &&
			   ( z < ( shape.z + shape.height + HULL_HEIGHT_BUFFER ) )
			   )
			   return false;
		}
	  }

	  return onwater;
	}

	Multi::UMulti* Realm::find_supporting_multi( unsigned short x, unsigned short y, short z )
	{
	  if ( !valid( x, y, z ) )
	  {
		return NULL;
	  }

	  static MapShapeList vec;
	  static MultiList mvec;
	  vec.clear();
	  mvec.clear();
	  readmultis( vec, x, y, FLAG::MOVE_FLAGS, mvec );

	  return find_supporting_multi( mvec, z );
	}

	/* The supporting multi is the highest multi that is below or equal
	 * to the Z-coord of the supported object.
	 */
	Multi::UMulti* Realm::find_supporting_multi( MultiList& mvec, short z )
	{
	  Multi::UMulti* found = NULL;
	  for ( MultiList::const_iterator itr = mvec.begin(), end = mvec.end(); itr != end; ++itr )
	  {
		Multi::UMulti* multi = ( *itr );
		if ( multi->z <= z )
		{
		  if ( ( found == NULL ) ||
			   ( multi->z > found->z ) )
		  {
			found = multi;
		  }
		}
	  }
	  return found;
	}

	bool Realm::findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const
	{
	  if ( is_shadowrealm )
		return baserealm->_staticserver->findstatic( x, y, objtype );
	  else
		return _staticserver->findstatic( x, y, objtype );
	}

	void Realm::getstatics( StaticEntryList& statics, unsigned short x, unsigned short y ) const
	{
	  if ( is_shadowrealm )
		return baserealm->_staticserver->getstatics( statics, x, y );
	  else
		_staticserver->getstatics( statics, x, y );
	}

	bool Realm::groundheight( unsigned short x, unsigned short y, short* z ) const
	{
	  MAPTILE_CELL cell = _maptileserver->GetMapTile( x, y );
	  *z = cell.z;

	  if ( cell.landtile == GRAPHIC_NODRAW ) // it's a nodraw tile
		*z = -128;

	  return ( ( cell.landtile < 0x4000 ) &&
			   ( ( Core::landtile_flags( cell.landtile ) & FLAG::BLOCKING ) == 0 ) );
	}

	MAPTILE_CELL Realm::getmaptile( unsigned short x, unsigned short y ) const
	{
	  if ( is_shadowrealm )
		return baserealm->_maptileserver->GetMapTile( x, y );
	  else
		return _maptileserver->GetMapTile( x, y );
	}

	void Realm::getmapshapes( MapShapeList& shapes, unsigned short x, unsigned short y, unsigned int anyflags ) const
	{
	  if ( is_shadowrealm )
		baserealm->_mapserver->GetMapShapes( shapes, x, y, anyflags );
	  else
		_mapserver->GetMapShapes( shapes, x, y, anyflags );
	}
  }
}
