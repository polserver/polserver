/*
History
=======
2005/01/24 Shinigami: added ObjMember character.spyonclient2 to get data from packet 0xd9 (Spy on Client 2)
2005/03/09 Shinigami: Added Prop Character::Delay_Mod [ms] for WeaponDelay
2005/04/02 Shinigami: UBoat::script_method_id & UBoat::script_method - added optional realm param
2005/04/04 Shinigami: Added Prop Character::CreatedAt [PolClock]
2005/08/29 Shinigami: get-/setspyonclient2 renamed to get-/setclientinfo
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2005/12/09 MuadDib:   Added uclang member for storing UC language from client.
2005/12/09 MuadDib:   Fixed ~ItemGivenEvent not returning items correctly if the script
                      did nothing with it. Was incorrect str/int comparision for times.
2006/05/16 Shinigami: added Prop Character.Race [RACE_* Constants] to support Elfs
2006/06/15 Austin:    Added mobile.Privs()
2007/07/09 Shinigami: added Prop Character.isUOKR [bool] - UO:KR client used?
2008/07/08 Turley:    Added character.movemode - returns the MoveMode like given in NPCDesc
                      Added item.stackable - Is item stackable?
2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
2009/07/20 MuadDib:   ReAdded on_facing_change() to MBR_FACING: for items.
2009/08/06 MuadDib:   Added gotten_by code for items.
2009/08/31 MuadDib:   Changed layer check on graphic change, excluding mount layer entirely.
2009/09/03 MuadDib:   Relocation of account related cpp/h
                      Relocation of multi related cpp/h
2009/09/06 Turley:    Removed isUOKR added ClientType
2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue) virtuals.
2009/10/09 Turley:    Added spellbook.spells() & .hasspell() methods

Notes
=======

*/

#include "../clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif


#include "../bscript/berror.h"
#include "../bscript/dict.h"
#include "../bscript/escrutil.h"
#include "../bscript/execmodl.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"

#include "../clib/endian.h"
#include "../clib/mlog.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/unicode.h"

#include "../plib/realm.h"

#include "accounts/account.h"
#include "accounts/acscrobj.h"
#include "item/armor.h"
#include "item/armrtmpl.h"
#include "mobile/attribute.h"
#include "multi/boatcomp.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "cmdlevel.h"
#include "door.h"
#include "exscrobj.h"
#include "fnsearch.h"
#include "module/guildmod.h"
#include "multi/house.h"
#include "item/item.h"
#include "umap.h"
#include "npc.h"
#include "objtype.h"
#include "polclass.h"
#include "realms.h"
#include "spelbook.h"
#include "statmsg.h"
#include "syshookscript.h"
#include "tooltips.h"
#include "ufunc.h"
#include "uofile.h"
#include "item/weapon.h"
#include "item/wepntmpl.h"
#include "module/uomod.h"
#include "uoexhelp.h"
#include "uworld.h"
#include "module/partymod.h"

#include "uoscrobj.h"

BApplicObjType euboatrefobjimp_type;
BApplicObjType emultirefobjimp_type;
BApplicObjType eitemrefobjimp_type;
BApplicObjType echaracterrefobjimp_type;
BApplicObjType echaracterequipobjimp_type;
BApplicObjType storage_area_type;
BApplicObjType menu_type;

const char* ECharacterRefObjImp::typeOf() const
{
	return "MobileRef";
}

BObjectImp* ECharacterRefObjImp::copy() const
{
	return new ECharacterRefObjImp( obj_.get() );
}

BObjectRef ECharacterRefObjImp::get_member_id( const int id )
{
	BObjectImp* result = obj_->get_script_member_id( id );
	if (result != NULL)
		return BObjectRef(result);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef ECharacterRefObjImp::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef ECharacterRefObjImp::set_member_id( const int id, BObjectImp* value )
{
	BObjectImp* result = NULL;
	if (value->isa( BObjectImp::OTLong ))
	{
		BLong* lng = static_cast<BLong*>(value);
		result = obj_->set_script_member_id( id, lng->value() );
	}
	else if (value->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(value);
		result = obj_->set_script_member_id( id, str->value() );
	}
	else if (value->isa( BObjectImp::OTDouble ))
	{
		Double* dbl = static_cast<Double*>(value);
		result = obj_->set_script_member_id_double( id, dbl->value() );
	}
	if (result != NULL)
	{
		return BObjectRef( result );
	}
	else
	{
		return BObjectRef(UninitObject::create());
	}
}

BObjectRef ECharacterRefObjImp::set_member( const char* membername, BObjectImp* value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_member_id(objmember->id, value);
	else
		return BObjectRef(UninitObject::create());
}

BObjectImp* ECharacterRefObjImp::call_method_id( const int id, Executor& ex )
{
	if (!obj_->orphan())
	{
		BObjectImp* imp = obj_->script_method_id( id, ex );
		if (imp != NULL)
			return imp;
		else
			return base::call_method_id( id, ex );
	}
	else
	{
		return new BError( "That object no longer exists" );
	}
}

BObjectImp* ECharacterRefObjImp::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return base::call_method(methodname, ex);
}

bool ECharacterRefObjImp::isTrue() const
{
	return (!obj_->orphan() && obj_->logged_in);
}

bool ECharacterRefObjImp::isEqual(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTApplicObj ))
	{
		const BApplicObjBase* aob = explicit_cast<const BApplicObjBase*, const BObjectImp*>(&objimp);

		if (aob->object_type() == &echaracterrefobjimp_type)
		{
			const ECharacterRefObjImp* chrref_imp =
					explicit_cast<const ECharacterRefObjImp*,const BApplicObjBase*>(aob);

			return (chrref_imp->obj_->serial == obj_->serial);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool ECharacterRefObjImp::isLessThan(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTApplicObj ))
	{
		const BApplicObjBase* aob = explicit_cast<const BApplicObjBase*, const BObjectImp*>(&objimp);

		if (aob->object_type() == &echaracterrefobjimp_type)
		{
			const ECharacterRefObjImp* chrref_imp =
					explicit_cast<const ECharacterRefObjImp*,const BApplicObjBase*>(aob);

			return (chrref_imp->obj_->serial < obj_->serial);
		}
		else if (aob->object_type() == &eitemrefobjimp_type)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

const char* EOfflineCharacterRefObjImp::typeOf() const
{
	return "OfflineMobileRef";
}

BObjectImp* EOfflineCharacterRefObjImp::copy() const
{
	return new EOfflineCharacterRefObjImp( obj_.get() );
}

bool EOfflineCharacterRefObjImp::isTrue() const
{
	return (!obj_->orphan());
}

const char* EItemRefObjImp::typeOf() const
{
	return "ItemRef";
}

BObjectImp* EItemRefObjImp::copy() const
{
	return new EItemRefObjImp( obj_.get() );
}

BObjectRef EItemRefObjImp::get_member_id( const int id )
{
	BObjectImp* result = obj_->get_script_member_id( id );
	if (result != NULL)
		return BObjectRef(result);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef EItemRefObjImp::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef EItemRefObjImp::set_member_id( const int id, BObjectImp* value )
{
	BObjectImp* result = NULL;
	if (value->isa( BObjectImp::OTLong ))
	{
		BLong* lng = static_cast<BLong*>(value);
		result = obj_->set_script_member_id( id, lng->value() );
	}
	else if (value->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(value);
		result = obj_->set_script_member_id( id, str->value() );
	}
	else if (value->isa( BObjectImp::OTDouble ))
	{
		Double* dbl = static_cast<Double*>(value);
		result = obj_->set_script_member_id_double( id, dbl->value() );
	}
	if (result != NULL)
	{
		return BObjectRef( result );
	}
	else
	{
		return BObjectRef(UninitObject::create());
	}
}

BObjectRef EItemRefObjImp::set_member( const char* membername, BObjectImp* value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_member_id(objmember->id, value);
	else
		return BObjectRef(UninitObject::create());
}

BObjectImp* EItemRefObjImp::call_method_id( const int id, Executor& ex )
{
	Item* item = obj_.get();
	if (!item->orphan())
	{
		ObjMethod* mth = getObjMethod(id);
		if ( mth->overridden )
		{
			BObjectImp* imp = item->custom_script_method( mth->code, ex );
			if (imp)
				return imp;
		}

		BObjectImp* imp = item->script_method_id( id, ex );
		if (imp != NULL)
			return imp;
		else
			return base::call_method_id( id, ex );
	}
	else
	{
		return new BError( "That object no longer exists" );
	}
}

BObjectImp* EItemRefObjImp::call_method( const char* methodname, Executor& ex )
{
	if (methodname[0] == '_')
		++methodname;
	
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
	{
		Item* item = obj_.get();
		BObjectImp* imp = item->custom_script_method( methodname, ex );
		if ( imp )
			return imp;
		else
			return base::call_method( methodname, ex );
	}
}

bool EItemRefObjImp::isTrue() const
{
	return (!obj_->orphan());
}

bool EItemRefObjImp::isEqual(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTApplicObj ))
	{
		const BApplicObjBase* aob = explicit_cast<const BApplicObjBase*, const BObjectImp*>(&objimp);

		if (aob->object_type() == &eitemrefobjimp_type)
		{
			const EItemRefObjImp* itemref_imp =
					explicit_cast<const EItemRefObjImp*,const BApplicObjBase*>(aob);

			return (itemref_imp->obj_->serial == obj_->serial);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool EItemRefObjImp::isLessThan(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTApplicObj ))
	{
		const BApplicObjBase* aob = explicit_cast<const BApplicObjBase*, const BObjectImp*>(&objimp);

		if (aob->object_type() == &eitemrefobjimp_type)
		{
			const EItemRefObjImp* itemref_imp =
					explicit_cast<const EItemRefObjImp*,const BApplicObjBase*>(aob);

			return (itemref_imp->obj_->serial < obj_->serial);
		}
		else
		{
			return (&eitemrefobjimp_type < aob->object_type());
		}
	}
	else
	{
		return false;
	}
}

const char* EUBoatRefObjImp::typeOf() const
{
	return "BoatRef";
}

BObjectImp* EUBoatRefObjImp::copy() const
{
	return new EUBoatRefObjImp( obj_.get() );
}

BObjectRef EUBoatRefObjImp::get_member_id( const int id )
{
	BObjectImp* result = obj_->get_script_member_id( id );
	if (result != NULL)
		return BObjectRef(result);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef EUBoatRefObjImp::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef EUBoatRefObjImp::set_member_id( const int id, BObjectImp* value )
{
	BObjectImp* result = NULL;
	if (value->isa( BObjectImp::OTLong ))
	{
		BLong* lng = static_cast<BLong*>(value);
		result = obj_->set_script_member_id( id, lng->value() );
	}
	else if (value->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(value);
		result = obj_->set_script_member_id( id, str->value() );
	}
	else if (value->isa( BObjectImp::OTDouble ))
	{
		Double* dbl = static_cast<Double*>(value);
		result = obj_->set_script_member_id_double( id, dbl->value() );
	}
	if (result != NULL)
	{
		return BObjectRef( result );
	}
	else
	{
		return BObjectRef(UninitObject::create());
	}
}

BObjectRef EUBoatRefObjImp::set_member( const char* membername, BObjectImp* value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_member_id(objmember->id, value);
	else
		return BObjectRef(UninitObject::create());
}

BObjectImp* EUBoatRefObjImp::call_method_id( const int id, Executor& ex )
{
	if (!obj_->orphan())
	{
		BObjectImp* imp = obj_->script_method_id( id, ex );
		if (imp != NULL)
			return imp;
		else
			return base::call_method_id( id, ex );
	}
	else
	{
		return new BError( "That object no longer exists" );
	}
}

BObjectImp* EUBoatRefObjImp::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return base::call_method(methodname, ex);
}

bool EUBoatRefObjImp::isTrue() const
{
	return (!obj_->orphan());
}

bool EUBoatRefObjImp::isEqual(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTApplicObj ))
	{
		const BApplicObjBase* aob = explicit_cast<const BApplicObjBase*, const BObjectImp*>(&objimp);

		if (aob->object_type() == &euboatrefobjimp_type)
		{
			const EUBoatRefObjImp* boatref_imp =
					explicit_cast<const EUBoatRefObjImp*,const BApplicObjBase*>(aob);

			return (boatref_imp->obj_->serial == obj_->serial);
		}
		else
			return false;
	}
	else
		return false;
}

BObjectImp* EMultiRefObjImp::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return base::call_method( methodname, ex );
}

BObjectImp* EMultiRefObjImp::call_method_id( const int id, Executor& ex )
{
	UMulti* multi = obj_.get();
	if (!multi->orphan())
	{
		ObjMethod* mth = getObjMethod(id);
		if ( mth->overridden )
		{
			BObjectImp* imp = multi->custom_script_method( mth->code, ex );
			if (imp)
				return imp;
		}

		BObjectImp* imp = multi->script_method_id( id, ex );
		if (imp != NULL)
			return imp;
		else
			return base::call_method_id( id, ex );
	}
	else
	{
		return new BError( "That object no longer exists" );
	}
}

const char* EMultiRefObjImp::typeOf() const
{
	return "MultiRef";
}

BObjectImp* EMultiRefObjImp::copy() const
{
	return new EMultiRefObjImp( obj_.get() );
}

BObjectRef EMultiRefObjImp::get_member_id( const int id )
{
	BObjectImp* result = obj_->get_script_member_id( id );
	if (result != NULL)
		return BObjectRef(result);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef EMultiRefObjImp::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef(UninitObject::create());
}

BObjectRef EMultiRefObjImp::set_member_id( const int id, BObjectImp* value )
{
	BObjectImp* result = NULL;
	if (value->isa( BObjectImp::OTLong ))
	{
		BLong* lng = static_cast<BLong*>(value);
		result = obj_->set_script_member_id( id, lng->value() );
	}
	else if (value->isa( BObjectImp::OTString ))
	{
		String* str = static_cast<String*>(value);
		result = obj_->set_script_member_id( id, str->value() );
	}
	else if (value->isa( BObjectImp::OTDouble ))
	{
		Double* dbl = static_cast<Double*>(value);
		result = obj_->set_script_member_id_double( id, dbl->value() );
	}
	if (result != NULL)
	{
		return BObjectRef( result );
	}
	else
	{
		return BObjectRef(UninitObject::create());
	}
}
BObjectRef EMultiRefObjImp::set_member( const char* membername, BObjectImp* value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_member_id(objmember->id, value);
	else
		return BObjectRef(UninitObject::create());
}

bool EMultiRefObjImp::isTrue() const
{
	return (!obj_->orphan());
}

bool EMultiRefObjImp::isEqual(const BObjectImp& objimp) const
{
	if (objimp.isa( BObjectImp::OTApplicObj ))
	{
		const BApplicObjBase* aob = explicit_cast<const BApplicObjBase*, const BObjectImp*>(&objimp);

		if (aob->object_type() == &emultirefobjimp_type)
		{
			const EMultiRefObjImp* multiref_imp =
					explicit_cast<const EMultiRefObjImp*,const BApplicObjBase*>(aob);

			return (multiref_imp->obj_->serial == obj_->serial);
		}
		else
			return false;
	}
	else
		return false;
}

#define GET_STRING_MEMBER(v) if (stricmp( membername, #v ) == 0) return new String( v )
#define SET_STRING_MEMBER(v) if (stricmp( membername, #v ) == 0) return new String( v = value )

#define GET_STRING_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new String( v##_ )
#define SET_STRING_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new String( v##_ = value )

#define GET_SHORT_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new BLong( v##_ )
#define SET_SHORT_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new BLong( v##_ = static_cast<short>(value) )

#define GET_SHORT_MEMBER(v) if (stricmp( membername, #v ) == 0) return new BLong( v )
#define SET_SHORT_MEMBER(v) if (stricmp( membername, #v ) == 0) return new BLong( v = static_cast<short>(value) )

#define GET_LONG_MEMBER(v) if (stricmp( membername, #v ) == 0) return new BLong( v )
#define SET_LONG_MEMBER(v) if (stricmp( membername, #v ) == 0) return new BLong( v = value )

#define GET_LONG_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new BLong( v##_ )
#define SET_LONG_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new BLong( v##_ = value )

#define GET_BOOL_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new BLong( v##_ ? 1 : 0)
#define SET_BOOL_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new BLong( v##_ = value?true:false )

#define GET_BOOL_MEMBER(v) if (stricmp( membername, #v ) == 0) return new BLong( v ? 1 : 0)
#define SET_BOOL_MEMBER(v) if (stricmp( membername, #v ) == 0) return new BLong( v = value?true:false )

#define GET_USHORT_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new BLong( v##_ )
#define SET_USHORT_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new BLong( v##_ = static_cast<unsigned short>(value) )

#define GET_USHORT_MEMBER(v) if (stricmp( membername, #v ) == 0) return new BLong( v )
#define SET_USHORT_MEMBER(v) if (stricmp( membername, #v ) == 0) return new BLong( v = static_cast<unsigned short>(value) )

#define GET_DOUBLE_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new Double( v##_ )
#define SET_DOUBLE_MEMBER_(v) if (stricmp( membername, #v ) == 0) return new Double( v##_ = static_cast<double>(value) )

BObjectImp* UObject::get_script_member_id( const int id ) const
{
	if (orphan())
		return new UninitObject;
	switch(id)
	{
		case MBR_X: return new BLong(x); break;
		case MBR_Y: return new BLong(y); break;
		case MBR_Z: return new BLong(z); break;
		case MBR_NAME: return new String(name_); break;
		case MBR_OBJTYPE: return new BLong(objtype_); break;
		case MBR_GRAPHIC: return new BLong(graphic); break;
		case MBR_SERIAL: return new BLong(serial); break;
		case MBR_COLOR: return new BLong(color); break;
		case MBR_HEIGHT: return new BLong(height); break;
		case MBR_FACING: return new BLong(facing); break;
		case MBR_DIRTY: return new BLong(dirty_ ? 1 : 0); break;
		case MBR_WEIGHT: return new BLong(weight()); break;
		case MBR_MULTI:
			UMulti* multi;
			if( NULL != (multi = realm->find_supporting_multi(x,y,z)) )
				return multi->make_ref();
			else
				return new BLong(0);
			break;
		case MBR_REALM: 
			if ( realm != NULL )
				return new String(realm->name());
			else
				return new BError("object does not belong to a realm.");
			break;
		default: return NULL;
	}
}

BObjectImp* UObject::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* UObject::set_script_member_id( const int id, const string& value )
{
	if (orphan())
		return new UninitObject;

	set_dirty();
	switch(id)
	{
		case MBR_NAME:
			if (ismobile() && (value.empty() || isspace(value[0])) )
				return new BError( "mobile.name must not be empty, and must not begin with a space" );
			setname( value );
			return new String( name() );
			break;
		default: return NULL;
	}
}

BObjectImp* UObject::set_script_member( const char *membername, const string& value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* UObject::set_script_member_id( const int id, long value )
{
    if (orphan())
        return new UninitObject;

    set_dirty();
    switch(id)
    {
    case MBR_GRAPHIC:
        setgraphic( static_cast<unsigned short>(value) );
        return new BLong( graphic );
    case MBR_COLOR:
        {
            bool res = setcolor( static_cast<unsigned short>(value) );
            on_color_changed();
            if (!res) // TODO log?
                return new BError( "Invalid color value " + hexint(value) );
            else
                return new BLong( color );
        }
    default:
        return NULL;
    }
}

BObjectImp* UObject::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* UObject::set_script_member_id_double( const int id, double value )
{
	set_dirty();
	return NULL;
}

BObjectImp* UObject::set_script_member_double( const char *membername, double value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id_double(objmember->id, value);
	else
		return NULL;
}

BObjectImp* Item::get_script_member_id( const int id ) const
{
	BObjectImp* imp = base::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_AMOUNT: return new BLong(amount_); break;
		case MBR_LAYER: return new BLong(layer); break;
		case MBR_TILE_LAYER: return new BLong(tile_layer); break;
		case MBR_CONTAINER:
			if (container != NULL)
				return container->make_ref();
			else
				return new BLong(0);
			break;
		case MBR_USESCRIPT: return new String( on_use_script_ ); break;
		case MBR_EQUIPSCRIPT: return new String( equip_script_ ); break;
		case MBR_UNEQUIPSCRIPT: return new String( unequip_script_ ); break;
		case MBR_DESC: return new String( description() ); break;
		case MBR_MOVABLE: return new BLong( movable_ ? 1 : 0); break;
		case MBR_INVISIBLE: return new BLong( invisible_ ? 1 : 0); break;
		case MBR_DECAYAT: return new BLong( decayat_gameclock_ ); break;
		case MBR_SELLPRICE: return new BLong(sellprice()); break;
		case MBR_BUYPRICE: return new BLong(buyprice()); break;
		case MBR_NEWBIE: return new BLong( newbie_ ? 1 : 0); break;
		case MBR_ITEM_COUNT: return new BLong( item_count() ); break;
		case MBR_STACKABLE:  return new BLong( stackable() ? 1 : 0); break;
		case MBR_SAVEONEXIT: return new BLong( saveonexit()); break;
		case MBR_FIRE_RESIST: return new BLong(calc_element_resist(ELEMENTAL_FIRE)); break;
		case MBR_COLD_RESIST: return new BLong(calc_element_resist(ELEMENTAL_COLD)); break;
		case MBR_ENERGY_RESIST: return new BLong(calc_element_resist(ELEMENTAL_ENERGY)); break;
		case MBR_POISON_RESIST: return new BLong(calc_element_resist(ELEMENTAL_POISON)); break;
		case MBR_PHYSICAL_RESIST: return new BLong(calc_element_resist(ELEMENTAL_PHYSICAL)); break;
		case MBR_FIRE_RESIST_MOD: return new BLong(element_resist_mod.fire); break;
		case MBR_COLD_RESIST_MOD: return new BLong(element_resist_mod.cold); break;
		case MBR_ENERGY_RESIST_MOD: return new BLong(element_resist_mod.energy); break;
		case MBR_POISON_RESIST_MOD: return new BLong(element_resist_mod.poison); break;
		case MBR_PHYSICAL_RESIST_MOD: return new BLong(element_resist_mod.physical); break;
		case MBR_FIRE_DAMAGE: return new BLong(calc_element_damage(ELEMENTAL_FIRE)); break;
		case MBR_COLD_DAMAGE: return new BLong(calc_element_damage(ELEMENTAL_COLD)); break;
		case MBR_ENERGY_DAMAGE: return new BLong(calc_element_damage(ELEMENTAL_ENERGY)); break;
		case MBR_POISON_DAMAGE: return new BLong(calc_element_damage(ELEMENTAL_POISON)); break;
		case MBR_PHYSICAL_DAMAGE: return new BLong(calc_element_damage(ELEMENTAL_PHYSICAL)); break;
		case MBR_FIRE_DAMAGE_MOD: return new BLong(element_damage_mod.fire); break;
		case MBR_COLD_DAMAGE_MOD: return new BLong(element_damage_mod.cold); break;
		case MBR_ENERGY_DAMAGE_MOD: return new BLong(element_damage_mod.energy); break;
		case MBR_POISON_DAMAGE_MOD: return new BLong(element_damage_mod.poison); break;
		case MBR_PHYSICAL_DAMAGE_MOD: return new BLong(element_damage_mod.physical); break;
		case MBR_GETGOTTENBY:
			if( this->is_gotten() )
				return new ECharacterRefObjImp(this->gotten_by);
			else
				return new BError( "Gotten By NULL" );
			break;
		default: return NULL;
	}
}

BObjectImp* Item::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* Item::set_script_member_id( const int id, const string& value )
{
	BObjectImp* imp = base::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_USESCRIPT:
			on_use_script_ = value;
			return new String( value );
		case MBR_EQUIPSCRIPT:
			equip_script_ = value;
			return new String( value );
		case MBR_UNEQUIPSCRIPT:
			unequip_script_ = value;
			return new String( value );
		default: return NULL;
	}
}

BObjectImp* Item::set_script_member( const char *membername, const string& value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* Item::set_script_member_id( const int id, long value )
{
	BObjectImp* imp = base::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_MOVABLE:
			restart_decay_timer();
			movable_ = value?true:false;
			increv();
			on_color_changed();
			return new BLong( movable_ );
		case MBR_INVISIBLE:
			restart_decay_timer();
			invisible_ = value?true:false;
			increv();
			on_invisible_changed();
			return new BLong( invisible_ );
		case MBR_DECAYAT:
			decayat_gameclock_ = value;
			return new BLong( decayat_gameclock_ );
		case MBR_SELLPRICE:
			return new BLong( sellprice_ = value );
		case MBR_BUYPRICE:
			return new BLong( buyprice_ = value );
		case MBR_NEWBIE:
			increv();
			restart_decay_timer();
			return new BLong( newbie_ = value?true:false );
		case MBR_FACING:
			setfacing((u8)value);
			on_facing_changed();
			return new BLong(facing);
		case MBR_SAVEONEXIT:
			saveonexit(value?true:false);
			return new BLong( saveonexit() );
		case MBR_FIRE_RESIST_MOD: 
			return new BLong( element_resist_mod.fire = static_cast<short>(value) ); 
			break;
		case MBR_COLD_RESIST_MOD: 
			return new BLong( element_resist_mod.cold = static_cast<short>(value) ); 
			break;
		case MBR_ENERGY_RESIST_MOD: 
			return new BLong( element_resist_mod.energy = static_cast<short>(value) ); 
			break;
		case MBR_POISON_RESIST_MOD: 
			return new BLong( element_resist_mod.poison = static_cast<short>(value) ); 
			break;
		case MBR_PHYSICAL_RESIST_MOD: 
			return new BLong( element_resist_mod.physical = static_cast<short>(value) ); 
			break;
		case MBR_FIRE_DAMAGE_MOD: 
			return new BLong( element_damage_mod.fire = static_cast<short>(value) ); 
			break;
		case MBR_COLD_DAMAGE_MOD: 
			return new BLong( element_damage_mod.cold = static_cast<short>(value) ); 
			break;
		case MBR_ENERGY_DAMAGE_MOD: 
			return new BLong( element_damage_mod.energy = static_cast<short>(value) ); 
			break;
		case MBR_POISON_DAMAGE_MOD: 
			return new BLong( element_damage_mod.poison = static_cast<short>(value) ); 
			break;
		case MBR_PHYSICAL_DAMAGE_MOD: 
			return new BLong( element_damage_mod.physical = static_cast<short>(value) ); 
			break;
		default: return NULL;
	}
}

BObjectImp* Item::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* Item::custom_script_method( const char* methodname, Executor& ex )
{
	const ItemDesc& id = itemdesc();
	if (id.method_script != NULL)
	{
		unsigned PC;
		if (id.method_script->FindExportedFunction( methodname, ex.numParams()+1, PC ))
		{
			return id.method_script->call( PC, make_ref(), ex.fparams );
		}
	}
	return NULL;
}

BObject Item::call_custom_method( const char* methodname )
{
	BObjectImpRefVec noparams;
	return call_custom_method( methodname, noparams );
}

BObject Item::call_custom_method( const char* methodname, BObjectImpRefVec& pmore )
{
	const ItemDesc& id = itemdesc();
	if (id.method_script != NULL)
	{
		unsigned PC;
		if (id.method_script->FindExportedFunction( methodname, pmore.size()+1, PC ))
		{
			return id.method_script->call( PC, new EItemRefObjImp(this), pmore );
		}
		else
		{
			string message;
			message = "Method script for objtype "
								+ id.objtype_description()
								+ " does not export function "
								+ string(methodname)
								+ " taking "
								+ decint( pmore.size() + 1 )
								+ " parameters";
			BError* err = new BError( message );
			return BObject( err );
		}
	}
	else
	{
		return BObject( new BError( "No method script defined for " + id.objtype_description() ) );
	}
}

class PrivUpdater
{
public:
	static void on_enable( Character* chr, const char* pstr )
	{
		if ( chr != NULL )
		{

/*			if ( chr->client )
			{
				cached_settings.clotheany	     = setting_enabled( "clotheany" );
				cached_settings.dblclickany	     = setting_enabled( "dblclickany" );
				cached_settings.hearghosts	     = setting_enabled( "hearghosts" );
				cached_settings.invul		     = setting_enabled( "invul" );
				cached_settings.losany		     = setting_enabled( "losany" );
				cached_settings.moveany		     = setting_enabled( "moveany" );
				cached_settings.renameany	     = setting_enabled( "renameany" );
				cached_settings.seeghosts	     = setting_enabled( "seeghosts" );
				cached_settings.seehidden	     = setting_enabled( "seehidden" );
				cached_settings.seeinvisitems    = setting_enabled( "seeinvisitems" );
				cached_settings.ignoredoors	     = setting_enabled( "ignoredoors" );
				cached_settings.freemove		 = setting_enabled( "freemove" );
				cached_settings.firewhilemoving = setting_enabled( "firewhilemoving" );
				cached_settings.attackhidden     = setting_enabled( "attackhidden" );
				cached_settings.hiddenattack     = setting_enabled( "hiddenattack" );
				cached_settings.plogany		     = setting_enabled( "plogany" );
				cached_settings.moveanydist		 = setting_enabled( "moveanydist" );
			}
*/
		}
	}
	static void on_disable( Character* chr, const char* pstr )
	{
		if ( chr != NULL )
		{
			if ( chr->client )
			{
				
			}
		}
	}
};

class ARUpdater
{
public:
	static void on_change( Character* chr )
	{
		chr->refresh_ar(); // FIXME inefficient
		//if (chr->client != NULL) // already send in refresh_ar()
		//{
		//	send_full_statmsg( chr->client, chr );
		//}
	}
};
class HiddenUpdater
{
public:
	static void on_change( Character* chr )
	{
		if (chr->hidden())
		{
			chr->set_stealthsteps(0);
			if (chr->client)
				send_move( chr->client, chr );
			send_remove_character_to_nearby_cantsee( chr );
			send_create_mobile_to_nearby_cansee( chr );
		}
		else
		{
			chr->unhide();
			chr->set_stealthsteps(0);
		}
	}
};


class ConcealedUpdater
{
public:
	static void on_change( Character* chr )
	{
		if (chr->concealed())
		{
			if (chr->client)
				send_move( chr->client, chr );
			send_remove_character_to_nearby_cantsee( chr );
			send_create_mobile_to_nearby_cansee( chr );
		}
		else if (chr->is_visible())
		{
			chr->unhide();
		}
	}
};

BObjectImp* Character::make_ref()
{
	return new ECharacterRefObjImp( this );
}
BObjectImp* Character::make_offline_ref()
{
	return new EOfflineCharacterRefObjImp( this );
}

BObjectImp* Character::get_script_member_id( const int id ) const
{
	BObjectImp* imp = base::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	Item* bp = NULL;
	string mode="";

	switch(id)
	{
		case MBR_WARMODE: return new BLong(warmode); break;
		case MBR_GENDER: return new BLong(gender); break;
		case MBR_RACE: return new BLong(race); break;
		case MBR_TRUEOBJTYPE: return new BLong(trueobjtype); break;
		case MBR_TRUECOLOR: return new BLong(truecolor); break;
		case MBR_AR_MOD: return new BLong(ar_mod()); break;
		case MBR_DELAY_MOD: return new BLong(delay_mod_); break;
		case MBR_HIDDEN: return new BLong(hidden_ ? 1 : 0); break;
		case MBR_CONCEALED: return new BLong(concealed_); break;
		case MBR_FROZEN: return new BLong(frozen_ ? 1 : 0); break;
		case MBR_PARALYZED: return new BLong(paralyzed_ ? 1 : 0); break;
		case MBR_POISONED: return new BLong(poisoned ? 1 : 0); break;
		case MBR_STEALTHSTEPS: return new BLong(stealthsteps_); break;
		case MBR_SQUELCHED: return new BLong( squelched() ? 1 : 0 ); break;
		case MBR_DEAD: return new BLong(dead_); break;
		case MBR_AR: return new BLong(ar()); break;
		case MBR_BACKPACK:
			bp = backpack();
			if (bp != NULL)
				return bp->make_ref();
			else
				return new BError( "That has no backpack" );
			break;
		case MBR_WEAPON:
			if (weapon != NULL)
				return weapon->make_ref();
			else
				return new BLong(0);
			break;
		case MBR_SHIELD:
			if (shield != NULL)
				return shield->make_ref();
			else
				return new BLong(0);
			break;
		case MBR_ACCTNAME:
			if (acct != NULL)
				return new String( acct->name() );
			else
				return new BError( "Not attached to an account" );
			break;

		case MBR_ACCT:
			if (acct != NULL)
				return new AccountObjImp( AccountPtrHolder(acct) );
			else
				return new BError( "Not attached to an account" );
			break;
		case MBR_CMDLEVEL: return new BLong(cmdlevel); break;
		case MBR_CMDLEVELSTR: return new String( cmdlevels2[cmdlevel].name ); break;
		case MBR_CRIMINAL: return new BLong( is_criminal() ? 1 : 0 ); break;
		case MBR_IP:
			if (client != NULL)
				return new String( client->ipaddrAsString() );
			else
				return new String( "" );
			break;
		case MBR_GOLD: return new BLong( gold_carried() ); break;

		case MBR_TITLE_PREFIX: return new String(title_prefix); break;
		case MBR_TITLE_SUFFIX: return new String(title_suffix); break;
		case MBR_TITLE_GUILD: return new String(title_guild); break;
		case MBR_TITLE_RACE: return new String(title_race); break;
		case MBR_UCLANG: return new String(uclang); break;
		case MBR_GUILDID: return new BLong( guildid() ); break;
		case MBR_GUILD:
			if (guild_ != NULL)
				return CreateGuildRefObjImp( guild_ );
			else
				return new BError( "Not a member of a guild" );
			break;


		case MBR_MURDERER: return new BLong( murderer_ ? 1 : 0 ); break;
		case MBR_ATTACHED:
			if (script_ex == NULL)
				return new BError( "No script attached." );
			else
				return new ScriptExObjImp( script_ex );
			break;
		case MBR_CLIENTVERSION:
			if (client != NULL)
				return new String(client->getversion());
			else
				return new String("");
			break;
		case MBR_CLIENTVERSIONDETAIL:
			if (client != NULL)
			{
				BStruct* info = new BStruct;
				info->addMember("major",  new BLong(client->getversiondetail().major));
				info->addMember("minor",  new BLong(client->getversiondetail().minor));
				info->addMember("rev",    new BLong(client->getversiondetail().rev));
				info->addMember("patch",  new BLong(client->getversiondetail().patch));
				return info;
			}
			else
			{
				BStruct* info = new BStruct;
				info->addMember("major",  new BLong(0));
				info->addMember("minor",  new BLong(0));
				info->addMember("rev",    new BLong(0));
				info->addMember("patch",  new BLong(0));
				return info;
			}
			break;

		case MBR_CLIENTINFO:
			if (client != NULL)
				return client->getclientinfo();
			else
				return new BLong(0);
			break;

		case MBR_CREATEDAT: return new BLong( created_at ); break;

		case MBR_REPORTABLES:  return GetReportables(); break;

		case MBR_OPPONENT:
			if ( opponent_ != NULL )
				return opponent_->make_ref();
			else
				return new BError("Mobile does not have any opponent selected.");
			break;
		case MBR_CONNECTED:
			if ( connected )
				return new BLong(1);
			else
				return new BLong(0);
			break;
		case MBR_TRADING_WITH:
			{
				if ( trading_with != NULL )
					return trading_with->make_ref();
				else
					return new BError("Mobile is not currently trading with anyone.");
			}
			break;
		case MBR_CLIENTTYPE:
			if (client != NULL)
				return new BLong( client->ClientType );
			else
				return new BLong(0);
			break;
		case MBR_CURSOR:
			if (client != NULL)
				return new BLong( target_cursor_busy() ? 1 : 0 );
			else
				return new BLong(0);
			break;
		case MBR_GUMP:
			if (client != NULL)
				return new BLong( has_active_gump() ? 1 : 0 );
			else
				return new BLong(0);
			break;
		case MBR_PROMPT:
			if (client != NULL)
				return new BLong( has_active_prompt() ? 1 : 0 );
			else
				return new BLong(0);
			break;
		case MBR_MOVEMODE:
		  if (movemode & MOVEMODE_LAND)
		    mode="L";
		  if (movemode & MOVEMODE_SEA)
		    mode+="S";
		  if (movemode & MOVEMODE_AIR)
		    mode+="A";
		  return new String( mode );
		  break;
	    case MBR_HITCHANCE_MOD: 
			return new BLong(hitchance_mod_); 
			break;
		case MBR_EVASIONCHANCE_MOD: 
			return new BLong(evasionchance_mod_); 
			break;
		case MBR_FIRE_RESIST: 
			return new BLong(element_resist.fire); 
			break;
		case MBR_COLD_RESIST: 
			return new BLong(element_resist.cold); 
			break;
		case MBR_ENERGY_RESIST: 
			return new BLong(element_resist.energy); 
			break;
		case MBR_POISON_RESIST: 
			return new BLong(element_resist.poison); 
			break;
		case MBR_PHYSICAL_RESIST: 
			return new BLong(element_resist.physical); 
			break;
		case MBR_FIRE_RESIST_MOD: 
			return new BLong(element_resist_mod.fire); 
			break;
		case MBR_COLD_RESIST_MOD: 
			return new BLong(element_resist_mod.cold); 
			break;
		case MBR_ENERGY_RESIST_MOD: 
			return new BLong(element_resist_mod.energy); 
			break;
		case MBR_POISON_RESIST_MOD: 
			return new BLong(element_resist_mod.poison); 
			break;
		case MBR_PHYSICAL_RESIST_MOD: 
			return new BLong(element_resist_mod.physical); 
			break;
		case MBR_STATCAP:
			return new BLong(expanded_statbar.statcap);
		case MBR_SKILLCAP:
			return new BLong(skillcap_);
			break;
		case MBR_LUCK:
			return new BLong(expanded_statbar.luck);
		case MBR_FOLLOWERSMAX:
			return new BLong(expanded_statbar.followers_max);
		case MBR_TITHING:
			return new BLong(expanded_statbar.tithing);
		case MBR_FOLLOWERS:
			return new BLong(expanded_statbar.followers);
		case MBR_FIRE_DAMAGE: 
			return new BLong(element_damage.fire); 
			break;
		case MBR_COLD_DAMAGE: 
			return new BLong(element_damage.cold); 
			break;
		case MBR_ENERGY_DAMAGE: 
			return new BLong(element_damage.energy); 
			break;
		case MBR_POISON_DAMAGE: 
			return new BLong(element_damage.poison); 
			break;
		case MBR_PHYSICAL_DAMAGE: 
			return new BLong(element_damage.physical); 
			break;
		case MBR_FIRE_DAMAGE_MOD: 
			return new BLong(element_damage_mod.fire); 
			break;
		case MBR_COLD_DAMAGE_MOD: 
			return new BLong(element_damage_mod.cold); 
			break;
		case MBR_ENERGY_DAMAGE_MOD: 
			return new BLong(element_damage_mod.energy); 
			break;
		case MBR_POISON_DAMAGE_MOD: 
			return new BLong(element_damage_mod.poison); 
			break;
		case MBR_PHYSICAL_DAMAGE_MOD: 
			return new BLong(element_damage_mod.physical); 
			break;
		case MBR_PARTY:
			if (party_ != NULL)
				return CreatePartyRefObjImp( party_ );
			else
				return new BError( "Not a member of a party" );
			break;
		case MBR_PARTYLOOT:
			return new BLong(party_can_loot());
		case MBR_CANDIDATE_OF_PARTY:
			if (candidate_of_ != NULL)
				return CreatePartyRefObjImp( candidate_of_ );
			else
				return new BError( "Not a candidate of a party" );
			break;
		case MBR_MOVECOST_WALK:
			return new Double( movement_cost.walk );
			break;
		case MBR_MOVECOST_RUN:
			return new Double( movement_cost.run );
			break;
		case MBR_MOVECOST_WALK_MOUNTED:
			return new Double( movement_cost.walk_mounted );
			break;
		case MBR_MOVECOST_RUN_MOUNTED:
			return new Double( movement_cost.run_mounted );
			break;

		case MBR_AGGRESSORTO:  return GetAggressorTo(); break;

		case MBR_LAWFULLYDAMAGED: return GetLawFullyDamaged(); break;

		case MBR_UO_EXPANSION_CLIENT:
			if (client != NULL)
				return new BLong( client->UOExpansionFlagClient );
			else
				return new BLong(0);
			break;

		default: 
			return NULL;
	}
}

BObjectImp* Character::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* Character::set_script_member_id( const int id, const std::string& value )
{
	BObjectImp* imp = base::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;

	String* ret;
	switch(id)
	{
		case MBR_TITLE_PREFIX: ret = new String( title_prefix = value ); break;
		case MBR_TITLE_SUFFIX: ret = new String( title_suffix = value ); break;
		case MBR_TITLE_GUILD: ret = new String( title_guild = value ); break;
		case MBR_TITLE_RACE: ret = new String( title_race = value ); break;
		default: return NULL;
	}
    set_dirty();
	increv();
	if (this->has_active_client())
		send_object_cache_to_inrange(this);
	return ret;
}

BObjectImp* Character::set_script_member( const char *membername, const std::string& value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* Character::set_script_member_id( const int id, long value )
{
	BObjectImp* imp = base::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_GENDER:
			if (value)
				gender = GENDER_FEMALE;
			else
				gender = GENDER_MALE;
			return new BLong( gender );
		case MBR_RACE:
			if (value == RACE_ELF)
				race = RACE_ELF;
			else
				race = RACE_HUMAN;
			return new BLong( race );
		case MBR_TRUEOBJTYPE:
			return new BLong( trueobjtype = static_cast<unsigned short>(value) );
		case MBR_TRUECOLOR:
			return new BLong( truecolor = static_cast<unsigned short>(value) );
		case MBR_AR_MOD:
			ar_mod(static_cast<short>(value));
			refresh_ar();
			return new BLong( ar_mod() );
		case MBR_DELAY_MOD:
			return new BLong( delay_mod_ = static_cast<short>(value) );
		case MBR_HIDDEN:
            {
			    //FIXME: don't call on_change unless the value actually changed?
			    bool oldhidden = hidden_;
			    hidden_ = value?true:false;
			    if(oldhidden != hidden_)
				    HiddenUpdater::on_change( this );
			    return new BLong( hidden_ );
            }
		case MBR_CONCEALED:
			concealed_ = static_cast<unsigned char>(value);
			ConcealedUpdater::on_change( this );
			return new BLong( concealed_ );
		case MBR_FROZEN:
			return new BLong( frozen_ = value?true:false );
		case MBR_PARALYZED:
			return new BLong( paralyzed_ = value?true:false );
		case MBR_POISONED:
			poisoned = value?true:false;
			on_poison_changed();
			return new BLong( poisoned );
		case MBR_STEALTHSTEPS:
			return new BLong( stealthsteps_ = static_cast<unsigned short>(value) );
		case MBR_CMDLEVEL:
			cmdlevel = static_cast<unsigned char>(value);
			if (cmdlevel >= cmdlevels2.size())
				cmdlevel = static_cast<unsigned char>(cmdlevels2.size())-1;
			ConcealedUpdater::on_change( this );
			return new BLong( cmdlevel );
		case MBR_MURDERER:
			return new BLong( murderer_ = value?true:false );
		case MBR_HITCHANCE_MOD:
			return new BLong( hitchance_mod_ = static_cast<short>(value) );
		case MBR_EVASIONCHANCE_MOD:
			return new BLong( evasionchance_mod_ = static_cast<short>(value) );
		case MBR_FACING:
			if(!face(static_cast<UFACING>(value & PKTIN_02_FACING_MASK), 0))
				return new BLong(0);
			on_facing_changed();
			return new BLong(1);
		case MBR_FIRE_RESIST_MOD: 
			element_resist_mod.fire = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_resist_mod.fire );
			break;
		case MBR_COLD_RESIST_MOD: 
			element_resist_mod.cold = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_resist_mod.cold );
			break;
		case MBR_ENERGY_RESIST_MOD: 
			element_resist_mod.energy = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_resist_mod.energy );
			break;
		case MBR_POISON_RESIST_MOD: 
			element_resist_mod.poison = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_resist_mod.poison );
			break;
		case MBR_PHYSICAL_RESIST_MOD: 
			element_resist_mod.physical = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_resist_mod.physical );
			break;
		case MBR_STATCAP: 
			expanded_statbar.statcap = static_cast<short>(value);
			if ( !this->isa(UObject::CLASS_NPC) )
				on_aos_ext_stat_changed();
			return new BLong( expanded_statbar.statcap );
			break;
		case MBR_SKILLCAP: 
			return new BLong(skillcap_ = static_cast<short>(value)); 
			break;
		case MBR_LUCK: 
			expanded_statbar.luck = static_cast<short>(value);
			if ( !this->isa(UObject::CLASS_NPC) )
				on_aos_ext_stat_changed();
			return new BLong( expanded_statbar.luck ); 
			break;
		case MBR_FOLLOWERSMAX: 
			expanded_statbar.followers_max = static_cast<u8>(value);
			if ( !this->isa(UObject::CLASS_NPC) )
				on_aos_ext_stat_changed();
			return new BLong( expanded_statbar.followers_max ); 
			break;
		case MBR_TITHING: 
			expanded_statbar.tithing = static_cast<short>(value);
			if ( !this->isa(UObject::CLASS_NPC) )
				on_aos_ext_stat_changed();
			return new BLong( expanded_statbar.tithing ); 
			break;
		case MBR_FOLLOWERS: 
			expanded_statbar.followers = static_cast<u8>(value);
			if ( !this->isa(UObject::CLASS_NPC) )
				on_aos_ext_stat_changed();
			return new BLong( expanded_statbar.followers ); 
			break;
		case MBR_FIRE_DAMAGE_MOD: 
			element_damage_mod.fire = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_damage_mod.fire );
			break;
		case MBR_COLD_DAMAGE_MOD: 
			element_damage_mod.cold = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_damage_mod.cold );
			break;
		case MBR_ENERGY_DAMAGE_MOD: 
			element_damage_mod.energy = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_damage_mod.energy );
			break;
		case MBR_POISON_DAMAGE_MOD: 
			element_damage_mod.poison = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_damage_mod.poison );
			break;
		case MBR_PHYSICAL_DAMAGE_MOD: 
			element_damage_mod.physical = static_cast<short>(value);
			refresh_ar();
			return new BLong( element_damage_mod.physical );
			break;
		case MBR_MOVECOST_WALK: return new Double( (movement_cost.walk = double(value)) ); break;
		case MBR_MOVECOST_RUN: return new Double( (movement_cost.run = double(value)) ); break;
		case MBR_MOVECOST_WALK_MOUNTED: return new Double( (movement_cost.walk_mounted = double(value)) ); break;
		case MBR_MOVECOST_RUN_MOUNTED: return new Double( (movement_cost.run_mounted = double(value)) ); break;
		default:
			return NULL;
	}
}

BObjectImp* Character::set_script_member_id_double( const int id, double value )
{
	switch(id)
	{
		case MBR_MOVECOST_WALK: return new Double( (movement_cost.walk = double(value)) ); break;
		case MBR_MOVECOST_RUN: return new Double( (movement_cost.run = double(value)) ); break;
		case MBR_MOVECOST_WALK_MOUNTED: return new Double( (movement_cost.walk_mounted = double(value)) ); break;
		case MBR_MOVECOST_RUN_MOUNTED: return new Double( (movement_cost.run_mounted = double(value)) ); break;
		default: return NULL;
	}

}

BObjectImp* Character::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* Character::script_method_id( const int id, Executor& ex )
{
	BObjectImp* imp = base::script_method_id( id, ex );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		/*
		mobile.SetPoisoned( ispoisoned := 1 )
		 If the poisoned flag was changed, and the script has a controller
			 If poisoned was SET,
				 apply RepSystem rules (Mobile damages Mobile)
			 else poisoned was CLEARED, so
				 apply RepSystem rules (Mobile helps Mobile)
		*/
	case MTH_SETPOISONED:
        {
            bool newval=true;
		    if (ex.hasParams(1))
		    {
			    long lval;
			    if (!ex.getParam( 0, lval ))
				    return new BError( "Invalid parameter type" );
			    if (!lval)
				    newval = false;
		    }

		    if (newval != poisoned)
		    {
			    set_dirty();
			    poisoned = newval;
			    check_undamaged();
			    UOExecutorModule* uoexec = static_cast<UOExecutorModule*>(ex.findModule( "UO" ));
			    if (uoexec && uoexec->controller_.get())
			    {
				    Character* attacker = uoexec->controller_.get();
				    if (!attacker->orphan())
				    {
					    if (poisoned)
						    attacker->repsys_on_damage( this );
					    else
						    attacker->repsys_on_help( this );
				    }
			    }
			    on_poison_changed();
		    }

		    return new BLong(1);
        }

		/*
		 mobile.SetParalyzed( isparalyzed := 1 )
			 If the paralyzed flag was changed, and the script has a controller
				 if paralyzed was SET,
					 apply RepSystem rules (Mobile damages Mobile)
				 else paralyzed was CLEARED, so
					 apply RepSystem rules (Mobile heals Mobile)
		*/
	case MTH_SETPARALYZED:
        {
		    bool newval = true;
		    if (ex.hasParams(1))
		    {
			    long lval;
			    if (!ex.getParam( 0, lval ))
				    return new BError( "Invalid parameter type" );
			    if (!lval)
				    newval = false;
		    }

		    if (newval != paralyzed_)
		    {
			    set_dirty();
			    paralyzed_ = newval;
			    check_undamaged();
			    UOExecutorModule* uoexec = static_cast<UOExecutorModule*>(ex.findModule( "UO" ));
			    if (uoexec && uoexec->controller_.get())
			    {
				    Character* attacker = uoexec->controller_.get();
				    if (!attacker->orphan())
				    {
					    if (paralyzed_)
						    attacker->repsys_on_damage( this );
					    else
						    attacker->repsys_on_help( this );
				    }
			    }
		    }

		    return new BLong(1);
        }

		/*
		 mobile.SetCriminal( level := 1 )
		   if level is 0, clears the CriminalTimer
		*/
	case MTH_SETCRIMINAL:
        {
		    long level = 1;
		    if (ex.hasParams(1))
		    {
			    if (!ex.getParam( 0, level ))
				    return new BError( "Invalid parameter type" );
			    if (level < 0)
				    return new BError( "Level must be >= 0" );
		    }
		    set_dirty();
		    make_criminal( level );
		    return new BLong(1);
        }

	case MTH_SETLIGHTLEVEL:
        {
            long level,duration;
		    if (!ex.hasParams(2))
			    return new BError( "Not enough parameters" );
		    if (ex.getParam( 0, level ) &&
			    ex.getParam( 1, duration ))
		    {
			    lightoverride = level;
			    lightoverride_until = read_gameclock() + duration;
			    check_region_changes();
			    return new BLong(1);
		    }
        }

	case MTH_SQUELCH:
        {
            long duration;
		    if (!ex.hasParams(1))
			    return new BError( "Not enough parameters" );
		    if (ex.getParam( 0, duration ))
		    {
			    set_dirty();
			    if (duration == -1)
				    squelched_until = ~0u;
			    else if (duration == 0)
				    squelched_until = 0;
			    else
				    squelched_until = read_gameclock() + duration;
			    return new BLong(1);
		    }
		    break;
        }
	case MTH_ENABLE:
        {
		    if (!ex.hasParams(1))
			    return new BError( "Not enough parameters" );
            const String* pstr;
		    if (ex.getStringParam( 0, pstr ))
		    {
			    if (has_privilege( pstr->data() ))
			    {
				    set_dirty();
				    set_setting( pstr->data(), true );
				    // Run the Priv Updater based on the priv
				    //PrivUpdater::on_enable( this, pstr->data() );
				    return new BLong(1);
			    }
			    else
			    {
				    return new BError( "Mobile doesn't have that privilege" );
			    }
		    }
        }

	case MTH_DISABLE:
        {
		    if (!ex.hasParams(1))
			    return new BError( "Not enough parameters" );
            const String* pstr;
		    if (ex.getStringParam( 0, pstr ))
		    {
			    set_dirty();
			    set_setting( pstr->data(), false );
			    return new BLong(1);
		    }
        }

	case MTH_ENABLED:
        {
		    if (!ex.hasParams(1))
			    return new BError( "Not enough parameters" );
            const String* pstr;
		    if (ex.getStringParam( 0, pstr ))
		    {
			    return new BLong( setting_enabled( pstr->data() ) ? 1 : 0);
		    }
        }

	case MTH_PRIVILEGES:
	{
		BDictionary* dict = new BDictionary;
		ISTRINGSTREAM istrm(all_privs());
		string tmp;
		while ( istrm >> tmp )
		{
			dict->addMember(new String (tmp), new BLong(setting_enabled(tmp.c_str())));
		}
		return dict;
		break;
	}

	case MTH_SETCMDLEVEL:
        {
		    if (!ex.hasParams(1))
			    return new BError( "Not enough parameters" );
            const String* pstr;
		    if (ex.getStringParam( 0, pstr ))
		    {
			    CmdLevel* pcmdlevel = find_cmdlevel( pstr->data() );
			    if (pcmdlevel)
			    {
				    set_dirty();
				    cmdlevel = static_cast<unsigned char>(pcmdlevel->cmdlevel);
				    return new BLong(1);
			    }
			    else
			    {
				    return new BError( "No such command level" );
			    }
		    }
		    break;
        }
	case MTH_SPENDGOLD:
        {
            long amt;
		    if (ex.numParams()!=1 ||
			    !ex.getParam( 0, amt ))
			    return new BError( "Invalid parameter type" );

		    if (gold_carried() < static_cast<unsigned long>(amt))
			    return new BError( "Insufficient funds" );

		    spend_gold( amt );
		    return new BLong( 1 );
        }

	case MTH_SETMURDERER:
        {
            long lnewval = 1;
		    if (ex.hasParams(1))
		    {
			    if (!ex.getParam( 0, lnewval ))
				    return new BError( "Invalid parameter type" );
		    }
		    set_dirty();
		    make_murderer( lnewval?true:false );
		    return new BLong(1);
        }
	case MTH_REMOVEREPORTABLE:
        {
		    if (!ex.hasParams(2))
			    return new BError( "Not enough parameters" );
            long serial, gameclock;
		    if (ex.getParam( 0, serial ) &&
			    ex.getParam( 1, gameclock ))
		    {
			    set_dirty();
			    clear_reportable( serial, gameclock );
			    return new BLong(1);
		    }
		    else
		    {
			    return new BError( "Invalid parameter type" );
		    }
		    break;
        }
	case MTH_GETGOTTENITEM:
		if( gotten_item != NULL )
			return new EItemRefObjImp(gotten_item);
		else
			return new BError( "Gotten Item NULL" );
		break;
	case MTH_CLEARGOTTENITEM:
		if( gotten_item != NULL )
		{
			clear_gotten_item();
			return new BLong(1);
		}
		else
			return new BError( "No Gotten Item" );
		break;
	case MTH_SETWARMODE:
        {
		    long newmode;
		    if (!ex.hasParams(1))
			    return new BError( "Not enough parameters" );
		    if(ex.getParam(0, newmode,0,1))
		    {
			    set_warmode( (newmode==0) ? false : true );
			    // FIXME: Additional checks needed?
			    if(client)
				    send_warmode();
			    return new BLong( warmode );
		    }
		    else
		    {
			    return new BError("Invalid parameter type");
		    }
		    break;
        }
	case MTH_GETCORPSE:
		{
		UCorpse* corpse_obj = static_cast<UCorpse*>(system_find_item(last_corpse));
		if ( corpse_obj != NULL && !corpse_obj->orphan() )
			return new EItemRefObjImp(corpse_obj);
		else
			return new BError("No corpse was found.");
		}
		break;
	case MTH_SET_SWINGTIMER:
        {
		    long time;
		    if (!ex.hasParams(1))
			    return new BError( "Not enough parameters" );
		    if (ex.getParam(0,time))
		    {
			    if (time < 0)
				    return new BError( "Time must be >= 0" );
			    polclock_t clocks;
			    clocks = (time * POLCLOCKS_PER_SEC) / 1000;
			    return new BLong( manual_set_swing_timer(clocks) ? 1 : 0);
		    }
		    else
			    return new BError( "Invalid parameter type" );
		    break;
        }
	case MTH_ATTACK_ONCE:
        {
		    Character* chr;
            if (ex.hasParams(1))
		    {
		      if (getCharacterParam( ex, 0, chr ))
		      {
			      if (dead_)
				      return new BError("Character is dead");
			      if (is_attackable(chr))
				      attack(chr);
			      else
				      return new BError("Opponent is not attackable");
		      }
		      else 
			      return new BError("Invalid parameter type");
		    }
		    else
		    {
			    chr=get_attackable_opponent();
			    if (chr != NULL) 
			    {
				    if (!dead_)
					    attack(chr);
				    else
					    return new BError("Character is dead");
			    }
			    else
				    return new BError("No opponent");
		    }
		    return new BLong(1);
		    break;
        }
	case MTH_KILL:
        if (ex.hasParams(1))
		{
			Character* chr;
			if (getCharacterParam( ex, 0, chr ))
				chr->repsys_on_damage( this );
		}
		die();
		return new BLong(1);
		break;
	case MTH_SETFACING:
		{
			long flags = 0;
			UFACING i_facing;

			if(ex.hasParams(2) && !ex.getParam(1, flags, 0, 1))
				return new BError( "Invalid flags for parameter 1" );

			BObjectImp* param0 = ex.getParamImp(0);
			if (param0->isa( BObjectImp::OTString ))
			{
				const char* szDir = ex.paramAsString(0);
				if (DecodeFacing( szDir, i_facing ) == false)
					return new BError( "Invalid string for parameter 0" );
			}
			else if (param0->isa( BObjectImp::OTLong ))
			{
				BLong* blong = static_cast<BLong*>(param0);
				i_facing = static_cast<UFACING>(blong->value() & PKTIN_02_FACING_MASK);
			}
			else
				return new BError( "Invalid type for parameter 0" );

			if(!face(i_facing, flags))
				return new BLong(0);
			
			on_facing_changed();
			return new BLong(1);
		}
		break;
	case MTH_COMPAREVERSION:
		if (client!=NULL)
		{
			if (!ex.hasParams(1))
				return new BError( "Not enough parameters" );
            const String* pstr;
			if (ex.getStringParam( 0, pstr ))
				return new BLong(client->compareVersion(pstr->getStringRep()) ? 1:0);
			else
				return new BError( "Invalid parameter type" );
		}
		else
			return new BError( "No client attached" );
		break;
	case MTH_SETAGGRESSORTO:
		if (ex.hasParams(1))
		{
			Character* chr;
			if (getCharacterParam( ex, 0, chr ))
			{
				this->make_aggressor_to(chr);
				return new BLong(1);
			}
			else
				return new BError("Invalid parameter type");
		}
		else
			return new BError( "Not enough parameters" );
		break;
	case MTH_SETLAWFULLYDAMAGEDTO:
		if (ex.hasParams(1))
		{
			Character* chr;
			if (getCharacterParam( ex, 0, chr ))
			{
				this->make_lawfullydamaged_to(chr);
				return new BLong(1);
			}
			else
				return new BError("Invalid parameter type");
		}
		else
			return new BError( "Not enough parameters" );
		break;
	case MTH_CLEARAGGRESSORTO:
		if (ex.hasParams(1))
		{
			Character* chr;
			if (getCharacterParam( ex, 0, chr ))
			{
				this->remove_as_aggressor_to(chr);
				return new BLong(1);
			}
			else
				return new BError("Invalid parameter type");
		}
		else
			return new BError( "Not enough parameters" );
		break;
	case MTH_CLEARLAWFULLYDAMAGEDTO:
		if (ex.hasParams(1))
		{
			Character* chr;
			if (getCharacterParam( ex, 0, chr ))
			{
				this->remove_as_lawful_damager(chr);
				return new BLong(1);
			}
			else
				return new BError("Invalid parameter type");
		}
		else
			return new BError( "Not enough parameters" );
		break;
	default:
		return NULL;
	}
	return new BError( "Invalid parameter type" );
}


BObjectImp* Character::script_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->script_method_id(objmethod->id, ex);	
	else
		return NULL;
}

ObjArray* Character::GetReportables() const
{
	ObjArray* arr = new ObjArray;

	for( ReportableList::const_iterator itr = reportable_.begin();
			itr != reportable_.end(); ++itr )
	{
		const reportable_t& rt = (*itr);

		BObjectImp* kmember = NULL;
		Character* killer = system_find_mobile( rt.serial );
		if (killer)
		{
			kmember = new EOfflineCharacterRefObjImp(killer);
		}
		else
		{
			kmember = new BError("Mobile not found");
		}

		BStruct* elem = new BStruct;;
		elem->addMember( "serial", new BLong( rt.serial ) );
		elem->addMember( "killer", kmember );
		elem->addMember( "gameclock", new BLong( rt.polclock ) );

		arr->addElement( elem );
	}
	return arr;
}

ObjArray* Character::GetAggressorTo() const
{
	ObjArray* arr = new ObjArray;

	for( Character::MobileCont::const_iterator itr = aggressor_to_.begin();
			itr != aggressor_to_.end(); ++itr )
	{
		BObjectImp* member = NULL;
		Character* chr = system_find_mobile( (*itr).first->serial);
		if (chr)
		{
			member = new EOfflineCharacterRefObjImp(chr);
		}
		else
		{
			member = new BError("Mobile not found");
		}

		BStruct* elem = new BStruct;;
		elem->addMember( "serial", new BLong( (*itr).first->serial ) );
		elem->addMember( "ref", member );
		elem->addMember( "seconds", new BLong( ( (*itr).second - polclock() ) / POLCLOCKS_PER_SEC ) );

		arr->addElement( elem );
	}
	return arr;
}

ObjArray* Character::GetLawFullyDamaged() const
{
	ObjArray* arr = new ObjArray;

	for( Character::MobileCont::const_iterator itr = lawfully_damaged_.begin();
			itr != lawfully_damaged_.end(); ++itr )
	{
		BObjectImp* member = NULL;
		Character* chr = system_find_mobile( (*itr).first->serial);
		if (chr)
		{
			member = new EOfflineCharacterRefObjImp(chr);
		}
		else
		{
			member = new BError("Mobile not found");
		}

		BStruct* elem = new BStruct;;
		elem->addMember( "serial", new BLong( (*itr).first->serial ) );
		elem->addMember( "ref", member );
		elem->addMember( "seconds", new BLong( ( (*itr).second - polclock() ) / POLCLOCKS_PER_SEC ) );

		arr->addElement( elem );
	}
	return arr;
}

BObjectImp* NPC::get_script_member_id( const int id ) const
{
	BObjectImp* imp = base::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	Character* master = NULL;
	switch(id)
	{
		case MBR_SCRIPT: return new String(script); break;
		case MBR_NPCTEMPLATE: return new String( template_name ); break;
		case MBR_MASTER:
			master = master_.get();
			if (master && !master->orphan())
				return new ECharacterRefObjImp( master );
			else
				return new BLong(0);
			break;

		case MBR_PROCESS:
			if (ex)
				return new ScriptExObjImp( ex );
			else
				return new BError( "No script running" );
			break;

		case MBR_EVENTMASK:
			if (ex)
				return new BLong( ex->eventmask );
			else
				return new BError( "No script running" );
			break;

		case MBR_SPEECH_COLOR: return new BLong( speech_color_ ); break;
		case MBR_SPEECH_FONT:  return new BLong( speech_font_ ); break;
		case MBR_USE_ADJUSTMENTS:  return new BLong( use_adjustments ? 1 : 0 ); break;
		case MBR_RUN_SPEED: return new BLong( run_speed ); break;
		case MBR_ALIGNMENT:
			return new BLong(this->template_.alignment);
		case MBR_SAVEONEXIT: return new BLong( saveonexit()); break;
		default: return NULL;
	}
}

BObjectImp* NPC::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* NPC::set_script_member_id( const int id, const std::string& value )
{
	BObjectImp* imp = base::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;
	switch(id)
	{
		case MBR_SCRIPT: return new String( script = value );
		default: return NULL;
	}
}

BObjectImp* NPC::set_script_member( const char *membername, const std::string& value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* NPC::set_script_member_id( const int id, long value )
{
	BObjectImp* imp = base::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;
	switch(id)
	{
		case MBR_SPEECH_COLOR: return new BLong( speech_color_ = static_cast<unsigned short>(value) );
		case MBR_SPEECH_FONT: return new BLong( speech_font_ = static_cast<unsigned short>(value) );
		case MBR_USE_ADJUSTMENTS: return new BLong( use_adjustments = value?true:false );
		case MBR_RUN_SPEED: return new BLong( run_speed = static_cast<unsigned short>(value) );
		case MBR_SAVEONEXIT:
			saveonexit(value?true:false);
			return new BLong( saveonexit() );
		default: return NULL;
	}
}
BObjectImp* NPC::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* NPC::script_method_id( const int id, Executor& ex )
{
	BObjectImp* imp = base::script_method_id( id, ex );
	if (imp != NULL)
		return imp;

	switch(id)
	{
	case MTH_SETMASTER:
		if (ex.numParams()!=1)
			return new BError( "Not enough parameters" );
		Character* chr;
		set_dirty();
		if (getCharacterParam( ex, 0, chr ))
		{
			master_.set( chr );
			return new BLong(1);
		}
		else
		{
			master_.clear();
			return new BLong(0);
		}
		break;
	default:
		return NULL;
	}
}

BObjectImp* NPC::script_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->script_method_id(objmethod->id, ex);
	else
		return NULL;
}

BObjectImp* Item::make_ref()
{
	return new EItemRefObjImp( this );
}

BObjectImp* ULockable::get_script_member_id( const int id ) const
{
	BObjectImp* imp = Item::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_LOCKED: return new BLong( locked_ ? 1 : 0 ); break;
		default: return NULL;
	}
}

BObjectImp* ULockable::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* ULockable::set_script_member_id( const int id, long value )
{
	BObjectImp* imp = Item::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;
	switch(id)
	{
		case MBR_LOCKED: return new BLong( locked_ = value?true:false );
		default: return NULL;
	}
}
BObjectImp* ULockable::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* UCorpse::get_script_member_id( const int id ) const
{
	BObjectImp* imp = base::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_CORPSETYPE:  return new BLong(corpsetype); break;
		case MBR_OWNERSERIAL:	return new BLong(ownerserial); break;
		default: return NULL;
	}
}

BObjectImp* UCorpse::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* Spellbook::script_method_id( const int id, Executor& ex )
{
    BObjectImp* imp = base::script_method_id( id, ex );
    if (imp != NULL)
        return imp;

    switch(id)
    {
    case MTH_HASSPELL:
        {
            long id;
            if (!ex.hasParams(1))
                return new BError( "Not enough parameters" );
            if (ex.getParam(0,id))
            {
                if (id < 0)
                    return new BError( "SpellID must be >= 0" );
                u8 school = static_cast<u8>(id / 100);
                if ((school == this->spell_school) && (this->has_spellid(static_cast<unsigned long>(id))))
                    return new BLong(1);
                else
                    return new BLong(0);
            }
            else
                return new BError( "Invalid parameter type" );
            break;
        }
    case MTH_SPELLS:
        {
            ObjArray* arr = new ObjArray;
            for ( u16 i = 0; i < 64; ++i )
            {
                unsigned long id = this->spell_school + i;
                if (this->has_spellid(id))
                    arr->addElement(new BLong(id));
            }
            return arr;
            break;
        }

    default:
        return NULL;
    }
    return new BLong(1);
}

BObjectImp* Spellbook::script_method( const char* methodname, Executor& ex )
{
    ObjMethod* objmethod = getKnownObjMethod(methodname);
    if ( objmethod != NULL )
        return this->script_method_id(objmethod->id, ex);
    else
        return NULL;
}

BObjectImp* UBoat::make_ref()
{
	return new EUBoatRefObjImp( this );
}

BObjectImp* UMulti::make_ref()
{
	return new EMultiRefObjImp( this );
}

BObjectImp* UBoat::get_script_member_id( const int id ) const
{
	BObjectImp* imp = base::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	Item* cp = NULL;
	switch(id)
	{
		case MBR_TILLERMAN:
			cp  = components_[0].get();
			if ( cp != NULL)
				return new EItemRefObjImp( cp );
			else
				return new BError( string("This ship doesn't have that component") );
			break;
		case MBR_PORTPLANK:
			cp  = components_[1].get();
			if ( cp != NULL)
				return new EItemRefObjImp( cp );
			else
				return new BError( string("This ship doesn't have that component") );
			break;
		case MBR_STARBOARDPLANK:
			cp  = components_[2].get();
			if ( cp != NULL)
				return new EItemRefObjImp( cp );
			else
				return new BError( string("This ship doesn't have that component") );
			break;
		case MBR_HOLD:
			cp  = components_[3].get();
			if ( cp != NULL)
				return new EItemRefObjImp( cp );
			else
				return new BError( string("This ship doesn't have that component") );
			break;
		case MBR_ITEMS: return items_list(); break;
		case MBR_MOBILES: return mobiles_list(); break;
		case MBR_HAS_OFFLINE_MOBILES:  return new BLong( has_offline_mobiles() ? 1 : 0 ); break;
		default: return NULL;
	}
}

BObjectImp* UBoat::get_script_member( const char* membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* UBoat::script_method_id( const int id, Executor& ex )
{
	BObjectImp* imp = base::script_method_id( id, ex );
	if (imp != NULL)
		return imp;

	switch(id)
	{
	case MTH_MOVE_OFFLINE_MOBILES:
		xcoord x;
		ycoord y;
		zcoord z;
		const String* strrealm;
		
		if (ex.numParams()==3)
		{
		  if (ex.getParam( 0, x ) &&
			  ex.getParam( 1, y ) &&
			  ex.getParam( 2, z, ZCOORD_MIN, ZCOORD_MAX ))
		  {
			if (!realm->valid(x,y,z))
			  return new BError( "Coordinates are out of range" );
			
			set_dirty();
			move_offline_mobiles( x, y, z, realm );
			return new BLong(1);
		  }
		  else
			return new BError( "Invalid parameter type" );
		}
		else
		  if (ex.numParams()==4)
		  {
			if (ex.getParam( 0, x ) &&
				ex.getParam( 1, y ) &&
				ex.getParam( 2, z, ZCOORD_MIN, ZCOORD_MAX ) &&
				ex.getStringParam( 3, strrealm ))
			{
			  Realm* realm = find_realm(strrealm->value());
			  if (!realm)
				return new BError( "Realm not found" );
			  
			  if (!realm->valid(x,y,z))
				return new BError( "Coordinates are out of range" );
			  
			  set_dirty();
			  move_offline_mobiles( x, y, z, realm );
			  return new BLong(1);
			}
			else
			  return new BError( "Invalid parameter type" );
		  }
		  else
			return new BError( "Not enough parameters" );
		break;
	
	default:
		return NULL;
	}
}

BObjectImp* UBoat::script_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->script_method_id(objmethod->id, ex);
	else
		return NULL;
}

BObjectImp* UPlank::get_script_member_id( const int id ) const
{
	switch(id)
	{
		case MBR_MULTI:
			if (boat_.get())
				return new EUBoatRefObjImp( boat_.get() );
			else
				return new BError( "No boat attached" );
			break;
	}
	return base::get_script_member_id( id );
}

/* UObject defines a 'multi' also, so we have to trap that here first */
BObjectImp* UPlank::get_script_member( const char* membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return base::get_script_member(membername);
}

BObjectImp* Map::get_script_member_id( const int id ) const
{
	BObjectImp* imp = base::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_XEAST: return new BLong(xeast); break;
		case MBR_XWEST: return new BLong(xwest); break;
		case MBR_YNORTH: return new BLong(ynorth); break;
		case MBR_YSOUTH: return new BLong(ysouth); break;
		case MBR_GUMPWIDTH: return new BLong(gumpwidth); break;
		case MBR_GUMPHEIGHT: return new BLong(gumpheight); break;
		default: return NULL;
	}
}

BObjectImp* Map::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* Map::set_script_member_id( const int id, long value )
{
	BObjectImp* imp = base::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;
	switch(id)
	{
		case MBR_XEAST: return new BLong( xeast = static_cast<unsigned short>(value) );
		case MBR_XWEST: return new BLong( xwest = static_cast<unsigned short>(value) );
		case MBR_YNORTH: return new BLong( ynorth = static_cast<unsigned short>(value) );
		case MBR_YSOUTH: return new BLong( ysouth = static_cast<unsigned short>(value) );
		case MBR_GUMPWIDTH: return new BLong( gumpwidth = static_cast<unsigned short>(value) );
		case MBR_GUMPHEIGHT: return new BLong( gumpheight = static_cast<unsigned short>(value) );
		default: return NULL;
	}
}
BObjectImp* Map::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* UObject::script_method_id( const int id, Executor& ex )
{
	const String* mname;
	BObjectImp* ret;

	switch(id)
	{
	case MTH_ISA:
		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );
		long isatype;
		if (!ex.getParam(0,isatype))
			return new BError( "Invalid parameter type" );
		return new BLong(script_isa( static_cast<unsigned>(isatype) ));
		break;
	case MTH_SET_MEMBER:
		if (!ex.hasParams(2))
			return new BError( "Not enough parameters" );
		BObjectImp* objimp;
		if (ex.getStringParam(0,mname) &&
			(objimp = ex.getParamImp(1)) )
		{
			if(objimp->isa( BObjectImp::OTLong ) )
			{
				BLong* lng = static_cast<BLong*>(objimp);
				ret = set_script_member(mname->value().c_str(),lng->value());
			}
			else if(objimp->isa( BObjectImp::OTDouble ) )
			{
				Double* dbl = static_cast<Double*>(objimp);
				ret = set_script_member_double(mname->value().c_str(),dbl->value());
			}
			else if(objimp->isa( BObjectImp::OTString ) )
			{
				String* str = static_cast<String*>(objimp);
				ret = set_script_member(mname->value().c_str(),str->value());
			}
			else
				return new BError( "Invalid value type" );

			if(ret != NULL)
				return ret;
			else
			{
				string message = string("Member ") + string(mname->value()) + string(" not found on that object");
				return new BError( message );
			}

		}
		else
			return new BError( "Invalid parameter type" );
		break;
	case MTH_GET_MEMBER:
		if (!ex.hasParams(1))
			return new BError( "Not enough parameters" );

		if (ex.getStringParam(0,mname))
		{
			ret = get_script_member(mname->value().c_str());
			if(ret != NULL)
				return ret;
			else
			{
				string message = string("Member ") + string(mname->value()) + string(" not found on that object");
				return new BError( message );
			}
		}
		else
			return new BError( "Invalid parameter type" );
		break;
	default:
		bool changed = false;
		BObjectImp* imp = CallPropertyListMethod_id( proplist_, id, ex, changed );
		if (changed)
			set_dirty();
		return imp;
	}
}


BObjectImp* UObject::script_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->script_method_id(objmethod->id, ex);
	else
	{
		bool changed = false;
		BObjectImp* imp = CallPropertyListMethod(proplist_, methodname, ex, changed);
		if ( changed )
			set_dirty();

		return imp;
	}
}

BObjectImp* UObject::custom_script_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->script_method_id(objmethod->id, ex);
	else
		return NULL;
}

BObjectImp* UDoor::get_script_member_id( const int id ) const
{
	BObjectImp* imp = ULockable::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_ISOPEN: return new BLong( is_open() ? 1 : 0 ); break;
		default: return NULL;
	}
}

BObjectImp* UDoor::get_script_member( const char* membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* UDoor::script_method_id( const int id, Executor& ex )
{
	BObjectImp* imp = ULockable::script_method_id( id, ex );
	if (imp != NULL)
		return imp;

	switch(id)
	{
	case MTH_OPEN:
		open(); break;
	case MTH_CLOSE:
		close(); break;
	case MTH_TOGGLE:
		toggle(); break;
	default:
		return NULL;
	}
	return new BLong(1);
}

BObjectImp* UDoor::script_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if ( objmethod != NULL )
		return this->script_method_id(objmethod->id, ex);
	else
		return NULL;
}

BObjectImp* Equipment::get_script_member_id( const int id ) const
{
	BObjectImp* imp = Item::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_QUALITY: return new Double( quality_ ); break;
		case MBR_HP: return new BLong( hp_ ); break;
		case MBR_MAXHP_MOD: return new BLong( maxhp_mod_ ); break;
		case MBR_MAXHP: return new BLong( static_cast<long>(maxhp() * quality_) ); break;
		default: return NULL;
	}
}
BObjectImp* Equipment::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* Equipment::set_script_member_id( const int id, long value )
{
	BObjectImp* imp = Item::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;
	switch(id)
	{
		case MBR_HP: 
		  hp_ = static_cast<unsigned short>(value);
			if (this->isa(CLASS_ARMOR))
			{
				if (container != NULL)
				{
					if (IsCharacter( container->serial ))
					{
						Character* chr = chr_from_wornitems( container );
						if (chr!=NULL)
							ARUpdater::on_change(chr);
					}
				}
			}
			return new BLong( hp_ );
		case MBR_MAXHP_MOD: 
		  maxhp_mod_ = static_cast<unsigned short>(value);
			if (this->isa(CLASS_ARMOR))
			{
				if (container != NULL) 
				{
					if (IsCharacter( container->serial ))
					{
						Character* chr = chr_from_wornitems( container );
						if (chr!=NULL)
							ARUpdater::on_change(chr);
					}
				}
			}
			return new BLong( maxhp_mod_ );
		case MBR_QUALITY: return new Double( (quality_ = double(value)) );
		default: return NULL;
	}
}
BObjectImp* Equipment::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* Equipment::set_script_member_id_double( const int id, double value )
{
	BObjectImp* imp = Item::set_script_member_id_double( id, value );
	if (imp != NULL)
		return imp;
	switch(id)
	{
		case MBR_QUALITY: return new Double( (quality_ = double(value)) );
		default: return NULL;
	}
}
BObjectImp* Equipment::set_script_member_double( const char *membername, double value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id_double(objmember->id, value);
	else
		return NULL;
}


BObjectImp* UWeapon::get_script_member_id( const int id ) const
{
	BObjectImp* imp = Equipment::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_DMG_MOD: return new BLong( dmg_mod_ ); break;
		case MBR_ATTRIBUTE: return new String( attribute().name ); break;
		case MBR_HITSCRIPT: return new String( hit_script_.relativename( tmpl->pkg ) ); break;
		case MBR_INTRINSIC:
			return new BLong(is_intrinsic()); break;
		default: return NULL;
	}
}
BObjectImp* UWeapon::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* UWeapon::set_script_member_id( const int id, const std::string& value )
{
	BObjectImp* imp = Item::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_HITSCRIPT: set_hit_script( value ); return new BLong(1);
		default: return NULL;
	}
}
BObjectImp* UWeapon::set_script_member( const char *membername, const std::string& value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* UWeapon::set_script_member_id( const int id, long value )
{
	if (is_intrinsic())
		return new BError("Cannot alter an instrinsic NPC weapon member values"); //executor won't return this to the script currently.

	BObjectImp* imp = Equipment::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_DMG_MOD: return new BLong( dmg_mod_ = static_cast<short>(value) );
		default: return NULL;
	}
}

BObjectImp* UWeapon::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* UWeapon::set_script_member_id_double( const int id , double value )
{
	if (is_intrinsic())
		return new BError("Cannot alter an instrinsic NPC weapon member values"); //executor won't return this to the script currently.
	return base::set_script_member_id_double(id, value);
}

BObjectImp* UWeapon::set_script_member_double( const char *membername, double value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id_double(objmember->id, value);
	else
		return base::set_script_member_double(membername, value);
}

BObjectImp* UArmor::get_script_member_id( const int id ) const 
{
	BObjectImp* imp = Equipment::get_script_member_id( id );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_AR_MOD: return new BLong(ar_mod_); break;
		case MBR_AR: return new BLong( ar() ); break;
		case MBR_AR_BASE: return new BLong( tmpl->ar ); break;
		case MBR_ONHIT_SCRIPT: return new String( onhitscript_.relativename( tmpl->pkg ) ); break;
		default: return NULL;
	}
}

BObjectImp* UArmor::get_script_member( const char *membername ) const
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->get_script_member_id(objmember->id);
	else
		return NULL;
}

BObjectImp* UArmor::set_script_member_id( const int id, const std::string& value ) 
{
	BObjectImp* imp = Item::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;
	switch(id)
	{
		case MBR_ONHIT_SCRIPT: set_onhitscript( value ); return new BLong(1);
		default: return NULL;
	}
}

BObjectImp* UArmor::set_script_member( const char *membername, const std::string& value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

BObjectImp* UArmor::set_script_member_id( const int id, long value ) 
{
	BObjectImp* imp = Equipment::set_script_member_id( id, value );
	if (imp != NULL)
		return imp;

	switch(id)
	{
		case MBR_AR_MOD: 
		  ar_mod_ = static_cast<short>(value);
			if (container != NULL) 
			{
				if (IsCharacter( container->serial ))
				{
					Character* chr = chr_from_wornitems( container );
					if (chr!=NULL)
						ARUpdater::on_change(chr);
				}
			}
			return new BLong( ar_mod_ );
		default: return NULL;
	}
}

BObjectImp* UArmor::set_script_member( const char *membername, long value )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if ( objmember != NULL )
		return this->set_script_member_id(objmember->id, value);
	else
		return NULL;
}

#include "eventid.h"

SourcedEvent::SourcedEvent( EVENTID type, Character* source )
{
	addMember( "type", new BLong( type ) );
	addMember( "source", new ECharacterRefObjImp( source ) );
}

SpeechEvent::SpeechEvent( Character* speaker, const char* speech )
{
	addMember( "type", new BLong( EVID_SPOKE ) );
	addMember( "source", new ECharacterRefObjImp( speaker ) );
	addMember( "text", new String( speech ) );
}
SpeechEvent::SpeechEvent( Character* speaker, const char* speech, const char* texttype) 
{
	addMember( "type", new BLong( EVID_SPOKE ) );
	addMember( "source", new ECharacterRefObjImp( speaker ) );
	addMember( "text", new String( speech ) );
	addMember( "texttype", new String(texttype) );
}

UnicodeSpeechEvent::UnicodeSpeechEvent( Character* speaker, const char* speech,
															const u16* wspeech, const char lang[4] )
{
	ObjArray* arr;
	addMember( "type", new BLong( EVID_SPOKE ) );
	addMember( "source", new ECharacterRefObjImp( speaker ) );
	addMember( "text", new String( speech ) );
	unsigned wlen = 0;
	while( wspeech[wlen] != L'\0' )
		++wlen;
	if ( !convertUCtoArray(wspeech, arr, wlen, true) )
		addMember( "uc_text", new BError("Invalid Unicode speech received.") );
	else {
		addMember( "uc_text", arr );
		addMember( "langcode", new String( lang ) );
	}
}
UnicodeSpeechEvent::UnicodeSpeechEvent( Character* speaker, const char* speech, const char* texttype,
															const u16* wspeech, const char lang[4] )
{
	ObjArray* arr;
	addMember( "type", new BLong( EVID_SPOKE ) );
	addMember( "source", new ECharacterRefObjImp( speaker ) );
	addMember( "text", new String( speech ) );
	unsigned wlen = 0;
	while( wspeech[wlen] != L'\0' )
		++wlen;
	if ( !convertUCtoArray(wspeech, arr, wlen, true) )
		addMember( "uc_text", new BError("Invalid Unicode speech received.") );
	else {
		addMember( "uc_text", arr );
		addMember( "langcode", new String( lang ) );
	}
	addMember( "texttype", new String(texttype) );
}

DamageEvent::DamageEvent( Character* source, unsigned short damage )
{
	addMember( "type", new BLong( EVID_DAMAGED ) );

	if (source != NULL)
		addMember( "source", new ECharacterRefObjImp( source ) );
	else
		addMember( "source", new BLong( 0 ) );

	addMember( "damage", new BLong( damage ) );
}

ItemGivenEvent::ItemGivenEvent( Character* chr_givenby, Item* item_given, NPC* chr_givento ) :
	SourcedEvent( EVID_ITEM_GIVEN, chr_givenby ),
	given_by_(NULL)
{
	addMember( "item", new EItemRefObjImp( item_given ) );

	given_time_ = read_gameclock();
	item_.set(item_given);
	cont_.set( item_given->container );
	given_by_.set(chr_givenby);

	BLong* givenby = new BLong( chr_givenby->serial );
	BLong* givento = new BLong( chr_givento->serial );
	BLong* giventime = new BLong( given_time_ );
	item_given->setprop( "GivenBy", givenby->pack() );
	item_given->setprop( "GivenTo", givento->pack() );
	item_given->setprop( "GivenTime", giventime->pack() );

}

ItemGivenEvent::~ItemGivenEvent()
{
	/* See if the item is still in the container it was in
	   This means the AI script didn't do anything with it.
	 */
	Item* item = item_.get();
	UContainer* cont = cont_.get();
	Character* chr = given_by_.get();

	std::string given_time_str;
	if (!item->getprop( "GivenTime", given_time_str ))
		given_time_str = "";

	item->eraseprop( "GivenBy" );
	item->eraseprop( "GivenTo" );
	item->eraseprop( "GivenTime" );

	BObjectImp* given_value = BObjectImp::unpack( given_time_str.c_str() );
	long gts = static_cast<BLong*>(given_value)->value();

	if (item->orphan() || cont->orphan() || chr->orphan())
		return;

	if (item->container == cont && decint( given_time_ ) == decint( gts ))
	{
		UContainer* backpack = chr->backpack();
		if (backpack != NULL && !chr->dead())
		{
			if (backpack->can_add( *item ))
			{
				cont->remove( item );
				u8 newSlot = 1;
				if ( !backpack->can_add_to_slot(newSlot) || !item->slot_index(newSlot) )
				{
					item->x = chr->x;
					item->y = chr->y;
					item->z = chr->z;
					add_item_to_world( item );
					register_with_supporting_multi( item );
					move_item( item, item->x, item->y, item->z, NULL );
					return;
				}
				backpack->add( item );
				update_item_to_inrange( item );
				return;
			}
		}
		cont->remove( item );
		item->x = chr->x;
		item->y = chr->y;
		item->z = chr->z;
		add_item_to_world( item );
		register_with_supporting_multi( item );
		move_item( item, item->x, item->y, item->z, NULL );
	}
}

bool UObject::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_OBJECT);
}

bool Item::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_ITEM) || base::script_isa(isatype);
}

bool Character::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_MOBILE) || base::script_isa(isatype);
}

bool NPC::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_NPC) || base::script_isa(isatype);
}

bool ULockable::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_LOCKABLE) || base::script_isa(isatype);
}

bool UContainer::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_CONTAINER) || base::script_isa(isatype);
}

bool UCorpse::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_CORPSE) || base::script_isa(isatype);
}

bool UDoor::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_DOOR) || base::script_isa(isatype);
}

bool Spellbook::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_SPELLBOOK) || base::script_isa(isatype);
}

bool Map::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_MAP) || base::script_isa(isatype);
}

bool UMulti::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_MULTI) || base::script_isa(isatype);
}

bool UBoat::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_BOAT) || base::script_isa(isatype);
}

bool UHouse::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_HOUSE) || base::script_isa(isatype);
}

bool Equipment::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_EQUIPMENT) || base::script_isa(isatype);
}

bool UArmor::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_ARMOR) || base::script_isa(isatype);
}

bool UWeapon::script_isa( unsigned isatype ) const
{
	return (isatype == POLCLASS_WEAPON) || base::script_isa(isatype);
}


