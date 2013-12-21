/*
History
=======
2005/06/06 Shinigami: added readobjects - to get a list of statics
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2009/09/03 MuadDib:   Relocation of multi related cpp/h
2009/09/14 MuadDib:   Squatters code added to register.unregister mobs.
2009/09/15 MuadDib:   Better cleanup handling on house destroy. Alos clears registered_house off character.
                      Houses now only allow mobiles to be registered. May add items later for other storage.
2012/02/02 Tomi:      Added boat member MBR_MULTIID

Notes
=======

*/


#include "../../clib/stl_inc.h"

#include <assert.h>

#include "../../bscript/berror.h"
#include "../../bscript/executor.h"
#include "../../bscript/objmembers.h"
#include "../../bscript/objmethods.h"

#include "../../clib/cfgelem.h"
#include "../../clib/endian.h"
#include "../../clib/logfile.h"
#include "../../clib/passert.h"
#include "../../clib/stlutil.h"
#include "../../clib/strutil.h"

#include "../../plib/mapcell.h"
#include "../../plib/realm.h"
#include "../../plib/mapshape.h"

#include "../network/cgdata.h"
#include "../core.h"
#include "../fnsearch.h"
#include "../item/itemdesc.h"
#include "multidef.h"
#include "../objtype.h"
#include "../polcfg.h"
#include "../realms.h"
#include "../scrsched.h"
#include "../scrstore.h"
#include "../tiles.h"
#include "../ufunc.h"
#include "../uofile.h"
#include "../uoexec.h"
#include "../module/uomod.h"
#include "../uoscrobj.h"
#include "../ustruct.h"
#include "../uvars.h"
#include "../uworld.h"

#include "house.h"

#include "../objecthash.h"

namespace Pol {
  namespace Multi {
	void UHouse::list_contents( const UHouse* house,
								ItemList& items_in,
								MobileList& chrs_in )
	{
	  const MultiDef& md = house->multidef();
	  unsigned short wxL, wyL, wxH, wyH;
	  short x1 = house->x + md.minrx, y1 = house->y + md.minry;
	  short x2 = house->x + md.maxrx, y2 = house->y + md.maxry;
	  Core::zone_convert_clip( x1, y1, house->realm, wxL, wyL );
      Core::zone_convert_clip( x2, y2, house->realm, wxH, wyH );
	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          Core::ZoneCharacters& wchr = house->realm->zone[wx][wy].characters;
          for ( Core::ZoneCharacters::iterator itr = wchr.begin( ), end = wchr.end( ); itr != end; ++itr )
		  {
			Mobile::Character* chr = *itr;
			if ( chr->x >= x1 && chr->x <= x2 &&
				 chr->y >= y1 && chr->y <= y2 )
			{
			  Items::Item* walkon;
			  UMulti* multi;
			  short newz;
			  if ( house->realm->walkheight( chr, chr->x, chr->y, chr->z, &newz, &multi, &walkon ) )
			  {
				if ( const_cast<const UMulti*>( multi ) == house )
				  chrs_in.push_back( chr );
			  }
			}
		  }

          Core::ZoneItems& witem = house->realm->zone[wx][wy].items;
          for ( Core::ZoneItems::iterator itr = witem.begin( ), end = witem.end( ); itr != end; ++itr )
		  {
			Items::Item* item = *itr;
			if ( item->x >= x1 && item->x <= x2 &&
				 item->y >= y1 && item->y <= y2 )
			{
			  Items::Item* walkon;
			  UMulti* multi;
			  short newz;
			  unsigned short sx = item->x;
			  unsigned short sy = item->y;
			  item->x = 0;    // move 'self' a bit so it doesn't interfere with itself
			  item->y = 0;
			  bool res = house->realm->walkheight( sx, sy, item->z, &newz, &multi, &walkon, true, Core::MOVEMODE_LAND );
			  item->x = sx;
			  item->y = sy;
			  if ( res )
			  {
				if ( const_cast<const UMulti*>( multi ) == house )
				{
                  if ( Core::tile_flags( item->graphic ) & Plib::FLAG::WALKBLOCK )
					items_in.push_front( item );
				  else
					items_in.push_back( item );
				}
			  }
			}
		  }
		}
	  }
	}

	UHouse::UHouse( const Items::ItemDesc& itemdesc ) : UMulti( itemdesc ),
	  editing( false ),
	  waiting_for_accept( false ),
	  editing_floor_num( 1 ),
	  revision( 0 ),
	  custom( false )
	{}

	void UHouse::create_components()
	{
	  const MultiDef& md = multidef();
	  for ( unsigned i = 0; i < md.elems.size(); ++i )
	  {
		const MULTI_ELEM& elem = md.elems[i];
		if ( !elem.is_static )
		{
		  Items::Item* item = Items::Item::create( elem.objtype );
		  item->x = x + elem.x;
		  item->y = y + elem.y;
		  item->z = static_cast<s8>( z + elem.z );
		  item->setprop( "house_serial", "i" + Clib::decint( serial ) );
		  item->disable_decay();
		  item->movable( false );
		  item->realm = realm;
		  update_item_to_inrange( item );
		  add_item_to_world( item );
		  components_.push_back( Component( item ) );
		}
	  }
	}

	void UHouse::add_component( Items::Item* item, s32 xoff, s32 yoff, u8 zoff )
	{
	  item->x = x + xoff;
	  item->y = y + yoff;
	  item->z = static_cast<s8>( z + zoff );
	  item->setprop( "house_serial", "i" + Clib::decint( serial ) );
	  item->disable_decay();
	  item->movable( false );
	  item->realm = realm;
	  update_item_to_inrange( item );
	  add_item_to_world( item );
	  components_.push_back( Component( item ) );
	}

	Bscript::ObjArray* UHouse::component_list() const
	{
      std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
	  for ( Components::const_iterator itr = components_.begin(), end = components_.end(); itr != end; ++itr )
	  {
		Items::Item* item = ( *itr ).get();
		if ( item != NULL && !item->orphan() )
		{
		  arr->addElement( new Module::EItemRefObjImp( item ) );
		}
	  }
	  return arr.release();
	}

    Bscript::ObjArray* UHouse::items_list( ) const
	{
	  ItemList itemlist;
	  MobileList moblist;
	  list_contents( this, itemlist, moblist );
      std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
	  for ( ItemList::iterator itr = itemlist.begin(); itr != itemlist.end(); ++itr )
	  {
		Items::Item* item = ( *itr );

		if ( Clib::const_find_in( components_, Component( item ) ) == components_.end() )
		{
		  arr->addElement( new Module::EItemRefObjImp( item ) );
		}
	  }
	  return arr.release();
	}

    Bscript::ObjArray* UHouse::mobiles_list( ) const
	{
	  ItemList itemlist;
	  MobileList moblist;
	  list_contents( this, itemlist, moblist );
      std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
	  for ( MobileList::iterator itr = moblist.begin(); itr != moblist.end(); ++itr )
	  {
		Mobile::Character* chr = ( *itr );
        arr->addElement( new Module::ECharacterRefObjImp( chr ) );
	  }
	  return arr.release();
	}

	UHouse* UHouse::as_house()
	{
	  return this;
	}

    Bscript::BObjectImp* UHouse::get_script_member_id( const int id ) const ///id test
	{
      using namespace Bscript;
	  BObjectImp* imp = base::get_script_member_id( id );
	  if ( imp )
		return imp;

	  switch ( id )
	  {
		case MBR_COMPONENTS: return component_list(); break;
		case MBR_ITEMS: return items_list(); break;
		case MBR_MOBILES: return mobiles_list(); break;
		case MBR_CUSTOM: return new BLong( IsCustom() ); break;
		case MBR_EDITING: return new BLong( IsEditing() ); break;
		case MBR_MULTIID: return new BLong( multiid ); break;
		case MBR_HOUSEPARTS:
		  if ( !IsCustom() )
			return new BError( "House is not custom" );
		  else if ( IsEditing() )
			return new BError( "House is currently been edited" );
		  else
			return CurrentDesign.list_parts();
		  break;

		default: return NULL;
	  }
	}

    Bscript::BObjectImp* UHouse::get_script_member( const char *membername ) const
	{
      Bscript::ObjMember* objmember = Bscript::getKnownObjMember( membername );
	  if ( objmember != NULL )
		return this->get_script_member_id( objmember->id );
	  else
		return NULL;
	}

    Bscript::BObjectImp* UHouse::script_method_id( const int id, Bscript::Executor& ex )
	{
      using namespace Bscript;
	  BObjectImp* imp = base::script_method_id( id, ex );
	  if ( imp != NULL )
		return imp;

	  switch ( id )
	  {
		case MTH_SETCUSTOM:
		{
							int _custom;
							if ( ex.getParam( 0, _custom ) )
							{
							  SetCustom( _custom ? true : false );
							  return new BLong( 1 );
							}
							else
							  return new BError( "Invalid parameter type" );
		}
		case MTH_ADD_COMPONENT:
		{
								BApplicObjBase* aob;
								if ( ex.hasParams( 0 ) )
                                  aob = ex.getApplicObjParam( 0, &Module::eitemrefobjimp_type );
								else
								  return new BError( "Invalid parameter type" );

								if ( aob != NULL )
								{
                                  Module::EItemRefObjImp* ir = static_cast<Module::EItemRefObjImp*>( aob );
                                  Core::ItemRef iref = ir->value( );
								  components_.push_back( iref );
								  return new BLong( 1 );
								}
		}

		case MTH_ERASE_COMPONENT:
		{
								  BApplicObjBase* aob;
								  if ( ex.hasParams( 0 ) )
                                    aob = ex.getApplicObjParam( 0, &Module::eitemrefobjimp_type );
								  else
									return new BError( "Invalid parameter type" );
								  if ( aob != NULL )
								  {
                                    Module::EItemRefObjImp* ir = static_cast<Module::EItemRefObjImp*>( aob );
									Core::ItemRef iref = ir->value();
									Components::iterator pos;
									pos = find( components_.begin(), components_.end(), iref );
									if ( pos != components_.end() )
									  components_.erase( pos );
									else
									  return new BError( "Component not found" );
									return new BLong( 1 );
								  }
		}
		case MTH_ADD_HOUSE_PART:
		{
								 if ( !IsCustom() )
								   return new BError( "House is not custom" );
								 else if ( IsEditing() )
								   return new BError( "House is currently been edited" );
								 else if ( !ex.hasParams( 4 ) )
								   return new BError( "Not enough parameters" );
								 int graphic, xoff, yoff, z;
								 if ( ex.getParam( 0, graphic ) &&
									  ex.getParam( 1, xoff ) &&
									  ex.getParam( 2, yoff ) &&
									  ex.getParam( 3, z ) )
								 {
								   CUSTOM_HOUSE_ELEMENT elem;
								   elem.graphic = graphic;
								   elem.xoffset = xoff;
								   elem.yoffset = yoff;
								   elem.z = z;
								   CurrentDesign.Add( elem );
								   //invalidate
								   //invalidate
								   WorkingDesign = CurrentDesign;
								   vector<u8> newvec;
								   WorkingCompressed.swap( newvec );
								   vector<u8> newvec2;
								   CurrentCompressed.swap( newvec2 );
								   revision++;
								   CustomHousesSendFullToInRange( this, HOUSE_DESIGN_CURRENT, RANGE_VISUAL_LARGE_BUILDINGS );
								   return new BLong( 1 );
								 }
		}
		case MTH_ERASE_HOUSE_PART:
		{
								   if ( !IsCustom() )
									 return new BError( "House is not custom" );
								   else if ( IsEditing() )
									 return new BError( "House is currently been edited" );
								   else if ( !ex.hasParams( 4 ) )
									 return new BError( "Not enough parameters" );
								   int graphic, xoff, yoff, z;
								   if ( ex.getParam( 0, graphic ) &&
										ex.getParam( 1, xoff ) &&
										ex.getParam( 2, yoff ) &&
										ex.getParam( 3, z ) )
								   {
									 bool ret = CurrentDesign.EraseGraphicAt( static_cast<u16>( graphic ),
																			  static_cast<u32>( xoff ),
																			  static_cast<u32>( yoff ),
																			  static_cast<u8>( z ) );
									 if ( ret )
									 {
									   //invalidate
									   WorkingDesign = CurrentDesign;
									   vector<u8> newvec;
									   WorkingCompressed.swap( newvec );
									   vector<u8> newvec2;
									   CurrentCompressed.swap( newvec2 );
									   CustomHousesSendFullToInRange( this, HOUSE_DESIGN_CURRENT, RANGE_VISUAL_LARGE_BUILDINGS );
									 }
									 return new BLong( ret ? 1 : 0 );
								   }
		}
		case MTH_ACCEPT_COMMIT:
		{
								if ( !IsCustom() )
								  return new BError( "House is not custom" );
								//else if (!IsEditing())
								//	return new BError( "House is currently not been edited" );
								else if ( !IsWaitingForAccept() )
								  return new BError( "House is currently not waiting for a commit" );
								else if ( !ex.hasParams( 2 ) )
								  return new BError( "Not enough parameters" );
								int accept;
								Mobile::Character* chr;
								if ( ex.getParam( 1, accept ) &&
									 getCharacterParam( ex, 0, chr ) )
								{
								  AcceptHouseCommit( chr, accept ? true : false );
								  return new BLong( 1 );
								}
								else
								  return new BError( "Invalid parameter type" );
		}
		case MTH_CANCEL_EDITING:
		{
								 if ( !IsCustom() )
								   return new BError( "House is not custom" );
								 else if ( !IsEditing() )
								   return new BError( "House is currently not been edited" );
								 else if ( !ex.hasParams( 2 ) )
								   return new BError( "Not enough parameters" );
								 Mobile::Character* chr;
								 int drop_changes;
								 if ( getCharacterParam( ex, 0, chr ) &&
									  ex.getParam( 1, drop_changes ) )
								 {
								   if ( chr->client->gd->custom_house_serial == serial )
									 CustomHousesQuit( chr, drop_changes ? true : false );
								   else
									 return new BError( "Character is not editing this house" );
								   return new BLong( 1 );
								 }
								 else
								   return new BError( "Invalid parameter type" );
		}

		default: return NULL;
	  }
	}

    Bscript::BObjectImp* UHouse::script_method( const char* methodname, Bscript::Executor& ex )
	{
      Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
	  if ( objmethod != NULL )
		return this->script_method_id( objmethod->id, ex );
	  else
		return NULL;
	  /*
	  BObjectImp* imp = base::script_method( membername, ex );
	  if (imp != NULL)
	  return imp;

	  if (stricmp( membername, "setcustom" ) == 0)
	  {
	  int _custom;
	  if (ex.getParam( 0, _custom ))
	  {
	  SetCustom( _custom ? true : false );
	  return new BLong(1);
	  }
	  else
	  return new BError( "Invalid parameter type" );
	  }
	  else if(stricmp( membername, "add_component" ) == 0)
	  {
	  BApplicObjBase* aob = NULL;
	  if(ex.hasParams( 0 ))
	  aob = ex.getApplicObjParam(0, &eitemrefobjimp_type);

	  if(aob != NULL)
	  {
	  EItemRefObjImp* ir = static_cast<EItemRefObjImp*>(aob);
	  ItemRef iref = ir->value();
	  components_.push_back(iref);
	  return new BLong(1);
	  }
	  else
	  return new BError( "Invalid parameter type" );

	  }
	  else if(stricmp( membername, "erase_component" ) == 0)
	  {
	  BApplicObjBase* aob = NULL;
	  if(ex.hasParams( 0 ))
	  aob = ex.getApplicObjParam(0, &eitemrefobjimp_type);

	  if(aob != NULL)
	  {
	  EItemRefObjImp* ir = static_cast<EItemRefObjImp*>(aob);
	  ItemRef iref = ir->value();
	  Components::iterator pos;
	  pos = find(components_.begin(), components_.end(), iref);
	  if(pos != components_.end())
	  components_.erase(pos);
	  else
	  return new BError("Component not found");
	  return new BLong(1);
	  }
	  else
	  return new BError( "Invalid parameter type" );
	  }
	  return NULL;
	  */
	}

	void UHouse::readProperties( Clib::ConfigElem& elem )
	{
	  base::readProperties( elem );
	  u32 tmp_serial;
	  multiid = elem.remove_ushort( "MultiID", this->multidef().multiid );

	  while ( elem.remove_prop( "Component", &tmp_serial ) )
	  {
		Items::Item* item = Core::find_toplevel_item( tmp_serial );
		if ( item != NULL )
		{
		  components_.push_back( Component( item ) );
		}
	  }
	  custom = elem.remove_bool( "Custom", false );
	  if ( custom )
	  {
		short ysize, xsize, xbase, ybase;
		const MultiDef& def = multidef();
		ysize = def.maxry - def.minry + 1; //+1 to include offset 0 in -3..3
		xsize = def.maxrx - def.minrx + 1; //+1 to include offset 0 in -3..3
		xbase = (short)abs( def.minrx );
		ybase = (short)abs( def.minry );
		CurrentDesign.InitDesign( ysize + 1, xsize, xbase, ybase ); //+1 for front steps outside multidef footprint
		WorkingDesign.InitDesign( ysize + 1, xsize, xbase, ybase ); //+1 for front steps outside multidef footprint
		BackupDesign.InitDesign( ysize + 1, xsize, xbase, ybase ); //+1 for front steps outside multidef footprint
		CurrentDesign.readProperties( elem, "Current" );
		//CurrentDesign.testprint(cout);
		WorkingDesign.readProperties( elem, "Working" );
		BackupDesign.readProperties( elem, "Backup" );

		elem.remove_prop( "DesignRevision", &revision );
	  }
	}

	void UHouse::printProperties( Clib::StreamWriter& sw ) const
	{
	  base::printProperties( sw );

	  sw() << "\tMultiID\t" << multiid << pf_endl;

	  for ( Components::const_iterator itr = components_.begin(), end = components_.end(); itr != end; ++itr )
	  {
		Items::Item* item = ( *itr ).get();
		if ( item != NULL && !item->orphan() )
		{
		  sw() << "\tComponent\t" << fmt::hex( item->serial ) << pf_endl;
		}
	  }
	  sw() << "\tCustom\t" << custom << pf_endl;
	  if ( custom )
	  {
		CurrentDesign.printProperties( sw, "Current" );
		WorkingDesign.printProperties( sw, "Working" );
		BackupDesign.printProperties( sw, "Backup" );
		sw() << "\tDesignRevision\t" << revision << pf_endl;
	  }
	}

	void UHouse::destroy_components()
	{
	  while ( !components_.empty() )
	  {
		Items::Item* item = components_.back().get();
		if ( Core::config.loglevel >= 5 )
		  Clib::Log( "Destroying component %p, serial=0x%lu\n", item, item->serial );
		if ( !item->orphan() )
		  Core::destroy_item( item );
		if ( Core::config.loglevel >= 5 )
		  Clib::Log( "Component destroyed\n" );
		components_.pop_back();
	  }
	}

	bool UHouse::readshapes( Plib::MapShapeList& vec, short x, short y, short zbase )
	{
	  if ( !custom )
		return false;

	  bool result = false;
	  HouseFloorZColumn* elems;
	  HouseFloorZColumn::iterator itr;
	  CustomHouseDesign* design;
	  design = editing ? &WorkingDesign : &CurrentDesign; //consider having a list of players that should use the working set

	  if ( x + design->xoff < 0 || x + design->xoff >= static_cast<s32>( design->width ) ||
		   y + design->yoff < 0 || y + design->yoff >= static_cast<s32>( design->height ) )
		   return false;
	  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
	  {
		elems = design->Elements[i].GetElementsAt( x, y );
		for ( itr = elems->begin(); itr != elems->end(); ++itr )
		{
		  Plib::MapShape shape;
		  shape.z = itr->z + zbase;
          shape.height = Core::tileheight( itr->graphic );
          shape.flags = Core::tile_flags( itr->graphic );
		  if ( !shape.height )
		  {
			++shape.height;
			--shape.z;
		  }
		  vec.push_back( shape );
		  result = true;
		}
	  }
	  return result;

	}

	bool UHouse::readobjects( Core::StaticList& vec, short x, short y, short zbase )
	{
	  if ( !custom )
		return false;

	  bool result = false;
	  HouseFloorZColumn* elems;
	  HouseFloorZColumn::iterator itr;
	  CustomHouseDesign* design;
	  design = editing ? &WorkingDesign : &CurrentDesign; //consider having a list of players that should use the working set

	  if ( x + design->xoff < 0 || x + design->xoff >= static_cast<s32>( design->width ) ||
		   y + design->yoff < 0 || y + design->yoff >= static_cast<s32>( design->height ) )
		   return false;
	  for ( int i = 0; i < CUSTOM_HOUSE_NUM_PLANES; i++ )
	  {
		elems = design->Elements[i].GetElementsAt( x, y );
		for ( itr = elems->begin(); itr != elems->end(); ++itr )
		{
          Core::StaticRec rec( itr->graphic, static_cast<signed char>( itr->z + zbase ), Core::tile_flags( itr->graphic ), Core::tileheight( itr->graphic ) );
		  if ( !rec.height )
		  {
			++rec.height;
			--rec.z;
		  }
		  vec.push_back( rec );
		  result = true;
		}
	  }
	  return result;

	}

	//consider: storing editing char serial list on the house, to validate who should see the working set
	UHouse* UHouse::FindWorkingHouse( u32 chrserial )
	{
      Mobile::Character* chr = Core::find_character( chrserial );
	  if ( chr == NULL )
		return NULL;
	  if ( chr->client == NULL )
		return NULL;

	  u32 house_serial = chr->client->gd->custom_house_serial;

      UMulti* multi = Core::system_find_multi( house_serial );
	  if ( multi == NULL )
		return NULL;

	  UHouse* house = multi->as_house();
	  if ( house == NULL )
		return NULL;

	  return house;
	}

	//fixme realm
	bool multis_exist_in( unsigned short mywest, unsigned short mynorth,
						  unsigned short myeast, unsigned short mysouth, Plib::Realm* realm )
	{
	  unsigned short wxL, wyL, wxH, wyH;

      Core::zone_convert_clip( mywest - 100, mynorth - 100, realm, wxL, wyL );
      Core::zone_convert_clip( myeast + 100, mysouth + 100, realm, wxH, wyH );
	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          Core::ZoneMultis& wmulti = realm->zone[wx][wy].multis;

          for ( Core::ZoneMultis::iterator itr = wmulti.begin( ), end = wmulti.end( ); itr != end; ++itr )
		  {
			UMulti* it = ( *itr );
			const MultiDef& edef = it->multidef();
			// find out if any of our walls would fall within its footprint. 
			unsigned short itswest, itseast, itsnorth, itssouth;

			itswest = static_cast<unsigned short>( it->x + edef.minrx );
			itseast = static_cast<unsigned short>( it->x + edef.maxrx );
			itsnorth = static_cast<unsigned short>( it->y + edef.minry );
			itssouth = static_cast<unsigned short>( it->y + edef.maxry );

			if ( mynorth >= itsnorth && mynorth <= itssouth )         // North
			{
			  if ( ( mywest >= itswest && mywest <= itseast ) ||     // NW
				   ( myeast >= itswest && myeast <= itseast ) )       // NE
			  {
				return true;
			  }
			}
			if ( mysouth >= itsnorth && mysouth <= itssouth )         // South
			{
			  if ( ( mywest >= itswest && mywest <= itseast ) ||     // SW
				   ( myeast >= itswest && myeast <= itseast ) )       // SE
			  {
				return true;
			  }

			}

			if ( itsnorth >= mynorth && itsnorth <= mysouth )         // North
			{
			  if ( ( itswest >= mywest && itswest <= myeast ) ||     // NW
				   ( itseast >= mywest && itseast <= myeast ) )       // NE
			  {
				return true;
			  }
			}
			if ( itssouth >= mynorth && itssouth <= mysouth )         // South
			{
			  if ( ( itswest >= mywest && itswest <= myeast ) ||     // SW
				   ( itseast >= mywest && itseast <= myeast ) )       // SE
			  {
				return true;
			  }
			}
		  }
		}
	  }
	  return false;
	}

	bool objects_exist_in( unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, Plib::Realm* realm )
	{
	  unsigned short wxL, wyL, wxH, wyH;
      Core::zone_convert_clip( x1, y1, realm, wxL, wyL );
      Core::zone_convert_clip( x2, y2, realm, wxH, wyH );
	  for ( unsigned short wx = wxL; wx <= wxH; ++wx )
	  {
		for ( unsigned short wy = wyL; wy <= wyH; ++wy )
		{
          Core::ZoneCharacters& wchr = realm->zone[wx][wy].characters;
          for ( Core::ZoneCharacters::iterator itr = wchr.begin( ), end = wchr.end( ); itr != end; ++itr )
		  {
			Mobile::Character* chr = *itr;
			if ( chr->x >= x1 && chr->x <= x2 &&
				 chr->y >= y1 && chr->y <= y2 )
			{
			  return true;
			}
		  }

          Core::ZoneItems& witem = realm->zone[wx][wy].items;
          for ( Core::ZoneItems::iterator itr = witem.begin( ), end = witem.end( ); itr != end; ++itr )
		  {
			Items::Item* item = *itr;
			if ( item->x >= x1 && item->x <= x2 &&
				 item->y >= y1 && item->y <= y2 )
			{
			  return true;
			}
		  }
		}
	  }
	  return false;
	}

	bool statics_cause_problems( unsigned short x1, unsigned short y1,
								 unsigned short x2, unsigned short y2,
								 s8 z, int flags, Plib::Realm* realm )
	{
	  for ( unsigned short x = x1; x <= x2; ++x )
	  {
		for ( unsigned short y = y1; y <= y2; ++y )
		{
		  short newz;
		  UMulti* multi;
		  Items::Item* item;
          if ( !realm->walkheight( x, y, z, &newz, &multi, &item, true, Core::MOVEMODE_LAND ) )
		  {
			Clib::Log( "Refusing to place house at %d,%d,%d: can't stand there\n",
				 int( x ), int( y ), int( z ) );
			return true;
		  }
		  if ( labs( z - newz ) > 2 )
		  {
			Clib::Log( "Refusing to place house at %d,%d,%d: result Z (%d) is too far afield\n",
				 int( x ), int( y ), int( z ), int( newz ) );
			return true;
		  }
		}
	  }
	  return false;
	}

    Bscript::BObjectImp* UHouse::scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y, s8 z, Plib::Realm* realm, int flags )
	{
	  const MultiDef* md = MultiDefByMultiID( descriptor.multiid );
	  if ( md == NULL )
	  {
        return new Bscript::BError( "Multi definition not found for House, objtype="
                                    + Clib::hexint( descriptor.objtype ) + ", multiid="
                                    + Clib::hexint( descriptor.multiid ) );
	  }

	  if ( ( !realm->valid( x + md->minrx, y + md->minry, z + md->minrz ) ) ||
		   ( !realm->valid( x + md->maxrx, y + md->maxry, z + md->maxrz ) ) )
           return new Bscript::BError( "That location is out of bounds" );

	  if ( ~flags & CRMULTI_IGNORE_MULTIS )
	  {
		if ( multis_exist_in( x + md->minrx - 1, y + md->minry - 5,
		  x + md->maxrx + 1, y + md->maxry + 5, realm ) )
		{
          return new Bscript::BError( "Location intersects with another structure" );
		}
	  }

	  if ( ~flags & CRMULTI_IGNORE_OBJECTS )
	  {
		if ( objects_exist_in( x + md->minrx, y + md->minry,
		  x + md->maxrx, y + md->maxry, realm ) )
		{
          return new Bscript::BError( "Something is blocking that location" );
		}
	  }
	  if ( ~flags & CRMULTI_IGNORE_FLATNESS )
	  {
		if ( statics_cause_problems( x + md->minrx - 1, y + md->minry - 1,
		  x + md->maxrx + 1, y + md->maxry + 1,
		  z,
		  flags, realm ) )
		{
          return new Bscript::BError( "That location is not suitable" );
		}
	  }

	  UHouse* house = new UHouse( descriptor );
	  house->serial = Core::GetNewItemSerialNumber();
	  house->serial_ext = ctBEu32( house->serial );
	  house->x = x;
	  house->y = y;
	  house->z = z;
	  house->realm = realm;
	  send_multi_to_inrange( house );
	  // update_item_to_inrange( house );
	  add_multi_to_world( house );
	  house->create_components();

	  ////Hash
      Core::objecthash.Insert( house );
	  ////

	  return house->make_ref();
	}


	void move_to_ground( Items::Item* item )
	{
	  item->set_dirty();
	  item->movable( true );
	  item->set_decay_after( 60 ); // just a dummy in case decay=0
	  item->restart_decay_timer();
	  for ( unsigned short xd = 0; xd < 5; ++xd )
	  {
		for ( unsigned short yd = 0; yd < 5; ++yd )
		{
		  Items::Item* walkon;
		  UMulti* multi;
		  short newz;
		  unsigned short sx = item->x;
		  unsigned short sy = item->y;
		  item->x = 0;    // move 'self' a bit so it doesn't interfere with itself
		  item->y = 0;
          bool res = item->realm->walkheight( sx + xd, sy + yd, item->z, &newz, &multi, &walkon, true, Core::MOVEMODE_LAND );
		  item->x = sx;
		  item->y = sy;
		  if ( res )
		  {
			move_item( item, item->x + xd, item->y + yd, static_cast<signed char>( newz ), NULL );
			return;
		  }
		}
	  }
	  short newz;
	  if ( item->realm->groundheight( item->x, item->y, &newz ) )
	  {
		move_item( item, item->x, item->y, static_cast<signed char>( newz ), NULL );
		return;
	  }
	}


	void move_to_ground( Mobile::Character* chr )
	{
      move_character_to( chr, chr->x, chr->y, chr->z, Core::MOVEITEM_FORCELOCATION, NULL );
	}

	//void send_remove_object_if_inrange( Client *client, const UObject *item );

    Bscript::BObjectImp* destroy_house( UHouse* house )
	{
	  if ( house->IsEditing() )
        return new Bscript::BError( "House currently being customized." );

	  house->destroy_components();

	  ItemList item_contents;
	  MobileList chr_contents;
	  UHouse::list_contents( house, item_contents, chr_contents );

      Clib::ConstForEach( Core::clients, Core::send_remove_object_if_inrange, static_cast<const Items::Item*>( house ) );
	  remove_multi_from_world( house );

	  while ( !item_contents.empty() )
	  {
		Items::Item* item = item_contents.front();
		move_to_ground( item );

		item_contents.pop_front();
	  }

	  while ( !chr_contents.empty() )
	  {
		Mobile::Character* chr = chr_contents.back();
		move_to_ground( chr );
		chr->registered_house = 0;
		chr_contents.pop_back();
	  }

	  house->ClearSquatters();

	  house->destroy();

      return new Bscript::BLong( 1 );
	}

	void UHouse::register_object( UObject* obj )
	{
	  if ( !obj->ismobile() )
		return;
	  if ( find( squatters_.begin(), squatters_.end(), obj ) == squatters_.end() )
	  {
		set_dirty();
		squatters_.push_back( Squatter( obj ) );
	  }
	}

	void UHouse::unregister_object( UObject* obj )
	{
	  Squatters::iterator this_squatter = find( squatters_.begin(), squatters_.end(), obj );

	  if ( this_squatter != squatters_.end() )
	  {
		set_dirty();
		squatters_.erase( this_squatter );
	  }
	}

	void UHouse::ClearSquatters()
	{
	  squatters_.clear();
	}

	void UHouse::walk_on( Mobile::Character* chr )
	{
      const Items::ItemDesc& itemdesc = Items::find_itemdesc( objtype_ );
	  if ( !itemdesc.walk_on_script.empty() )
	  {
        ref_ptr<Bscript::EScriptProgram> prog;
		prog = find_script2( itemdesc.walk_on_script,
							 true, // complain if not found
                             Core::config.cache_interactive_scripts );
		if ( prog.get() != NULL )
		{
          std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor( ) );
		  ex->addModule( new Module::UOExecutorModule( *ex ) );
		  if ( prog->haveProgram )
		  {
            ex->pushArg( new Bscript::BLong( chr->lastz ) );
            ex->pushArg( new Bscript::BLong( chr->lasty ) );
            ex->pushArg( new Bscript::BLong( chr->lastx ) );
            ex->pushArg( new Module::EItemRefObjImp( this ) );
            ex->pushArg( new Module::ECharacterRefObjImp( chr ) );
		  }

		  ex->os_module->priority = 100;

		  if ( ex->setProgram( prog.get() ) )
		  {
			schedule_executor( ex.release() );
		  }
		}
	  }
	}

	void UHouse::AcceptHouseCommit( Mobile::Character* chr, bool accept )
	{
	  waiting_for_accept = false;
	  if ( accept )
	  {
		revision++;

		//commit working design to current design
		CurrentDesign = WorkingDesign;

		//invalidate old packet
		vector<u8> newvec;
		CurrentCompressed.swap( newvec );

		CustomHouseStopEditing( chr, this );

		//send full house
		CustomHousesSendFullToInRange( this, HOUSE_DESIGN_CURRENT, RANGE_VISUAL_LARGE_BUILDINGS );
	  }
	  else
	  {
		WorkingDesign.AddComponents( this );
		CustomHouseDesign::ClearComponents( this );
		if ( chr && chr->client )
		  CustomHousesSendFull( this, chr->client, HOUSE_DESIGN_WORKING );
	  }
	}
  }
}
