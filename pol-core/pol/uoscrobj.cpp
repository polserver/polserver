/** @file
 *
 * @par History
 * - 2005/01/24 Shinigami: added ObjMember character.spyonclient2 to get data from packet 0xd9 (Spy
 * on Client 2)
 * - 2005/03/09 Shinigami: Added Prop Character::Delay_Mod [ms] for WeaponDelay
 * - 2005/04/02 Shinigami: UBoat::script_method_id & UBoat::script_method - added optional realm
 * param
 * - 2005/04/04 Shinigami: Added Prop Character::CreatedAt [PolClock]
 * - 2005/08/29 Shinigami: get-/setspyonclient2 renamed to get-/setclientinfo
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2005/12/09 MuadDib:   Added uclang member for storing UC language from client.
 * - 2005/12/09 MuadDib:   Fixed ~ItemGivenEvent not returning items correctly if the script
 *                         did nothing with it. Was incorrect str/int comparision for times.
 * - 2006/05/16 Shinigami: added Prop Character.Race [RACE_* Constants] to support Elfs
 * - 2006/06/15 Austin:    Added mobile.Privs()
 * - 2007/07/09 Shinigami: added Prop Character.isUOKR [bool] - UO:KR client used?
 * - 2008/07/08 Turley:    Added character.movemode - returns the MoveMode like given in NPCDesc
 *                         Added item.stackable - Is item stackable?
 * - 2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
 * - 2009/07/20 MuadDib:   ReAdded on_facing_change() to MBR_FACING: for items.
 * - 2009/08/06 MuadDib:   Added gotten_by code for items.
 * - 2009/08/31 MuadDib:   Changed layer check on graphic change, excluding mount layer entirely.
 * - 2009/09/03 MuadDib:   Relocation of account related cpp/h
 *                         Relocation of multi related cpp/h
 * - 2009/09/06 Turley:    Removed isUOKR added ClientType
 * - 2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue)
 * virtuals.
 * - 2009/10/09 Turley:    Added spellbook.spells() & .hasspell() methods
 * - 2009/10/10 Turley:    Added spellbook.addspell() & .removespell() methods
 * - 2009/10/14 Turley:    Added char.deaf() methods & char.deafened member
 * -            MuadDib:   Squelch and Deaf members set to return the gameclock they are in effect
 * till.
 * - 2009/10/17 Turley:    Moved PrivUpdater to charactr.cpp - Tomi
 * - 2009/11/19 Turley:    lightlevel now supports endless duration - Tomi
 * - 2009/12/02 Turley:    gargoyle race support
 * - 2009/12/03 Turley:    added gargoyle flying movemode
 * - 2009/12/21 Turley:    ._method() call fix
 *                         multi support of methodscripts
 * - 2010/01/15 Turley:    (Tomi) season stuff
 * - 2010/02/03 Turley:    MethodScript support for mobiles
 * - 2011/12/13 Tomi:      Boat members MBR_COMPONENT, MBR_HULL, MBR_ROPE, MBR_SAILS, MBR_WHEEL,
 * MBR_TILLER, MBR_RUDDER, MBR_STORAGE, MBR_WEAPONSLOT
 * - 2012/02/02 Tomi:      Added boat member MBR_MULTIID
 * - 2012/03/26 Tomi:      Added MBR_LASTCOORD
 * - 2012/04/14 Tomi:      Added MBR_FACETID for new map message packet
 * - 2012/04/15 Tomi:      Added MBR_EDITABLE for maps
 * - 2012/06/02 Tomi:      Added MBR_ACTIVE_SKILL and MBR_CASTING_SPELL for characters
 */


#include "uoscrobj.h"

#include <string>

#include "../bscript/berror.h"
#include "../bscript/dict.h"
#include "../bscript/execmodl.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"
#include "../clib/stlutil.h"
#include "../plib/poltype.h"
#include "../plib/uconst.h"
#include "accounts/account.h"
#include "accounts/acscrobj.h"
#include "cmdlevel.h"
#include "containr.h"
#include "door.h"
#include "dynproperties.h"
#include "equipdsc.h"
#include "exscrobj.h"
#include "fnsearch.h"
#include "globals/network.h"
#include "globals/uvars.h"
#include "item/armor.h"
#include "item/equipmnt.h"
#include "item/item.h"
#include "item/itemdesc.h"
#include "item/weapon.h"
#include "lockable.h"
#include "mobile/charactr.h"
#include "mobile/corpse.h"
#include "mobile/npc.h"
#include "mobile/ufacing.h"
#include "module/guildmod.h"
#include "module/partymod.h"
#include "module/uomod.h"
#include "multi/boat.h"
#include "multi/boatcomp.h"
#include "multi/house.h"
#include "multi/multi.h"
#include "network/client.h"
#include "network/packethelper.h"
#include "network/packets.h"
#include "network/pktdef.h"
#include "npctmpl.h"
#include "polclass.h"
#include "polclock.h"
#include "proplist.h"
#include "realms.h"
#include "realms/realm.h"
#include "spelbook.h"
#include "statmsg.h"
#include "syshookscript.h"
#include "tooltips.h"
#include "ufunc.h"
#include "umap.h"
#include "uobject.h"
#include "uoclient.h"
#include "uoexec.h"
#include "uworld.h"

namespace Pol
{
namespace Module
{
using namespace Bscript;

BApplicObjType euboatrefobjimp_type;
BApplicObjType emultirefobjimp_type;
BApplicObjType eitemrefobjimp_type;
BApplicObjType echaracterrefobjimp_type;
BApplicObjType echaracterequipobjimp_type;
BApplicObjType storage_area_type;
BApplicObjType menu_type;
BApplicObjType eclientrefobjimp_type;

const char* ECharacterRefObjImp::typeOf() const
{
  return "MobileRef";
}
u8 ECharacterRefObjImp::typeOfInt() const
{
  return OTMobileRef;
}

BObjectImp* ECharacterRefObjImp::copy() const
{
  return new ECharacterRefObjImp( obj_.get() );
}

BObjectRef ECharacterRefObjImp::get_member_id( const int id )
{
  BObjectImp* result = obj_->get_script_member_id( id );
  if ( result != nullptr )
    return BObjectRef( result );
  return BObjectRef( UninitObject::create() );
}

BObjectRef ECharacterRefObjImp::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}

BObjectRef ECharacterRefObjImp::set_member_id( const int id, BObjectImp* value, bool /*copy*/ )
{
  BObjectImp* result = nullptr;
  if ( value->isa( BObjectImp::OTLong ) )
  {
    BLong* lng = static_cast<BLong*>( value );
    result = obj_->set_script_member_id( id, lng->value() );
  }
  else if ( value->isa( BObjectImp::OTString ) )
  {
    String* str = static_cast<String*>( value );
    result = obj_->set_script_member_id( id, str->value() );
  }
  else if ( value->isa( BObjectImp::OTDouble ) )
  {
    Double* dbl = static_cast<Double*>( value );
    result = obj_->set_script_member_id_double( id, dbl->value() );
  }
  if ( result != nullptr )
    return BObjectRef( result );
  return BObjectRef( UninitObject::create() );
}

BObjectRef ECharacterRefObjImp::set_member( const char* membername, BObjectImp* value, bool copy )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_member_id( objmember->id, value, copy );
  return BObjectRef( UninitObject::create() );
}

BObjectImp* ECharacterRefObjImp::call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                    bool forcebuiltin )
{
  // MethodScript for npcs in npc->template_ (npctmpl.h) (aka templatebased)
  //             for chars in uoclient_general (uoclient.h) (aka one global definition)
  if ( obj_->orphan() )
    return new BError( "That object no longer exists" );
  ObjMethod* mth = getObjMethod( id );
  if ( mth->overridden && !forcebuiltin )
  {
    BObjectImp* imp = obj_->custom_script_method( mth->code, ex );
    if ( imp )
      return imp;
  }
  BObjectImp* imp = obj_->script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;
  return base::call_polmethod_id( id, ex );
}

BObjectImp* ECharacterRefObjImp::call_polmethod( const char* methodname, Core::UOExecutor& ex )
{
  // MethodScript for npcs in npc->template_ (npctmpl.h) (aka templatebased)
  //             for chars in uoclient_general (uoclient.h) (aka one global definition)
  bool forcebuiltin{Executor::builtinMethodForced( methodname )};
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex, forcebuiltin );
  BObjectImp* imp = obj_->custom_script_method( methodname, ex );
  if ( imp )
    return imp;
  return base::call_polmethod( methodname, ex );
}

bool ECharacterRefObjImp::isTrue() const
{
  return ( !obj_->orphan() && obj_->logged_in() );
}

bool ECharacterRefObjImp::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &echaracterrefobjimp_type )
    {
      const ECharacterRefObjImp* chrref_imp =
          Clib::explicit_cast<const ECharacterRefObjImp*, const BApplicObjBase*>( aob );

      return ( chrref_imp->obj_->serial == obj_->serial );
    }
    else
      return false;
  }
  else if ( objimp.isa( Bscript::BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const Bscript::BBoolean&>( objimp ).isTrue();
  return false;
}

bool ECharacterRefObjImp::operator<( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &echaracterrefobjimp_type )
    {
      const ECharacterRefObjImp* chrref_imp =
          Clib::explicit_cast<const ECharacterRefObjImp*, const BApplicObjBase*>( aob );

      return ( chrref_imp->obj_->serial < obj_->serial );
    }
    else if ( aob->object_type() == &eitemrefobjimp_type )
      return true;
    return false;
  }
  return base::operator<( objimp );
}

const char* EOfflineCharacterRefObjImp::typeOf() const
{
  return "OfflineMobileRef";
}
u8 EOfflineCharacterRefObjImp::typeOfInt() const
{
  return OTOfflineMobileRef;
}

BObjectImp* EOfflineCharacterRefObjImp::copy() const
{
  return new EOfflineCharacterRefObjImp( obj_.get() );
}

bool EOfflineCharacterRefObjImp::isTrue() const
{
  return ( !obj_->orphan() );
}

const char* EItemRefObjImp::typeOf() const
{
  return "ItemRef";
}
u8 EItemRefObjImp::typeOfInt() const
{
  return OTItemRef;
}

BObjectImp* EItemRefObjImp::copy() const
{
  return new EItemRefObjImp( obj_.get() );
}

BObjectRef EItemRefObjImp::get_member_id( const int id )
{
  BObjectImp* result = obj_->get_script_member_id( id );
  if ( result != nullptr )
    return BObjectRef( result );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EItemRefObjImp::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EItemRefObjImp::set_member_id( const int id, BObjectImp* value, bool /*copy*/ )
{
  BObjectImp* result = nullptr;
  if ( value->isa( BObjectImp::OTLong ) )
  {
    BLong* lng = static_cast<BLong*>( value );
    result = obj_->set_script_member_id( id, lng->value() );
  }
  else if ( value->isa( BObjectImp::OTString ) )
  {
    String* str = static_cast<String*>( value );
    result = obj_->set_script_member_id( id, str->value() );
  }
  else if ( value->isa( BObjectImp::OTDouble ) )
  {
    Double* dbl = static_cast<Double*>( value );
    result = obj_->set_script_member_id_double( id, dbl->value() );
  }
  if ( result != nullptr )
    return BObjectRef( result );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EItemRefObjImp::set_member( const char* membername, BObjectImp* value, bool copy )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_member_id( objmember->id, value, copy );
  return BObjectRef( UninitObject::create() );
}

BObjectImp* EItemRefObjImp::call_polmethod_id( const int id, Core::UOExecutor& ex,
                                               bool forcebuiltin )
{
  if ( obj_->orphan() )
    return new BError( "That object no longer exists" );

  ObjMethod* mth = getObjMethod( id );
  if ( mth->overridden && !forcebuiltin )
  {
    BObjectImp* imp = obj_->custom_script_method( mth->code, ex );
    if ( imp )
      return imp;
  }
  BObjectImp* imp = obj_->script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;
  return base::call_polmethod_id( id, ex );
}

BObjectImp* EItemRefObjImp::call_polmethod( const char* methodname, Core::UOExecutor& ex )
{
  bool forcebuiltin{Executor::builtinMethodForced( methodname )};

  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex, forcebuiltin );
  Items::Item* item = obj_.get();
  BObjectImp* imp = item->custom_script_method( methodname, ex );
  if ( imp )
    return imp;
  return base::call_polmethod( methodname, ex );
}

bool EItemRefObjImp::isTrue() const
{
  return ( !obj_->orphan() );
}

bool EItemRefObjImp::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &eitemrefobjimp_type )
    {
      const EItemRefObjImp* itemref_imp =
          Clib::explicit_cast<const EItemRefObjImp*, const BApplicObjBase*>( aob );

      return ( itemref_imp->obj_->serial == obj_->serial );
    }
    return false;
  }
  else if ( objimp.isa( Bscript::BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const Bscript::BBoolean&>( objimp ).isTrue();
  return false;
}

bool EItemRefObjImp::operator<( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &eitemrefobjimp_type )
    {
      const EItemRefObjImp* itemref_imp =
          Clib::explicit_cast<const EItemRefObjImp*, const BApplicObjBase*>( aob );

      return ( itemref_imp->obj_->serial < obj_->serial );
    }
    return ( &eitemrefobjimp_type < aob->object_type() );
  }

  return base::operator<( objimp );
}

const char* EUBoatRefObjImp::typeOf() const
{
  return "BoatRef";
}
u8 EUBoatRefObjImp::typeOfInt() const
{
  return OTBoatRef;
}

BObjectImp* EUBoatRefObjImp::copy() const
{
  return new EUBoatRefObjImp( obj_.get() );
}

BObjectRef EUBoatRefObjImp::get_member_id( const int id )
{
  BObjectImp* result = obj_->get_script_member_id( id );
  if ( result != nullptr )
    return BObjectRef( result );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EUBoatRefObjImp::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EUBoatRefObjImp::set_member_id( const int id, BObjectImp* value, bool /*copy*/ )
{
  BObjectImp* result = nullptr;
  if ( value->isa( BObjectImp::OTLong ) )
  {
    BLong* lng = static_cast<BLong*>( value );
    result = obj_->set_script_member_id( id, lng->value() );
  }
  else if ( value->isa( BObjectImp::OTString ) )
  {
    String* str = static_cast<String*>( value );
    result = obj_->set_script_member_id( id, str->value() );
  }
  else if ( value->isa( BObjectImp::OTDouble ) )
  {
    Double* dbl = static_cast<Double*>( value );
    result = obj_->set_script_member_id_double( id, dbl->value() );
  }
  if ( result != nullptr )
    return BObjectRef( result );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EUBoatRefObjImp::set_member( const char* membername, BObjectImp* value, bool copy )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_member_id( objmember->id, value, copy );
  return BObjectRef( UninitObject::create() );
}

BObjectImp* EUBoatRefObjImp::call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                bool forcebuiltin )
{
  if ( obj_->orphan() )
    return new BError( "That object no longer exists" );

  ObjMethod* mth = getObjMethod( id );
  if ( mth->overridden && !forcebuiltin )
  {
    BObjectImp* imp = obj_->custom_script_method( mth->code, ex );
    if ( imp )
      return imp;
  }
  BObjectImp* imp = obj_->script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;
  return base::call_polmethod_id( id, ex );
}

BObjectImp* EUBoatRefObjImp::call_polmethod( const char* methodname, Core::UOExecutor& ex )
{
  bool forcebuiltin{Executor::builtinMethodForced( methodname )};
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex, forcebuiltin );
  return base::call_polmethod( methodname, ex );
}

bool EUBoatRefObjImp::isTrue() const
{
  return ( !obj_->orphan() );
}

bool EUBoatRefObjImp::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &euboatrefobjimp_type )
    {
      const EUBoatRefObjImp* boatref_imp =
          Clib::explicit_cast<const EUBoatRefObjImp*, const BApplicObjBase*>( aob );

      return ( boatref_imp->obj_->serial == obj_->serial );
    }
    return false;
  }
  else if ( objimp.isa( Bscript::BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const Bscript::BBoolean&>( objimp ).isTrue();
  return false;
}

BObjectImp* EMultiRefObjImp::call_polmethod( const char* methodname, Core::UOExecutor& ex )
{
  bool forcebuiltin{Executor::builtinMethodForced( methodname )};
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex, forcebuiltin );
  Multi::UMulti* multi = obj_.get();
  BObjectImp* imp = multi->custom_script_method( methodname, ex );
  if ( imp )
    return imp;
  return base::call_polmethod( methodname, ex );
}

BObjectImp* EMultiRefObjImp::call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                bool forcebuiltin )
{
  Multi::UMulti* multi = obj_.get();
  if ( multi->orphan() )
    return new BError( "That object no longer exists" );

  ObjMethod* mth = getObjMethod( id );
  if ( mth->overridden && !forcebuiltin )
  {
    BObjectImp* imp = multi->custom_script_method( mth->code, ex );
    if ( imp )
      return imp;
  }

  BObjectImp* imp = multi->script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;
  return base::call_polmethod_id( id, ex, forcebuiltin );
}

const char* EMultiRefObjImp::typeOf() const
{
  return "MultiRef";
}
u8 EMultiRefObjImp::typeOfInt() const
{
  return OTMultiRef;
}

BObjectImp* EMultiRefObjImp::copy() const
{
  return new EMultiRefObjImp( obj_.get() );
}

BObjectRef EMultiRefObjImp::get_member_id( const int id )
{
  BObjectImp* result = obj_->get_script_member_id( id );
  if ( result != nullptr )
    return BObjectRef( result );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EMultiRefObjImp::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EMultiRefObjImp::set_member_id( const int id, BObjectImp* value, bool /*copy*/ )
{
  BObjectImp* result = nullptr;
  if ( value->isa( BObjectImp::OTLong ) )
  {
    BLong* lng = static_cast<BLong*>( value );
    result = obj_->set_script_member_id( id, lng->value() );
  }
  else if ( value->isa( BObjectImp::OTString ) )
  {
    String* str = static_cast<String*>( value );
    result = obj_->set_script_member_id( id, str->value() );
  }
  else if ( value->isa( BObjectImp::OTDouble ) )
  {
    Double* dbl = static_cast<Double*>( value );
    result = obj_->set_script_member_id_double( id, dbl->value() );
  }
  if ( result != nullptr )
    return BObjectRef( result );
  return BObjectRef( UninitObject::create() );
}
BObjectRef EMultiRefObjImp::set_member( const char* membername, BObjectImp* value, bool copy )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_member_id( objmember->id, value, copy );
  return BObjectRef( UninitObject::create() );
}

bool EMultiRefObjImp::isTrue() const
{
  return ( !obj_->orphan() );
}

bool EMultiRefObjImp::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &emultirefobjimp_type )
    {
      const EMultiRefObjImp* multiref_imp =
          Clib::explicit_cast<const EMultiRefObjImp*, const BApplicObjBase*>( aob );

      return ( multiref_imp->obj_->serial == obj_->serial );
    }
    return false;
  }
  else if ( objimp.isa( Bscript::BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const Bscript::BBoolean&>( objimp ).isTrue();
  return false;
}
}  // namespace Module

namespace Core
{
using namespace Bscript;

BObjectImp* UObject::get_script_member_id( const int id ) const
{
  if ( orphan() )
    return new UninitObject;
  switch ( id )
  {
  case MBR_X:
    return new BLong( pos().x() );
    break;
  case MBR_Y:
    return new BLong( pos().y() );
    break;
  case MBR_Z:
    return new BLong( pos().z() );
    break;
  case MBR_NAME:
    return new String( name() );
    break;
  case MBR_OBJTYPE:
    return new BLong( objtype_ );
    break;
  case MBR_GRAPHIC:
    return new BLong( graphic );
    break;
  case MBR_SERIAL:
    return new BLong( serial );
    break;
  case MBR_COLOR:
    return new BLong( color );
    break;
  case MBR_HEIGHT:
    return new BLong( height );
    break;
  case MBR_FACING:
    return new BLong( facing );
    break;
  case MBR_DIRTY:
    return new BLong( dirty() ? 1 : 0 );
    break;
  case MBR_WEIGHT:
    return new BLong( weight() );
    break;
  case MBR_MULTI:
  {
    Multi::UMulti* multi = this->supporting_multi();
    if ( multi != nullptr )
      return multi->make_ref();
    else
      return new BLong( 0 );
  }
  break;
  case MBR_REALM:
    if ( toplevel_pos().realm() != nullptr )
      return new String( toplevel_pos().realm()->name() );
    else
      return new BError( "object does not belong to a realm." );
    break;
  case MBR_SPECIFIC_NAME:
    return new BLong( specific_name() );
  default:
    return nullptr;
  }
}

BObjectImp* UObject::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* UObject::set_script_member_id( const int id, const std::string& value )
{
  if ( orphan() )
    return new UninitObject;

  set_dirty();
  switch ( id )
  {
  case MBR_NAME:
    if ( ismobile() && ( value.empty() || isspace( value[0] ) ) )
      return new BError( "mobile.name must not be empty, and must not begin with a space" );
    setname( value );
    return new String( name() );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* UObject::set_script_member( const char* membername, const std::string& value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* UObject::set_script_member_id( const int id, int value )
{
  if ( orphan() )
    return new UninitObject;

  set_dirty();
  switch ( id )
  {
  case MBR_GRAPHIC:
    setgraphic( static_cast<unsigned short>( value ) );
    return new BLong( graphic );
  case MBR_COLOR:
  {
    bool res = setcolor( static_cast<unsigned short>( value ) );
    if ( !res )  // TODO log?
      return new BError( "Invalid color value " + Clib::hexint( value ) );
    return new BLong( color );
  }
  default:
    return nullptr;
  }
}

BObjectImp* UObject::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* UObject::set_script_member_id_double( const int /*id*/, double /*value*/ )
{
  set_dirty();
  return nullptr;
}

BObjectImp* UObject::set_script_member_double( const char* membername, double value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id_double( objmember->id, value );
  return nullptr;
}
}  // namespace Core

namespace Items
{
using namespace Bscript;

BObjectImp* Item::get_script_member_id( const int id ) const
{
  BObjectImp* imp = base::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_AMOUNT:
    return new BLong( amount_ );
    break;
  case MBR_LAYER:
    return new BLong( layer );
    break;
  case MBR_TILE_LAYER:
    return new BLong( tile_layer );
    break;
  case MBR_CONTAINER:
    if ( container != nullptr )
      return container->make_ref();
    return new BLong( 0 );
    break;
  case MBR_USESCRIPT:
    return new String( on_use_script_ );
    break;
  case MBR_EQUIPSCRIPT:
    return new String( equip_script_ );
    break;
  case MBR_UNEQUIPSCRIPT:
    return new String( unequip_script_ );
    break;
  case MBR_DESC:
    return new String( description() );
    break;
  case MBR_MOVABLE:
    return new BLong( movable() ? 1 : 0 );
    break;
  case MBR_INVISIBLE:
    return new BLong( invisible() ? 1 : 0 );
    break;
  case MBR_DECAYAT:
    return new BLong( decayat_gameclock_ );
    break;
  case MBR_SELLPRICE:
    return new BLong( sellprice() );
    break;
  case MBR_BUYPRICE:
    return new BLong( buyprice() );
    break;
  case MBR_NEWBIE:
    return new BLong( newbie() ? 1 : 0 );
    break;
  case MBR_INSURED:
    return new BLong( insured() ? 1 : 0 );
    break;
  case MBR_ITEM_COUNT:
    return new BLong( item_count() );
    break;
  case MBR_STACKABLE:
    return new BLong( stackable() ? 1 : 0 );
    break;
  case MBR_SAVEONEXIT:
    return new BLong( saveonexit() );
    break;
  case MBR_FIRE_RESIST:
    return new BLong( fire_resist().sum() );
    break;
  case MBR_COLD_RESIST:
    return new BLong( cold_resist().sum() );
    break;
  case MBR_ENERGY_RESIST:
    return new BLong( energy_resist().sum() );
    break;
  case MBR_POISON_RESIST:
    return new BLong( poison_resist().sum() );
    break;
  case MBR_PHYSICAL_RESIST:
    return new BLong( physical_resist().sum() );
    break;
  case MBR_LOWER_REAG_COST:
    return new BLong( lower_reagent_cost().sum() );
    break;
  case MBR_SPELL_DAMAGE_INCREASE:
    return new BLong( spell_damage_increase().sum() );
    break;
  case MBR_FASTER_CASTING:
    return new BLong( faster_casting().sum() );
    break;
  case MBR_FASTER_CAST_RECOVERY:
    return new BLong( faster_casting().sum() );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE:
    return new BLong( defence_increase().sum() );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_CAP:
    return new BLong( defence_increase_cap().sum() );
    break;
  case MBR_LOWER_MANA_COST:
    return new BLong( lower_mana_cost().sum() );
    break;
  case MBR_HIT_CHANCE:
    return new BLong( hit_chance().sum() );
    break;
  case MBR_FIRE_RESIST_CAP:
    return new BLong( fire_resist_cap().sum() );
    break;
  case MBR_COLD_RESIST_CAP:
    return new BLong( cold_resist_cap().sum() );
    break;
  case MBR_ENERGY_RESIST_CAP:
    return new BLong( energy_resist_cap().sum() );
    break;
  case MBR_POISON_RESIST_CAP:
    return new BLong( poison_resist_cap().sum() );
    break;
  case MBR_PHYSICAL_RESIST_CAP:
    return new BLong( physical_resist_cap().sum() );
    break;
  case MBR_LUCK:
    return new BLong( luck().sum() );
    break;
  case MBR_SWING_SPEED_INCREASE:
    return new BLong( swing_speed_increase().sum() );
    break;
  case MBR_FIRE_RESIST_MOD:
    return new BLong( fire_resist().mod );
    break;
  case MBR_COLD_RESIST_MOD:
    return new BLong( cold_resist().mod );
    break;
  case MBR_ENERGY_RESIST_MOD:
    return new BLong( energy_resist().mod );
    break;
  case MBR_POISON_RESIST_MOD:
    return new BLong( poison_resist().mod );
    break;
  case MBR_PHYSICAL_RESIST_MOD:
    return new BLong( physical_resist().mod );
    break;
  case MBR_FIRE_DAMAGE:
    return new BLong( fire_damage().sum() );
    break;
  case MBR_COLD_DAMAGE:
    return new BLong( cold_damage().sum() );
    break;
  case MBR_ENERGY_DAMAGE:
    return new BLong( energy_damage().sum() );
    break;
  case MBR_POISON_DAMAGE:
    return new BLong( poison_damage().sum() );
    break;
  case MBR_PHYSICAL_DAMAGE:
    return new BLong( physical_damage().sum() );
    break;
  case MBR_FIRE_DAMAGE_MOD:
    return new BLong( fire_damage().mod );
    break;
  case MBR_COLD_DAMAGE_MOD:
    return new BLong( cold_damage().mod );
    break;
  case MBR_ENERGY_DAMAGE_MOD:
    return new BLong( energy_damage().mod );
    break;
  case MBR_POISON_DAMAGE_MOD:
    return new BLong( poison_damage().mod );
    break;
  case MBR_PHYSICAL_DAMAGE_MOD:
    return new BLong( physical_damage().mod );
    break;
  case MBR_LOWER_REAG_COST_MOD:
    return new BLong( lower_reagent_cost().mod );
    break;
  case MBR_SPELL_DAMAGE_INCREASE_MOD:
    return new BLong( spell_damage_increase().mod );
    break;
  case MBR_FASTER_CASTING_MOD:
    return new BLong( faster_casting().mod );
    break;
  case MBR_FASTER_CAST_RECOVERY_MOD:
    return new BLong( faster_casting().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_MOD:
    return new BLong( defence_increase().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_CAP_MOD:
    return new BLong( defence_increase_cap().mod );
    break;
  case MBR_LOWER_MANA_COST_MOD:
    return new BLong( lower_mana_cost().mod );
    break;
  case MBR_HIT_CHANCE_MOD:
    return new BLong( hit_chance().mod );
    break;
  case MBR_FIRE_RESIST_CAP_MOD:
    return new BLong( fire_resist_cap().mod );
    break;
  case MBR_COLD_RESIST_CAP_MOD:
    return new BLong( cold_resist_cap().mod );
    break;
  case MBR_ENERGY_RESIST_CAP_MOD:
    return new BLong( energy_resist_cap().mod );
    break;
  case MBR_POISON_RESIST_CAP_MOD:
    return new BLong( poison_resist_cap().mod );
    break;
  case MBR_PHYSICAL_RESIST_CAP_MOD:
    return new BLong( physical_resist_cap().mod );
    break;
  case MBR_LUCK_MOD:
    return new BLong( luck().mod );
    break;
  case MBR_SWING_SPEED_INCREASE_MOD:
    return new BLong( swing_speed_increase().mod );
    break;

  case MBR_GETGOTTENBY:
    if ( has_gotten_by() )
      return new Module::ECharacterRefObjImp( gotten_by() );
    return new BError( "Gotten By nullptr" );
    break;
  case MBR_PROCESS:
  {
    Module::UOExecutorModule* proc = process();
    if ( proc )
    {
      Core::UOExecutor* executor = static_cast<Core::UOExecutor*>( &proc->exec );
      return new Core::ScriptExObjImp( executor );
    }
    return new BError( "No script running" );
    break;
  }
  case MBR_DOUBLECLICKRANGE:
  {
    const ItemDesc& itemdesc = this->itemdesc();
    return new BLong( itemdesc.doubleclick_range );
    break;
  }
  case MBR_QUALITY:
    return new Double( quality() );
    break;
  case MBR_HP:
    return new BLong( hp_ );
    break;
  case MBR_MAXHP_MOD:
    return new BLong( maxhp_mod() );
    break;
  case MBR_MAXHP:
    return new BLong( static_cast<int>( maxhp() * quality() ) );
    break;
  case MBR_NAME_SUFFIX:
    return new String( name_suffix() );
    break;
  case MBR_HOUSE:
    if ( house() != nullptr )
      return house()->make_ref();
    return new BError( "This is a not component of any house" );
    break;
  case MBR_NO_DROP:
    return new BLong( no_drop() );
  default:
    return nullptr;
  }
}

BObjectImp* Item::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* Item::set_script_member_id( const int id, const std::string& value )
{
  BObjectImp* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  switch ( id )
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
  case MBR_NAME_SUFFIX:
    set_dirty();
    increv();
    send_object_cache_to_inrange( this );
    name_suffix( value );
    return new String( value );

  default:
    return nullptr;
  }
}

BObjectImp* Item::set_script_member( const char* membername, const std::string& value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* Item::set_script_member_id( const int id, int value )
{
  BObjectImp* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_MOVABLE:
    restart_decay_timer();
    movable( value ? true : false );
    increv();
    return new BLong( movable() );
  case MBR_INVISIBLE:
    restart_decay_timer();
    invisible( value ? true : false );
    increv();
    return new BLong( invisible() );
  case MBR_DECAYAT:
    decayat_gameclock_ = value;
    return new BLong( decayat_gameclock_ );
  case MBR_SELLPRICE:
    sellprice( value );
    return new BLong( value );
  case MBR_BUYPRICE:
    buyprice( value );
    return new BLong( value );
  case MBR_NEWBIE:
    restart_decay_timer();
    newbie( value ? true : false );
    increv();
    return new BLong( newbie() );
  case MBR_INSURED:
    restart_decay_timer();
    insured( value ? true : false );
    increv();
    return new BLong( insured() );
  case MBR_FACING:
    setfacing( (u8)value );
    return new BLong( facing );
  case MBR_SAVEONEXIT:
    saveonexit( value ? true : false );
    return new BLong( saveonexit() );
  case MBR_FIRE_RESIST_MOD:
    fire_resist( fire_resist().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( fire_resist().mod );
    break;
  case MBR_COLD_RESIST_MOD:
    cold_resist( cold_resist().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( cold_resist().mod );
    break;
  case MBR_ENERGY_RESIST_MOD:
    energy_resist( energy_resist().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( energy_resist().mod );
    break;
  case MBR_POISON_RESIST_MOD:
    poison_resist( poison_resist().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( poison_resist().mod );
    break;
  case MBR_PHYSICAL_RESIST_MOD:
    physical_resist( physical_resist().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( physical_resist().mod );
    break;
  case MBR_FIRE_RESIST_CAP_MOD:
    fire_resist_cap( fire_resist_cap().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( fire_resist_cap().mod );
    break;
  case MBR_COLD_RESIST_CAP_MOD:
    cold_resist_cap( cold_resist_cap().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( cold_resist_cap().mod );
    break;
  case MBR_ENERGY_RESIST_CAP_MOD:
    energy_resist_cap( energy_resist_cap().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( energy_resist_cap().mod );
    break;
  case MBR_POISON_RESIST_CAP_MOD:
    poison_resist_cap( poison_resist_cap().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( poison_resist_cap().mod );
    break;
  case MBR_PHYSICAL_RESIST_CAP_MOD:
    physical_resist_cap( physical_resist_cap().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( physical_resist_cap().mod );
    break;

  case MBR_FIRE_DAMAGE_MOD:
    fire_damage( fire_damage().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( fire_damage().mod );
    break;
  case MBR_COLD_DAMAGE_MOD:
    cold_damage( cold_damage().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( cold_damage().mod );
    break;
  case MBR_ENERGY_DAMAGE_MOD:
    energy_damage( energy_damage().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( energy_damage().mod );
    break;
  case MBR_POISON_DAMAGE_MOD:
    poison_damage( poison_damage().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( poison_damage().mod );
    break;

  case MBR_PHYSICAL_DAMAGE_MOD:
    physical_damage( physical_damage().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( physical_damage().mod );
    break;

  case MBR_LOWER_REAG_COST_MOD:
    lower_reagent_cost( lower_reagent_cost().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( lower_reagent_cost().mod );
    break;
  case MBR_SPELL_DAMAGE_INCREASE_MOD:
    spell_damage_increase( spell_damage_increase().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( spell_damage_increase().mod );
    break;
  case MBR_FASTER_CASTING_MOD:
    faster_casting( faster_casting().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( faster_casting().mod );
    break;
  case MBR_FASTER_CAST_RECOVERY_MOD:
    faster_cast_recovery( faster_cast_recovery().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( faster_cast_recovery().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_MOD:
    defence_increase( defence_increase().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( defence_increase().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_CAP_MOD:
    defence_increase_cap( defence_increase_cap().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( defence_increase_cap().mod );
    break;
  case MBR_LOWER_MANA_COST_MOD:
    lower_mana_cost( lower_mana_cost().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( lower_mana_cost().mod );
    break;
  case MBR_HITCHANCE_MOD:  // to be made redundant in the future
  case MBR_HIT_CHANCE_MOD:
    hit_chance( hit_chance().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( hit_chance().mod );
    break;
  case MBR_LUCK_MOD:
    luck( luck().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( luck().mod );
    break;
  case MBR_SWING_SPEED_INCREASE_MOD:
    swing_speed_increase( swing_speed_increase().setAsMod( static_cast<short>( value ) ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          chr->refresh_ar();
      }
    }
    return new BLong( swing_speed_increase().mod );
    break;

  case MBR_QUALITY:
    quality( double( value ) );
    return new Double( double( value ) );
    break;
  case MBR_HP:
    hp_ = static_cast<unsigned short>( value );

    if ( this->isa( Core::UOBJ_CLASS::CLASS_ARMOR ) )
    {
      if ( container != nullptr )
      {
        if ( Core::IsCharacter( container->serial ) )
        {
          Mobile::Character* chr = container->toplevel_chr();
          if ( chr != nullptr )
            chr->refresh_ar();
        }
      }
    }
    return new BLong( hp_ );
  case MBR_MAXHP_MOD:
    this->maxhp_mod( static_cast<s16>( value ) );
    if ( this->isa( Core::UOBJ_CLASS::CLASS_ARMOR ) )
    {
      if ( container != nullptr )
      {
        if ( Core::IsCharacter( container->serial ) )
        {
          Mobile::Character* chr = container->toplevel_chr();
          if ( chr != nullptr )
            chr->refresh_ar();
        }
      }
    }
    return new BLong( value );
  case MBR_NO_DROP:
    no_drop( value ? true : false );
    return new BLong( no_drop() );
  default:
    return nullptr;
  }
}

BObjectImp* Item::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* Item::set_script_member_id_double( const int id, double value )
{
  BObjectImp* imp = base::set_script_member_id_double( id, value );
  if ( imp != nullptr )
    return imp;
  switch ( id )
  {
  case MBR_QUALITY:
    quality( value );
    return new Double( value );
  default:
    return nullptr;
  }
}

BObjectImp* Item::set_script_member_double( const char* membername, double value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id_double( objmember->id, value );
  return nullptr;
}

BObjectImp* Item::script_method_id( const int id, Core::UOExecutor& ex )
{
  BObjectImp* imp = base::script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;
  switch ( id )
  {
  case MTH_SPLITSTACK_AT:
  {
    unsigned short amt;
    Core::Pos4d newpos;
    Item* new_stack( nullptr );
    u16 item_amount = this->getamount();

    if ( !ex.hasParams( 5 ) )
      return new BError( "Not enough parameters" );
    else if ( !ex.getPos4dParam( 0, 1, 2, 3, &newpos ) )
      return new BError( "Invalid parameter type" );
    else if ( !ex.getParam( 4, amt ) )
      return new BError( "No amount specified to pull from existing stack" );
    else if ( amt > this->getamount() )
      return new BError( "Amount must be less than or equal to the stack amount" );
    else if ( amt < 1 )
      return new BError( "Amount was less than 1" );
    else if ( this->inuse() )
      return new BError( "Item is in use" );

    // Check first if the item is non-stackable and just force stacked with CreateItemInInventory
    if ( !this->stackable() && amt > 1 )
    {
      unsigned short i;

      for ( i = 1; i <= amt; i++ )
      {
        if ( this->getamount() == 1 )
          new_stack = this->clone();
        else
          new_stack = this->remove_part_of_stack( 1 );

        Core::Pos4d oldpos = new_stack->pos();
        new_stack->setposition( newpos );
        add_item_to_world( new_stack );
        move_item( new_stack, newpos, oldpos );
        update_item_to_inrange( new_stack );
      }

      if ( this->getamount() == 1 )
        destroy_item( this );
      else
        update_item_to_inrange( this );
      if ( new_stack != nullptr )
        return new Module::EItemRefObjImp( new_stack );
      return nullptr;
    }


    if ( amt == this->getamount() )
      new_stack = this->clone();
    else
      new_stack = this->remove_part_of_stack( amt );

    new_stack->setamount( amt );

    Core::Pos4d oldpos = new_stack->pos();
    new_stack->setposition( newpos );
    add_item_to_world( new_stack );
    move_item( new_stack, newpos, oldpos );
    update_item_to_inrange( new_stack );

    if ( amt == item_amount )
      destroy_item( this );
    else
      update_item_to_inrange( this );

    return new Module::EItemRefObjImp( new_stack );

    break;
  }
  case MTH_SPLITSTACK_INTO:
  {
    unsigned short amt;
    Item* cont_item;

    if ( !ex.hasParams( 2 ) )
      return new BError( "Not enough parameters" );
    else if ( !ex.getItemParam( 0, cont_item ) )
      return new BError( "No container specified" );
    else if ( !ex.getParam( 1, amt ) )
      return new BError( "No amount specified to pull from existing stack" );
    else if ( amt > this->getamount() )
      return new BError( "Amount must be less than or equal to stack amount" );
    else if ( amt < 1 )
      return new BError( "Amount was less than 1" );
    else if ( this->inuse() )
      return new BError( "Item is in use" );
    else if ( !cont_item->isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
      return new BError( "Non-container selected as target" );

    Core::UContainer* newcontainer = static_cast<Core::UContainer*>( cont_item );

    // Check first if the item is non-stackable and just force stacked with CreateItemInInventory

    Item* new_stack( nullptr );
    u16 item_amount = this->getamount();

    if ( !this->stackable() && amt > 1 )
    {
      for ( unsigned short i = 1; i <= amt; i++ )
      {
        if ( this->getamount() == 1 )
          new_stack = this->clone();
        else
          new_stack = this->remove_part_of_stack( 1 );

        bool can_insert = newcontainer->can_insert_add_item(
            nullptr, Core::UContainer::MT_CORE_MOVED, new_stack );
        if ( !can_insert )
        {
          // Put new_stack back with the original stack
          if ( new_stack != this )
            this->add_to_self( new_stack );
          return new BError( "Could not insert new stack into container" );
        }

        newcontainer->add_at_random_location( new_stack );
        update_item_to_inrange( new_stack );
        UpdateCharacterWeight( new_stack );
        newcontainer->on_insert_add_item( nullptr, Core::UContainer::MT_CORE_MOVED, new_stack );
      }

      if ( this->getamount() == 1 )
        destroy_item( this );
      else
        update_item_to_inrange( this );
      if ( new_stack != nullptr )
        return new Module::EItemRefObjImp( new_stack );
      return nullptr;
    }

    if ( amt == this->getamount() )
      new_stack = this->clone();
    else
      new_stack = this->remove_part_of_stack( amt );

    auto create_new_stack = [&]() -> BObjectImp* {
      bool can_insert =
          newcontainer->can_insert_add_item( nullptr, Core::UContainer::MT_CORE_MOVED, new_stack );
      if ( !can_insert )
      {
        // Put newstack back with the original stack
        if ( new_stack != this )
          this->add_to_self( new_stack );
        return new BError( "Could not insert new stack into container" );
      }
      newcontainer->add_at_random_location( new_stack );
      new_stack->setamount( amt );
      update_item_to_inrange( new_stack );
      UpdateCharacterWeight( new_stack );
      newcontainer->on_insert_add_item( nullptr, Core::UContainer::MT_CORE_MOVED, new_stack );

      if ( amt == item_amount )
        destroy_item( this );
      else
        update_item_to_inrange( this );

      return new Module::EItemRefObjImp( new_stack );
    };

    int add_to_existing_stack;
    if ( !ex.hasParams( 3 ) ||
         ( ex.getParam( 2, add_to_existing_stack ) && add_to_existing_stack != 0 ) )
    {
      Item* existing_stack = newcontainer->find_addable_stack( new_stack );
      if ( existing_stack != nullptr && new_stack->stackable() )
      {
        if ( !newcontainer->can_insert_increase_stack( nullptr, Core::UContainer::MT_CORE_MOVED,
                                                       existing_stack, new_stack->getamount(),
                                                       new_stack ) )
        {
          if ( new_stack != this )
            this->add_to_self( new_stack );
          return new BError( "Could not add to existing stack" );
        }
      }
      else
        return create_new_stack();

      u16 amount = new_stack->getamount();
      existing_stack->add_to_self( new_stack );
      update_item_to_inrange( existing_stack );

      UpdateCharacterWeight( existing_stack );

      newcontainer->on_insert_increase_stack( nullptr, Core::UContainer::MT_CORE_MOVED,
                                              existing_stack, amount );

      if ( amt == item_amount )
        destroy_item( this );
      else
        update_item_to_inrange( this );

      return new Module::EItemRefObjImp( existing_stack );
    }
    else
      return create_new_stack();

    break;
  }
  case MTH_HAS_EXISTING_STACK:
  {
    Item* cont = nullptr;

    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough params" );
    else if ( !ex.getItemParam( 0, cont ) )
      return new BError( "No container specified" );
    else if ( this->inuse() )
      return new BError( "Item is in use" );
    else if ( !cont->isa( Core::UOBJ_CLASS::CLASS_CONTAINER ) )
      return new BError( "Non-container selected as target" );

    Core::UContainer* stackcontainer = static_cast<Core::UContainer*>( cont );

    Item* existing_stack = stackcontainer->find_addable_stack( this );

    if ( existing_stack != nullptr )
      return new Module::EItemRefObjImp( existing_stack );
    return nullptr;

    break;
  }
  default:
    return nullptr;
  }
}

BObjectImp* Item::script_method( const char* methodname, Core::UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  return nullptr;
}

BObjectImp* Item::custom_script_method( const char* methodname, Core::UOExecutor& ex )
{
  const ItemDesc& id = itemdesc();
  if ( id.method_script != nullptr )
  {
    unsigned PC;
    if ( id.method_script->FindExportedFunction(
             methodname, static_cast<unsigned int>( ex.numParams() + 1 ), PC ) )
      return id.method_script->call( PC, make_ref(), ex.fparams );
  }
  return Core::gamestate.system_hooks.call_script_method( methodname, &ex, this );
}

BObject Item::call_custom_method( const char* methodname )
{
  // no systemhook needed used for openbook uo module function
  BObjectImpRefVec noparams;
  return call_custom_method( methodname, noparams );
}

BObject Item::call_custom_method( const char* methodname, BObjectImpRefVec& pmore )
{
  // no systemhook needed used for openbook uo module function
  const ItemDesc& id = itemdesc();
  if ( id.method_script != nullptr )
  {
    unsigned PC;
    if ( id.method_script->FindExportedFunction(
             methodname, static_cast<unsigned int>( pmore.size() + 1 ), PC ) )
      return id.method_script->call( PC, new Module::EItemRefObjImp( this ), pmore );
    else
    {
      std::string message;
      message = "Method script for objtype " + id.objtype_description() +
                " does not export function " + std::string( methodname ) + " taking " +
                Clib::tostring( pmore.size() + 1 ) + " parameters";
      BError* err = new BError( message );
      return BObject( err );
    }
  }
  return BObject( new BError( "No method script defined for " + id.objtype_description() ) );
}

BObjectImp* Item::make_ref()
{
  return new Module::EItemRefObjImp( this );
}
}  // namespace Items
namespace Mobile
{
using namespace Bscript;

class ARUpdater
{
public:
  static void on_change( Character* chr )
  {
    chr->refresh_ar();  // FIXME inefficient
    // if (chr->client != nullptr) // already send in refresh_ar()
    //{
    //  send_full_statmsg( chr->client, chr );
    //}
  }
};

BObjectImp* Character::make_ref()
{
  return new Module::ECharacterRefObjImp( this );
}
BObjectImp* Character::make_offline_ref()
{
  return new Module::EOfflineCharacterRefObjImp( this );
}

BObjectImp* Character::get_script_member_id( const int id ) const
{
  BObjectImp* imp = base::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;


  auto EnforceCaps = []( s16 baseValue, const s16 capValue ) -> s16 {
    const bool ignore_caps = Core::settingsManager.ssopt.core_ignores_defence_caps;


    if ( !ignore_caps )
      baseValue = std::min( baseValue, capValue );

    return baseValue;
  };

  switch ( id )
  {
  case MBR_WARMODE:
    return new BLong( warmode() );
    break;
  case MBR_GENDER:
    return new BLong( gender );
    break;
  case MBR_RACE:
    return new BLong( race );
    break;
  case MBR_TRUEOBJTYPE:
    return new BLong( trueobjtype );
    break;
  case MBR_TRUECOLOR:
    return new BLong( truecolor );
    break;
  case MBR_AR_MOD:
    return new BLong( ar_mod() );
    break;
  case MBR_DELAY_MOD:
    return new BLong( delay_mod() );
    break;
  case MBR_HIDDEN:
    return new BLong( hidden() ? 1 : 0 );
    break;
  case MBR_CONCEALED:
    return new BLong( concealed() );
    break;
  case MBR_FROZEN:
    return new BLong( frozen() ? 1 : 0 );
    break;
  case MBR_PARALYZED:
    return new BLong( paralyzed() ? 1 : 0 );
    break;
  case MBR_POISONED:
    return new BLong( poisoned() ? 1 : 0 );
    break;
  case MBR_STEALTHSTEPS:
    return new BLong( stealthsteps_ );
    break;
  case MBR_MOUNTEDSTEPS:
    return new BLong( mountedsteps_ );
    break;
  case MBR_SQUELCHED:
    return new BLong( squelched() ? 1 : 0 );
    break;
  case MBR_DEAD:
    return new BLong( dead() );
    break;
  case MBR_AR:
    return new BLong( ar() );
    break;
  case MBR_BACKPACK:
  {
    Items::Item* bp = backpack();
    if ( bp != nullptr )
      return bp->make_ref();
    return new BError( "That has no backpack" );
    break;
  }
  case MBR_TRADEWINDOW:
  {
    Core::UContainer* tw = trading_cont.get();
    if ( trading_with != nullptr )
      return tw->make_ref();
    return new BError( "That has no active tradewindow" );
    break;
  }
  case MBR_WEAPON:
    if ( weapon != nullptr )
      return weapon->make_ref();
    return new BLong( 0 );
    break;
  case MBR_SHIELD:
    if ( shield != nullptr )
      return shield->make_ref();
    return new BLong( 0 );
    break;
  case MBR_ACCTNAME:
    if ( acct != nullptr )
      return new String( acct->name() );
    return new BError( "Not attached to an account" );
    break;

  case MBR_ACCT:
    if ( acct != nullptr )
      return new Accounts::AccountObjImp( Accounts::AccountPtrHolder( acct ) );
    return new BError( "Not attached to an account" );
    break;
  case MBR_CMDLEVEL:
    return new BLong( cmdlevel() );
    break;
  case MBR_CMDLEVELSTR:
    return new String( Core::gamestate.cmdlevels[cmdlevel()].name );
    break;
  case MBR_CRIMINAL:
    return new BLong( is_criminal() ? 1 : 0 );
    break;
  case MBR_TEMPORALLY_CRIMINAL:
    return new BLong( is_temporally_criminal() ? 1 : 0 );
    break;
  case MBR_IP:
    if ( client != nullptr )
      return new String( client->ipaddrAsString() );
    return new String( "" );
    break;
  case MBR_GOLD:
    return new BLong( gold_carried() );
    break;

  case MBR_TITLE_PREFIX:
    return new String( title_prefix() );
    break;
  case MBR_TITLE_SUFFIX:
    return new String( title_suffix() );
    break;
  case MBR_TITLE_GUILD:
    return new String( title_guild() );
    break;
  case MBR_TITLE_RACE:
    return new String( title_race() );
    break;
  case MBR_UCLANG:
    return new String( uclang );
    break;
  case MBR_GUILDID:
    return new BLong( guildid() );
    break;
  case MBR_GUILD:
    if ( has_guild() )
      return Module::GuildExecutorModule::CreateGuildRefObjImp( guild() );
    return new BError( "Not a member of a guild" );
    break;

  case MBR_MURDERER:
    return new BLong( is_murderer() ? 1 : 0 );
    break;
  case MBR_ATTACHED:
    if ( script_ex == nullptr )
      return new BError( "No script attached." );
    return new Core::ScriptExObjImp( script_ex );
    break;
  case MBR_CLIENTVERSION:
    if ( client != nullptr )
      return new String( client->getversion() );
    return new String( "" );
    break;
  case MBR_CLIENTVERSIONDETAIL:
    if ( client != nullptr )
    {
      std::unique_ptr<BStruct> info( new BStruct );
      Network::VersionDetailStruct version = client->getversiondetail();
      info->addMember( "major", new BLong( version.major ) );
      info->addMember( "minor", new BLong( version.minor ) );
      info->addMember( "rev", new BLong( version.rev ) );
      info->addMember( "patch", new BLong( version.patch ) );
      return info.release();
    }
    else
    {
      std::unique_ptr<BStruct> info( new BStruct );
      info->addMember( "major", new BLong( 0 ) );
      info->addMember( "minor", new BLong( 0 ) );
      info->addMember( "rev", new BLong( 0 ) );
      info->addMember( "patch", new BLong( 0 ) );
      return info.release();
    }
    break;

  case MBR_CLIENTINFO:
    if ( client != nullptr )
      return client->getclientinfo();
    return new BLong( 0 );
    break;

  case MBR_CREATEDAT:
    return new BLong( created_at );
    break;

  case MBR_REPORTABLES:
    return GetReportables();
    break;

  case MBR_OPPONENT:
    if ( opponent_ != nullptr )
      return opponent_->make_ref();
    return new BError( "Mobile does not have any opponent selected." );
    break;
  case MBR_CONNECTED:
    return new BLong( connected() ? 1 : 0 );
    break;
  case MBR_TRADING_WITH:
    if ( trading_with != nullptr )
      return trading_with->make_ref();
    return new BError( "Mobile is not currently trading with anyone." );
    break;
  case MBR_CLIENTTYPE:
    return new BLong( client != nullptr ? client->ClientType : 0 );
    break;
  case MBR_CURSOR:
    if ( client != nullptr )
      return new BLong( target_cursor_busy() ? 1 : 0 );
    return new BLong( 0 );
    break;
  case MBR_GUMP:
    if ( client != nullptr )
      return new BLong( has_active_gump() ? 1 : 0 );
    return new BLong( 0 );
    break;
  case MBR_PROMPT:
    if ( client != nullptr )
      return new BLong( has_active_prompt() ? 1 : 0 );
    return new BLong( 0 );
    break;
  case MBR_MOVEMODE:
  {
    std::string mode = "";
    if ( movemode & Plib::MOVEMODE_LAND )
      mode = "L";
    if ( movemode & Plib::MOVEMODE_SEA )
      mode += "S";
    if ( movemode & Plib::MOVEMODE_AIR )
      mode += "A";
    if ( movemode & Plib::MOVEMODE_FLY )
      mode += "F";
    return new String( mode );
    break;
  }
  case MBR_HITCHANCE_MOD:
    return new BLong( hitchance_mod() );
    break;
  case MBR_EVASIONCHANCE_MOD:
    return new BLong( evasionchance_mod() );
    break;
  case MBR_CARRYINGCAPACITY_MOD:
    return new BLong( carrying_capacity_mod() );
    break;
  case MBR_CARRYINGCAPACITY:
    return new BLong( carrying_capacity() );
    break;
  case MBR_FIRE_RESIST:
    return new BLong( EnforceCaps( fire_resist().sum(), fire_resist_cap().sum() ) );
  case MBR_COLD_RESIST:
    return new BLong( EnforceCaps( cold_resist().sum(), cold_resist_cap().sum() ) );
  case MBR_ENERGY_RESIST:
    return new BLong( EnforceCaps( energy_resist().sum(), energy_resist_cap().sum() ) );
  case MBR_POISON_RESIST:
    return new BLong( EnforceCaps( poison_resist().sum(), poison_resist_cap().sum() ) );
  case MBR_PHYSICAL_RESIST:
    return new BLong( EnforceCaps( physical_resist().sum(), physical_resist_cap().sum() ) );
  case MBR_FIRE_RESIST_MOD:
    return new BLong( fire_resist().mod );
    break;
  case MBR_COLD_RESIST_MOD:
    return new BLong( cold_resist().mod );
    break;
  case MBR_ENERGY_RESIST_MOD:
    return new BLong( energy_resist().mod );
    break;
  case MBR_POISON_RESIST_MOD:
    return new BLong( poison_resist().mod );
    break;
  case MBR_PHYSICAL_RESIST_MOD:
    return new BLong( physical_resist().mod );
    break;
  case MBR_FIRE_RESIST_CAP:
    return new BLong( fire_resist_cap().sum() );
    break;
  case MBR_COLD_RESIST_CAP:
    return new BLong( cold_resist_cap().sum() );
    break;
  case MBR_ENERGY_RESIST_CAP:
    return new BLong( energy_resist_cap().sum() );
    break;
  case MBR_POISON_RESIST_CAP:
    return new BLong( poison_resist_cap().sum() );
    break;
  case MBR_PHYSICAL_RESIST_CAP:
    return new BLong( physical_resist_cap().sum() );
    break;
  case MBR_FIRE_RESIST_CAP_MOD:
    return new BLong( fire_resist_cap().mod );
    break;
  case MBR_COLD_RESIST_CAP_MOD:
    return new BLong( cold_resist_cap().mod );
    break;
  case MBR_ENERGY_RESIST_CAP_MOD:
    return new BLong( energy_resist_cap().mod );
    break;
  case MBR_POISON_RESIST_CAP_MOD:
    return new BLong( poison_resist_cap().mod );
    break;
  case MBR_PHYSICAL_RESIST_CAP_MOD:
    return new BLong( physical_resist_cap().mod );
    break;
  case MBR_LOWER_REAG_COST:
    return new BLong( lower_reagent_cost().sum() );
    break;
  case MBR_SPELL_DAMAGE_INCREASE:
    return new BLong( spell_damage_increase().sum() );
    break;
  case MBR_FASTER_CASTING:
    return new BLong( faster_casting().sum() );
    break;
  case MBR_FASTER_CAST_RECOVERY:
    return new BLong( faster_cast_recovery().sum() );
    break;
  case MBR_LOWER_REAG_COST_MOD:
    return new BLong( lower_reagent_cost().mod );
    break;
  case MBR_SPELL_DAMAGE_INCREASE_MOD:
    return new BLong( spell_damage_increase().mod );
    break;
  case MBR_FASTER_CASTING_MOD:
    return new BLong( faster_casting().mod );
    break;
  case MBR_FASTER_CAST_RECOVERY_MOD:
    return new BLong( faster_cast_recovery().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_MOD:
    return new BLong( defence_increase().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_CAP_MOD:
    return new BLong( defence_increase_cap().mod );
    break;
  case MBR_LOWER_MANA_COST_MOD:
    return new BLong( lower_mana_cost().mod );
    break;
  case MBR_HIT_CHANCE_MOD:
    return new BLong( hit_chance().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE:
    return new BLong( EnforceCaps( defence_increase().sum(), defence_increase_cap().sum() ) );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_CAP:
    return new BLong( defence_increase_cap().sum() );
    break;
  case MBR_LOWER_MANA_COST:
    return new BLong( lower_mana_cost().sum() );
    break;
  case MBR_HIT_CHANCE:
    return new BLong( hit_chance().sum() );
    break;
  case MBR_STATCAP:
    return new BLong( skillstatcap().statcap );
    break;
  case MBR_SKILLCAP:
    return new BLong( skillstatcap().skillcap );
    break;
  case MBR_LUCK:
    return new BLong( luck().sum() );
    break;
  case MBR_LUCK_MOD:
    return new BLong( luck().mod );
    break;
  case MBR_SWING_SPEED_INCREASE:
    return new BLong( swing_speed_increase().sum() );
    break;
  case MBR_SWING_SPEED_INCREASE_MOD:
    return new BLong( swing_speed_increase().mod );
    break;
  case MBR_FOLLOWERSMAX:
    return new BLong( followers().followers_max );
    break;
  case MBR_TITHING:
    return new BLong( tithing() );
    break;
  case MBR_FOLLOWERS:
    return new BLong( followers().followers );
    break;
  case MBR_FIRE_DAMAGE:
    return new BLong( fire_damage().sum() );
    break;
  case MBR_COLD_DAMAGE:
    return new BLong( cold_damage().sum() );
    break;
  case MBR_ENERGY_DAMAGE:
    return new BLong( energy_damage().sum() );
    break;
  case MBR_POISON_DAMAGE:
    return new BLong( poison_damage().sum() );
    break;
  case MBR_PHYSICAL_DAMAGE:
    return new BLong( physical_damage().sum() );
    break;
  case MBR_FIRE_DAMAGE_MOD:
    return new BLong( fire_damage().mod );
    break;
  case MBR_COLD_DAMAGE_MOD:
    return new BLong( cold_damage().mod );
    break;
  case MBR_ENERGY_DAMAGE_MOD:
    return new BLong( energy_damage().mod );
    break;
  case MBR_POISON_DAMAGE_MOD:
    return new BLong( poison_damage().mod );
    break;
  case MBR_PHYSICAL_DAMAGE_MOD:
    return new BLong( physical_damage().mod );
    break;
  case MBR_PARTY:
    if ( has_party() )
      return Module::CreatePartyRefObjImp( party() );
    return new BError( "Not a member of a party" );
    break;
  case MBR_PARTYLOOT:
    return new BLong( party_can_loot() );
  case MBR_CANDIDATE_OF_PARTY:
    if ( has_candidate_of() )
      return Module::CreatePartyRefObjImp( candidate_of() );
    return new BError( "Not a candidate of a party" );
    break;
  case MBR_MOVECOST_WALK:
    return new Double( movement_cost().walk );
    break;
  case MBR_MOVECOST_RUN:
    return new Double( movement_cost().run );
    break;
  case MBR_MOVECOST_WALK_MOUNTED:
    return new Double( movement_cost().walk_mounted );
    break;
  case MBR_MOVECOST_RUN_MOUNTED:
    return new Double( movement_cost().run_mounted );
    break;

  case MBR_AGGRESSORTO:
    return GetAggressorTo();
    break;
  case MBR_LAWFULLYDAMAGED:
    return GetLawFullyDamaged();
    break;

  case MBR_UO_EXPANSION_CLIENT:
    return new BLong( client != nullptr ? client->UOExpansionFlagClient : 0 );
    break;

  case MBR_DEAFENED:
    return new BLong( deafened() ? 1 : 0 );
    break;

  case MBR_CLIENT:
    if ( ( client != nullptr ) && ( client->isConnected() ) )
      return client->make_ref();
    return new BError( "No client attached." );
    break;
  case MBR_EDITING:
    return new BLong( is_house_editing() ? 1 : 0 );
    break;
  case MBR_LASTCOORD:
    if ( client != nullptr )
    {
      const Core::Pos3d last_pos = this->lastxyz;

      std::unique_ptr<BStruct> lastcoord( new BStruct );
      lastcoord->addMember( "lastx", new BLong( last_pos.x() ) );
      lastcoord->addMember( "lasty", new BLong( last_pos.y() ) );
      lastcoord->addMember( "lastz", new BLong( last_pos.z() ) );
      return lastcoord.release();
    }
    return new BError( "No client attached." );
  case MBR_ACTIVE_SKILL:
    return new BLong( skill_ex_active() );
  case MBR_CASTING_SPELL:
    return new BLong( casting_spell() );
  case MBR_LAST_TEXTCOLOR:
    return new BLong( last_textcolor() );
  }
  // if all else fails, returns nullptr
  return nullptr;
}

BObjectImp* Character::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* Character::set_script_member_id( const int id, const std::string& value )
{
  BObjectImp* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  String* ret;
  switch ( id )
  {
  case MBR_TITLE_PREFIX:
    title_prefix( value );
    ret = new String( value );
    break;
  case MBR_TITLE_SUFFIX:
    title_suffix( value );
    ret = new String( value );
    break;
  case MBR_TITLE_GUILD:
    title_guild( value );
    ret = new String( value );
    break;
  case MBR_TITLE_RACE:
    title_race( value );
    ret = new String( value );
    break;
  default:
    return nullptr;
  }
  set_dirty();
  increv();
  if ( this->has_active_client() )
    send_object_cache_to_inrange( this );
  return ret;
}

BObjectImp* Character::set_script_member( const char* membername, const std::string& value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* Character::set_script_member_id( const int id, int value )
{
  BObjectImp* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_GENDER:
    if ( value )
      gender = Plib::GENDER_FEMALE;
    else
      gender = Plib::GENDER_MALE;
    return new BLong( gender );
  case MBR_RACE:
    if ( value == Plib::RACE_HUMAN )
      race = Plib::RACE_HUMAN;
    else if ( value == Plib::RACE_ELF )
      race = Plib::RACE_ELF;
    else if ( value == Plib::RACE_GARGOYLE )
      race = Plib::RACE_GARGOYLE;
    if ( ( race != Plib::RACE_GARGOYLE ) &&
         ( movemode & Plib::MOVEMODE_FLY ) )                           // FIXME graphic based maybe?
      movemode = ( Plib::MOVEMODE )( movemode ^ Plib::MOVEMODE_FLY );  // remove flying
    return new BLong( race );
  case MBR_TRUEOBJTYPE:
    return new BLong( trueobjtype = static_cast<unsigned int>( value ) );
  case MBR_TRUECOLOR:
    return new BLong( truecolor = static_cast<unsigned short>( value ) );
  case MBR_AR_MOD:
    ar_mod( static_cast<short>( value ) );
    refresh_ar();
    return new BLong( ar_mod() );
  case MBR_DELAY_MOD:
    delay_mod( static_cast<short>( value ) );
    return new BLong( delay_mod() );
  case MBR_HIDDEN:
  {
    // FIXME: don't call on_change unless the value actually changed?
    hidden( value ? true : false );
    return new BLong( hidden() );
  }
  case MBR_CONCEALED:
    concealed( static_cast<unsigned char>( value ) );
    return new BLong( concealed() );
  case MBR_FROZEN:
    mob_flags_.change( MOB_FLAGS::FROZEN, value ? true : false );
    return new BLong( frozen() );
  case MBR_PARALYZED:
    mob_flags_.change( MOB_FLAGS::PARALYZED, value ? true : false );
    return new BLong( paralyzed() );
  case MBR_POISONED:
    poisoned( value ? true : false );
    return new BLong( poisoned() );
  case MBR_STEALTHSTEPS:
    return new BLong( stealthsteps_ = static_cast<unsigned short>( value ) );
  case MBR_MOUNTEDSTEPS:
    return new BLong( mountedsteps_ = static_cast<unsigned int>( value ) );
  case MBR_CMDLEVEL:
    if ( value >= static_cast<int>( Core::gamestate.cmdlevels.size() ) )
      cmdlevel( static_cast<unsigned char>( Core::gamestate.cmdlevels.size() ) - 1, true );
    else
      cmdlevel( static_cast<unsigned char>( value ), true );
    return new BLong( cmdlevel() );
  case MBR_MURDERER:
    // make_murderer handles the updating
    make_murderer( value ? true : false );
    return new BLong( is_murderer() );
  case MBR_HITCHANCE_MOD:
    hitchance_mod( static_cast<short>( value ) );
    return new BLong( hitchance_mod() );
  case MBR_EVASIONCHANCE_MOD:
    evasionchance_mod( static_cast<short>( value ) );
    return new BLong( evasionchance_mod() );
  case MBR_CARRYINGCAPACITY_MOD:
    carrying_capacity_mod( static_cast<short>( value ) );
    if ( client != nullptr )
    {  // CHECKME consider sending less frequently
      send_full_statmsg( client, this );
    }
    return new BLong( carrying_capacity_mod() );
  case MBR_FACING:
    if ( !face( static_cast<Core::UFACING>( value & PKTIN_02_FACING_MASK ), 0 ) )
      return new BLong( 0 );
    on_facing_changed();
    return new BLong( 1 );
  case MBR_FIRE_RESIST_MOD:
    fire_resist( fire_resist().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( fire_resist().mod );
    break;
  case MBR_COLD_RESIST_MOD:
    cold_resist( cold_resist().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( cold_resist().mod );
    break;
  case MBR_ENERGY_RESIST_MOD:
    energy_resist( energy_resist().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( energy_resist().mod );
    break;
  case MBR_POISON_RESIST_MOD:
    poison_resist( poison_resist().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( poison_resist().mod );
    break;
  case MBR_PHYSICAL_RESIST_MOD:
    physical_resist( physical_resist().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( physical_resist().mod );
    break;
  case MBR_LOWER_REAG_COST_MOD:
    lower_reagent_cost( lower_reagent_cost().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( lower_reagent_cost().mod );
    break;
  case MBR_SPELL_DAMAGE_INCREASE_MOD:
    spell_damage_increase( spell_damage_increase().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( spell_damage_increase().mod );
    break;
  case MBR_FASTER_CASTING_MOD:
    faster_casting( faster_casting().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( faster_casting().mod );
    break;
  case MBR_FASTER_CAST_RECOVERY_MOD:
    faster_cast_recovery( faster_cast_recovery().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( faster_cast_recovery().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_MOD:
    defence_increase( defence_increase().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( defence_increase().mod );
    break;
  case MBR_DEFENCE_CHANCE_INCREASE_CAP_MOD:
    defence_increase_cap( defence_increase_cap().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( defence_increase_cap().mod );
    break;
  case MBR_LOWER_MANA_COST_MOD:
    lower_mana_cost( lower_mana_cost().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( lower_mana_cost().mod );
    break;
  case MBR_HIT_CHANCE_MOD:
    hit_chance( hit_chance().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( hit_chance().mod );
    break;
  case MBR_FIRE_RESIST_CAP_MOD:
    fire_resist_cap( fire_resist_cap().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( fire_resist_cap().mod );
    break;
  case MBR_COLD_RESIST_CAP_MOD:
    cold_resist_cap( cold_resist_cap().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( cold_resist_cap().mod );
    break;
  case MBR_ENERGY_RESIST_CAP_MOD:
    energy_resist_cap( energy_resist_cap().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( energy_resist_cap().mod );
    break;
  case MBR_PHYSICAL_RESIST_CAP_MOD:
    physical_resist_cap( physical_resist_cap().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( physical_resist_cap().mod );
    break;
  case MBR_POISON_RESIST_CAP_MOD:
    poison_resist_cap( poison_resist_cap().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( poison_resist_cap().mod );
    break;
  case MBR_LUCK_MOD:
    luck( luck().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( luck().mod );
    break;
  case MBR_SWING_SPEED_INCREASE_MOD:
    swing_speed_increase( swing_speed_increase().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( swing_speed_increase().mod );
    break;
  case MBR_STATCAP:
  {
    auto val = skillstatcap();
    val.statcap = static_cast<short>( value );
    skillstatcap( val );
    if ( !this->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
      on_aos_ext_stat_changed();
    return new BLong( skillstatcap().statcap );
  }
  case MBR_SKILLCAP:
  {
    auto val = skillstatcap();
    val.skillcap = static_cast<u16>( value );
    skillstatcap( val );
    return new BLong( skillstatcap().skillcap );
  }
  case MBR_FOLLOWERSMAX:
  {
    auto val = followers();
    val.followers_max = static_cast<u8>( value );
    followers( val );
    if ( !this->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
      on_aos_ext_stat_changed();
    return new BLong( followers().followers_max );
  }
  case MBR_TITHING:
    tithing( static_cast<s32>( value ) );
    if ( !this->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
      on_aos_ext_stat_changed();
    return new BLong( tithing() );
    break;
  case MBR_FOLLOWERS:
  {
    auto val = followers();
    val.followers = static_cast<u8>( value );
    followers( val );
    if ( !this->isa( Core::UOBJ_CLASS::CLASS_NPC ) )
      on_aos_ext_stat_changed();
    return new BLong( followers().followers );
  }
  case MBR_FIRE_DAMAGE_MOD:
    fire_damage( fire_damage().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( fire_damage().mod );
    break;
  case MBR_COLD_DAMAGE_MOD:
    cold_damage( cold_damage().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( cold_damage().mod );
    break;
  case MBR_ENERGY_DAMAGE_MOD:
    energy_damage( energy_damage().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( energy_damage().mod );
    break;
  case MBR_POISON_DAMAGE_MOD:
    poison_damage( poison_damage().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( poison_damage().mod );
    break;
  case MBR_PHYSICAL_DAMAGE_MOD:
    physical_damage( physical_damage().setAsMod( static_cast<short>( value ) ) );
    refresh_ar();
    return new BLong( physical_damage().mod );
    break;
  case MBR_MOVECOST_WALK:
  {
    auto val = movement_cost();
    val.walk = static_cast<double>( value );
    movement_cost( val );
    return new Double( movement_cost().walk );
  }
  case MBR_MOVECOST_RUN:
  {
    auto val = movement_cost();
    val.run = static_cast<double>( value );
    movement_cost( val );
    return new Double( movement_cost().run );
  }
  case MBR_MOVECOST_WALK_MOUNTED:
  {
    auto val = movement_cost();
    val.walk_mounted = static_cast<double>( value );
    movement_cost( val );
    return new Double( movement_cost().walk_mounted );
  }
  case MBR_MOVECOST_RUN_MOUNTED:
  {
    auto val = movement_cost();
    val.run_mounted = static_cast<double>( value );
    movement_cost( val );
    return new Double( movement_cost().run_mounted );
  }
  default:
    return nullptr;
  }
}

BObjectImp* Character::set_script_member_id_double( const int id, double value )
{
  switch ( id )
  {
  case MBR_MOVECOST_WALK:
  {
    auto val = movement_cost();
    val.walk = value;
    movement_cost( val );
    return new Double( movement_cost().walk );
  }
  case MBR_MOVECOST_RUN:
  {
    auto val = movement_cost();
    val.run = value;
    movement_cost( val );
    return new Double( movement_cost().run );
  }
  case MBR_MOVECOST_WALK_MOUNTED:
  {
    auto val = movement_cost();
    val.walk_mounted = value;
    movement_cost( val );
    return new Double( movement_cost().walk_mounted );
  }
  case MBR_MOVECOST_RUN_MOUNTED:
  {
    auto val = movement_cost();
    val.run_mounted = value;
    movement_cost( val );
    return new Double( movement_cost().run_mounted );
  }
  default:
    return nullptr;
  }
}

BObjectImp* Character::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* Character::script_method_id( const int id, Core::UOExecutor& ex )
{
  BObjectImp* imp = base::script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;

  switch ( id )
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
    bool newval = true;
    if ( ex.hasParams( 1 ) )
    {
      int lval;
      if ( !ex.getParam( 0, lval ) )
        return new BError( "Invalid parameter type" );
      if ( !lval )
        newval = false;
    }

    if ( newval != poisoned() )
    {
      set_dirty();
      poisoned( newval );
      check_undamaged();
      Module::UOExecutorModule* uoexec =
          static_cast<Module::UOExecutorModule*>( ex.findModule( "UO" ) );
      if ( uoexec != nullptr && uoexec->controller_.get() )
      {
        Character* attacker = uoexec->controller_.get();
        if ( !attacker->orphan() )
        {
          if ( poisoned() )
            attacker->repsys_on_damage( this );
          else
            attacker->repsys_on_help( this );
        }
      }
    }
    return new BLong( 1 );
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
    if ( ex.hasParams( 1 ) )
    {
      int lval;
      if ( !ex.getParam( 0, lval ) )
        return new BError( "Invalid parameter type" );
      if ( !lval )
        newval = false;
    }

    if ( newval != paralyzed() )
    {
      set_dirty();
      mob_flags_.change( MOB_FLAGS::PARALYZED, newval );
      check_undamaged();
      Module::UOExecutorModule* uoexec =
          static_cast<Module::UOExecutorModule*>( ex.findModule( "UO" ) );
      if ( uoexec != nullptr && uoexec->controller_.get() )
      {
        Character* attacker = uoexec->controller_.get();
        if ( !attacker->orphan() )
        {
          if ( paralyzed() )
            attacker->repsys_on_damage( this );
          else
            attacker->repsys_on_help( this );
        }
      }
    }
    return new BLong( 1 );
  }

  /*
   mobile.SetCriminal( level := 1 )
   if level is 0, clears the CriminalTimer
   */
  case MTH_SETCRIMINAL:
  {
    int level = 1;
    if ( ex.hasParams( 1 ) )
    {
      if ( !ex.getParam( 0, level ) )
        return new BError( "Invalid parameter type" );
      if ( level < 0 )
        return new BError( "Level must be >= 0" );
    }
    set_dirty();
    // make_criminal handles the updating
    make_criminal( level );
    return new BLong( 1 );
  }

  case MTH_SETLIGHTLEVEL:
  {
    int level, duration;
    if ( !ex.hasParams( 2 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, level ) && ex.getParam( 1, duration ) )
    {
      lightoverride( level );

      if ( duration == -1 )
        lightoverride_until( ~0u );
      else if ( duration == 0 )
        lightoverride_until( 0 );
      else
        lightoverride_until( Core::read_gameclock() + duration );

      check_region_changes();
      if ( duration == -1 )
        return new BLong( duration );
      return new BLong( lightoverride_until() );
    }
    break;
  }

  case MTH_SETSEASON:
  {
    int season_id, playsound;

    if ( !ex.hasParams( 2 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, season_id ) && ex.getParam( 1, playsound ) )
    {
      if ( season_id < 0 || season_id > 4 )
        return new BError( "Invalid season id" );

      if ( client && client->getversiondetail().major >= 1 )
      {
        Network::PktHelper::PacketOut<Network::PktOut_BC> msg;
        msg->Write<u8>( static_cast<u16>( season_id ) );
        msg->Write<u8>( static_cast<u16>( playsound ) );
        msg.Send( client );
        return new BLong( 1 );
      }
    }
    break;
  }
  case MTH_SQUELCH:
  {
    int duration;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, duration ) )
    {
      set_dirty();
      if ( duration == -1 )
      {
        squelched_until( ~0u );
        return new BLong( -1 );
      }
      else if ( duration == 0 )
        squelched_until( 0 );
      else
        squelched_until( Core::read_gameclock() + duration );
      return new BLong( squelched_until() );
    }
    break;
  }
  case MTH_ENABLE:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      if ( has_privilege( pstr->data() ) )
      {
        set_dirty();
        set_setting( pstr->data(), true );
        return new BLong( 1 );
      }
      return new BError( "Mobile doesn't have that privilege" );
    }
    break;
  }

  case MTH_DISABLE:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      set_dirty();
      set_setting( pstr->data(), false );
      return new BLong( 1 );
    }
    break;
  }

  case MTH_ENABLED:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
      return new BLong( setting_enabled( pstr->data() ) ? 1 : 0 );
    break;
  }

  case MTH_PRIVILEGES:
  {
    std::unique_ptr<BDictionary> dict( new BDictionary );
    ISTRINGSTREAM istrm( all_privs() );
    std::string tmp;
    while ( istrm >> tmp )
    {
      dict->addMember( new String( tmp ), new BLong( setting_enabled( tmp.c_str() ) ) );
    }
    return dict.release();
    break;
  }

  case MTH_SETCMDLEVEL:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
    {
      Core::CmdLevel* pcmdlevel = Core::find_cmdlevel( pstr->data() );
      if ( pcmdlevel )
      {
        set_dirty();
        cmdlevel( pcmdlevel->cmdlevel, true );
        return new BLong( 1 );
      }
      return new BError( "No such command level" );
    }
    break;
  }
  case MTH_SPENDGOLD:
  {
    int amt;
    if ( ex.numParams() != 1 || !ex.getParam( 0, amt ) )
      return new BError( "Invalid parameter type" );

    if ( gold_carried() < static_cast<unsigned int>( amt ) )
      return new BError( "Insufficient funds" );

    spend_gold( amt );
    return new BLong( 1 );
  }

  case MTH_SETMURDERER:
  {
    int lnewval = 1;
    if ( ex.hasParams( 1 ) )
    {
      if ( !ex.getParam( 0, lnewval ) )
        return new BError( "Invalid parameter type" );
    }
    set_dirty();
    // make_murderer handles the updating
    make_murderer( lnewval ? true : false );
    return new BLong( 1 );
  }
  case MTH_REMOVEREPORTABLE:
  {
    if ( !ex.hasParams( 2 ) )
      return new BError( "Not enough parameters" );
    int repserial, gameclock;
    if ( ex.getParam( 0, repserial ) && ex.getParam( 1, gameclock ) )
    {
      set_dirty();
      clear_reportable( repserial, gameclock );
      return new BLong( 1 );
    }
    break;
  }
  case MTH_GETGOTTENITEM:
    if ( has_gotten_item() )
      return new Module::EItemRefObjImp( gotten_item() );
    return new BError( "Gotten Item nullptr" );
    break;
  case MTH_CLEARGOTTENITEM:
    if ( has_gotten_item() )
    {
      clear_gotten_item();
      return new BLong( 1 );
    }
    return new BError( "No Gotten Item" );
    break;
  case MTH_SETWARMODE:
  {
    int newmode;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, newmode, 0, 1 ) )
    {
      set_warmode( ( newmode == 0 ) ? false : true );
      // FIXME: Additional checks needed?
      if ( client )
        send_warmode();
      return new BLong( warmode() );
    }
    break;
  }
  case MTH_GETCORPSE:
  {
    Core::UCorpse* corpse_obj =
        static_cast<Core::UCorpse*>( Core::system_find_item( last_corpse ) );
    if ( corpse_obj != nullptr && !corpse_obj->orphan() )
      return new Module::EItemRefObjImp( corpse_obj );
    return new BError( "No corpse was found." );
    break;
  }
  case MTH_SET_SWINGTIMER:
  {
    int time;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, time ) )
    {
      if ( time < 0 )
        return new BError( "Time must be >= 0" );
      Core::polclock_t clocks;
      clocks = ( time * Core::POLCLOCKS_PER_SEC ) / 1000;
      return new BLong( manual_set_swing_timer( clocks ) ? 1 : 0 );
    }
    break;
  }
  case MTH_ATTACK_ONCE:
  {
    Character* chr;
    if ( ex.hasParams( 1 ) )
    {
      if ( ex.getCharacterParam( 0, chr ) )
      {
        if ( dead() )
          return new BError( "Character is dead" );
        if ( is_attackable( chr ) )
          attack( chr );
        else
          return new BError( "Opponent is not attackable" );
      }
      else
        return new BError( "Invalid parameter type" );
    }
    else
    {
      chr = get_attackable_opponent();
      if ( chr != nullptr )
      {
        if ( !dead() )
          attack( chr );
        else
          return new BError( "Character is dead" );
      }
      else
        return new BError( "No opponent" );
    }
    return new BLong( 1 );
    break;
  }
  case MTH_KILL:
    if ( ex.hasParams( 1 ) )
    {
      Character* chr;
      if ( ex.getCharacterParam( 0, chr ) )
        chr->repsys_on_damage( this );
    }
    if ( dead() )
      return new BError( "That is already dead!" );

    die();
    return new BLong( 1 );
    break;
  case MTH_SETFACING:
  {
    int flags = 0;
    Core::UFACING i_facing;

    if ( ex.hasParams( 2 ) && !ex.getParam( 1, flags, 0, 1 ) )
      return new BError( "Invalid flags for parameter 1" );

    BObjectImp* param0 = ex.getParamImp( 0 );
    if ( param0->isa( BObjectImp::OTString ) )
    {
      const char* szDir = ex.paramAsString( 0 );
      if ( DecodeFacing( szDir, i_facing ) == false )
        return new BError( "Invalid string for parameter 0" );
    }
    else if ( param0->isa( BObjectImp::OTLong ) )
    {
      BLong* blong = static_cast<BLong*>( param0 );
      i_facing = static_cast<Core::UFACING>( blong->value() & PKTIN_02_FACING_MASK );
    }
    else
      return new BError( "Invalid type for parameter 0" );

    if ( !face( i_facing, flags ) )
      return new BLong( 0 );

    on_facing_changed();
    return new BLong( 1 );
    break;
  }
  case MTH_COMPAREVERSION:
    if ( client != nullptr )
    {
      if ( !ex.hasParams( 1 ) )
        return new BError( "Not enough parameters" );
      const String* pstr;
      if ( ex.getStringParam( 0, pstr ) )
        return new BLong( client->compareVersion( pstr->getStringRep() ) ? 1 : 0 );
      return new BError( "Invalid parameter type" );
    }
    return new BError( "No client attached" );
    break;
  case MTH_SETAGGRESSORTO:
    if ( ex.hasParams( 1 ) )
    {
      Character* chr;
      if ( ex.getCharacterParam( 0, chr ) )
      {
        // make_aggressor_to handles the updating
        this->make_aggressor_to( chr );
        return new BLong( 1 );
      }
      return new BError( "Invalid parameter type" );
    }
    return new BError( "Not enough parameters" );
    break;
  case MTH_SETLAWFULLYDAMAGEDTO:
    if ( ex.hasParams( 1 ) )
    {
      Character* chr;
      if ( ex.getCharacterParam( 0, chr ) )
      {
        // make_lawfullydamaged_to handled the updating
        this->make_lawfullydamaged_to( chr );
        return new BLong( 1 );
      }
      return new BError( "Invalid parameter type" );
    }
    return new BError( "Not enough parameters" );
    break;
  case MTH_CLEARAGGRESSORTO:
    if ( ex.hasParams( 1 ) )
    {
      Character* chr;
      if ( ex.getCharacterParam( 0, chr ) )
      {
        this->remove_as_aggressor_to( chr );
        return new BLong( 1 );
      }
      return new BError( "Invalid parameter type" );
    }
    return new BError( "Not enough parameters" );
    break;
  case MTH_CLEARLAWFULLYDAMAGEDTO:
    if ( ex.hasParams( 1 ) )
    {
      Character* chr;
      if ( ex.getCharacterParam( 0, chr ) )
      {
        this->remove_as_lawful_damager( chr );
        return new BLong( 1 );
      }
      return new BError( "Invalid parameter type" );
    }
    return new BError( "Not enough parameters" );
    break;
  case MTH_DEAF:
  {
    int duration;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, duration ) )
    {
      set_dirty();
      if ( duration == -1 )
      {
        deafened_until( ~0u );
        return new BLong( -1 );
      }
      else if ( duration == 0 )
        deafened_until( 0 );
      else
        deafened_until( Core::read_gameclock() + duration );
      return new BLong( deafened_until() );
    }
    break;
  }
  case MTH_DISABLE_SKILLS_FOR:
  {
    int duration;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, duration ) )
    {
      if ( duration < 0 )
        return new BError( "Duration must be >= 0" );
      disable_skills_until( Core::poltime() + duration );
      return new BLong( static_cast<int>( disable_skills_until() ) );
    }
    break;
  }
  case MTH_ADD_BUFF:
  {
    u16 icon;
    u16 duration;
    u32 cl_name;
    u32 cl_descr;
    const String* text;

    if ( !ex.hasParams( 5 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, icon ) && ex.getParam( 1, duration ) && ex.getParam( 2, cl_name ) &&
         ex.getParam( 3, cl_descr ) && ex.getUnicodeStringParam( 4, text ) )
    {
      if ( !( icon && cl_name && cl_descr ) )
        return new BError( "Invalid parameters" );

      if ( text->length() > SPEECH_MAX_LEN )
        return new BError( "Text exceeds maximum size." );

      addBuff( icon, duration, cl_name, cl_descr, text->value() );
      return new BLong( 1 );
    }
    break;
  }
  case MTH_CLEAR_BUFFS:
  {
    clearBuffs();

    return new BLong( 1 );
    break;
  }
  case MTH_DEL_BUFF:
  {
    u16 icon;

    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, icon ) )
    {
      if ( !icon )
        return new BError( "Invalid parameter" );

      if ( !delBuff( icon ) )
        return new BError( "Buff not found" );

      return new BLong( 1 );
    }
    break;
  }
  default:
    return nullptr;
  }
  return new BError( "Invalid parameter type" );
}


BObjectImp* Character::script_method( const char* methodname, Core::UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  return nullptr;
}

BObjectImp* Character::custom_script_method( const char* methodname, Core::UOExecutor& ex )
{
  // TODO uoclient entry deprecated
  if ( Core::networkManager.uoclient_general.method_script != nullptr )
  {
    unsigned PC;
    if ( Core::networkManager.uoclient_general.method_script->FindExportedFunction(
             methodname, static_cast<unsigned int>( ex.numParams() + 1 ), PC ) )
      return Core::networkManager.uoclient_general.method_script->call( PC, make_ref(),
                                                                        ex.fparams );
  }
  return Core::gamestate.system_hooks.call_script_method( methodname, &ex, this );
}

ObjArray* Character::GetReportables() const
{
  std::unique_ptr<ObjArray> arr( new ObjArray );

  for ( ReportableList::const_iterator itr = reportable_.begin(), end = reportable_.end();
        itr != end; ++itr )
  {
    const reportable_t& rt = ( *itr );

    std::unique_ptr<BObjectImp> kmember( nullptr );
    Character* killer = Core::system_find_mobile( rt.serial );
    if ( killer )
      kmember.reset( new Module::EOfflineCharacterRefObjImp( killer ) );
    else
      kmember.reset( new BError( "Mobile not found" ) );

    std::unique_ptr<BStruct> elem( new BStruct );
    elem->addMember( "serial", new BLong( rt.serial ) );
    elem->addMember( "killer", kmember.release() );
    elem->addMember( "gameclock", new BLong( rt.polclock ) );

    arr->addElement( elem.release() );
  }
  return arr.release();
}

ObjArray* Character::GetAggressorTo() const
{
  std::unique_ptr<ObjArray> arr( new ObjArray );

  for ( Character::MobileCont::const_iterator itr = aggressor_to_.begin(),
                                              end = aggressor_to_.end();
        itr != end; ++itr )
  {
    std::unique_ptr<BObjectImp> member( nullptr );
    Character* chr = Core::system_find_mobile( ( *itr ).first->serial );
    if ( chr )
      member.reset( new Module::EOfflineCharacterRefObjImp( chr ) );
    else
      member.reset( new BError( "Mobile not found" ) );

    std::unique_ptr<BStruct> elem( new BStruct );
    elem->addMember( "serial", new BLong( ( *itr ).first->serial ) );
    elem->addMember( "ref", member.release() );
    elem->addMember(
        "seconds", new BLong( ( ( *itr ).second - Core::polclock() ) / Core::POLCLOCKS_PER_SEC ) );

    arr->addElement( elem.release() );
  }
  return arr.release();
}

ObjArray* Character::GetLawFullyDamaged() const
{
  std::unique_ptr<ObjArray> arr( new ObjArray );

  for ( Character::MobileCont::const_iterator itr = lawfully_damaged_.begin(),
                                              end = lawfully_damaged_.end();
        itr != end; ++itr )
  {
    std::unique_ptr<BObjectImp> member( nullptr );
    Character* chr = Core::system_find_mobile( ( *itr ).first->serial );
    if ( chr )
      member.reset( new Module::EOfflineCharacterRefObjImp( chr ) );
    else
      member.reset( new BError( "Mobile not found" ) );

    std::unique_ptr<BStruct> elem( new BStruct );
    elem->addMember( "serial", new BLong( ( *itr ).first->serial ) );
    elem->addMember( "ref", member.release() );
    elem->addMember(
        "seconds", new BLong( ( ( *itr ).second - Core::polclock() ) / Core::POLCLOCKS_PER_SEC ) );

    arr->addElement( elem.release() );
  }
  return arr.release();
}

BObjectImp* NPC::get_script_member_id( const int id ) const
{
  BObjectImp* imp = base::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_SCRIPT:
    return new String( script );
    break;
  case MBR_NPCTEMPLATE:
    return new String( template_name );
    break;
  case MBR_MASTER:
  {
    Character* master = master_.get();
    if ( master != nullptr && !master->orphan() )
      return new Module::EOfflineCharacterRefObjImp( master );
    return new BLong( 0 );
    break;
  }

  case MBR_PROCESS:
    if ( ex )
      return new Core::ScriptExObjImp( ex );
    return new BError( "No script running" );
    break;

  case MBR_EVENTMASK:
    if ( ex )
      return new BLong( ex->eventmask );
    return new BError( "No script running" );
    break;

  case MBR_SPEECH_COLOR:
    return new BLong( speech_color() );
    break;
  case MBR_SPEECH_FONT:
    return new BLong( speech_font() );
    break;
  case MBR_USE_ADJUSTMENTS:
    return new BLong( use_adjustments() ? 1 : 0 );
    break;
  case MBR_RUN_SPEED:
    return new BLong( run_speed );
    break;
  case MBR_ALIGNMENT:
    return new BLong( this->template_.alignment );
    break;
  case MBR_SAVEONEXIT:
    return new BLong( saveonexit() );
    break;
  case MBR_NO_DROP_EXCEPTION:
    return new BLong( no_drop_exception() );
  default:
    return nullptr;
  }
}

BObjectImp* NPC::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* NPC::set_script_member_id( const int id, const std::string& value )
{
  BObjectImp* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;
  switch ( id )
  {
  case MBR_SCRIPT:
    return new String( script = value );
  default:
    return nullptr;
  }
}

BObjectImp* NPC::set_script_member( const char* membername, const std::string& value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* NPC::set_script_member_id( const int id, int value )
{
  BObjectImp* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;
  switch ( id )
  {
  case MBR_SPEECH_COLOR:
    speech_color( static_cast<unsigned short>( value ) );
    return new BLong( speech_color() );
  case MBR_SPEECH_FONT:
    speech_font( static_cast<unsigned short>( value ) );
    return new BLong( speech_font() );
  case MBR_USE_ADJUSTMENTS:
    use_adjustments( value ? true : false );
    return new BLong( use_adjustments() );
  case MBR_RUN_SPEED:
    return new BLong( run_speed = static_cast<unsigned short>( value ) );
  case MBR_SAVEONEXIT:
    saveonexit( value ? true : false );
    return new BLong( saveonexit() );
  case MBR_NO_DROP_EXCEPTION:
    no_drop_exception( value ? true : false );
    return new BLong( no_drop_exception() );
  default:
    return nullptr;
  }
}
BObjectImp* NPC::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* NPC::script_method_id( const int id, Core::UOExecutor& executor )
{
  BObjectImp* imp = base::script_method_id( id, executor );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MTH_SETMASTER:
  {
    if ( executor.numParams() != 1 )
      return new BError( "Not enough parameters" );
    Character* chr;
    set_dirty();
    if ( executor.getCharacterParam( 0, chr ) )
    {
      master_.set( chr );
      return new BLong( 1 );
    }
    else
    {
      master_.clear();
      return new BLong( 0 );
    }
    break;
  }
  default:
    return nullptr;
  }
}

BObjectImp* NPC::script_method( const char* methodname, Core::UOExecutor& executor )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, executor );
  return nullptr;
}

BObjectImp* NPC::custom_script_method( const char* methodname, Core::UOExecutor& executor )
{
  if ( template_.method_script != nullptr )
  {
    unsigned PC;
    if ( template_.method_script->FindExportedFunction(
             methodname, static_cast<unsigned int>( executor.numParams() + 1 ), PC ) )
      return template_.method_script->call( PC, make_ref(), executor.fparams );
  }
  return Core::gamestate.system_hooks.call_script_method( methodname, &executor, this );
}
}  // namespace Mobile
namespace Core
{
using namespace Bscript;

BObjectImp* ULockable::get_script_member_id( const int id ) const
{
  BObjectImp* imp = Item::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_LOCKED:
    return new BLong( locked() ? 1 : 0 );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* ULockable::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* ULockable::set_script_member_id( const int id, int value )
{
  BObjectImp* imp = Item::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;
  switch ( id )
  {
  case MBR_LOCKED:
    locked( value ? true : false );
    return new BLong( locked() );
  default:
    return nullptr;
  }
}

BObjectImp* ULockable::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* UContainer::get_script_member_id( const int id ) const
{
  BObjectImp* imp = base::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_MAX_ITEMS_MOD:
    return new BLong( max_items_mod() );
    break;
  case MBR_MAX_WEIGHT_MOD:
    return new BLong( max_weight_mod() );
    break;
  case MBR_MAX_SLOTS_MOD:
    return new BLong( max_slots_mod() );
    break;
  case MBR_NO_DROP_EXCEPTION:
    return new BLong( no_drop_exception() );
  default:
    return nullptr;
  }
}

BObjectImp* UContainer::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* UContainer::set_script_member_id( const int id, int value )
{
  BObjectImp* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;
  switch ( id )
  {
  case MBR_MAX_ITEMS_MOD:
    max_items_mod( static_cast<s16>( value ) );
    break;
  case MBR_MAX_WEIGHT_MOD:
    max_weight_mod( static_cast<s16>( value ) );
    break;
  case MBR_MAX_SLOTS_MOD:
    max_slots_mod( static_cast<s8>( value ) );
    break;
  case MBR_NO_DROP_EXCEPTION:
    no_drop_exception( value ? true : false );
    return new BLong( no_drop_exception() );
  default:
    return nullptr;
  }
  return new BLong( value );
}

BObjectImp* UContainer::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* UCorpse::get_script_member_id( const int id ) const
{
  BObjectImp* imp = base::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_CORPSETYPE:
    return new BLong( corpsetype );
    break;
  case MBR_OWNERSERIAL:
    return new BLong( ownerserial );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* UCorpse::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* Spellbook::script_method_id( const int id, Core::UOExecutor& ex )
{
  BObjectImp* imp = base::script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MTH_HASSPELL:
  {
    int sid;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, sid ) )
    {
      if ( sid <= 0 )
        return new BError( "SpellID must be >= 1" );
      if ( this->has_spellid( static_cast<unsigned int>( sid ) ) )
        return new BLong( 1 );
      else
        return new BLong( 0 );
    }
    break;
  }
  case MTH_SPELLS:
  {
    std::unique_ptr<ObjArray> arr( new ObjArray );
    for ( u16 i = 0; i < 64; ++i )
    {
      unsigned int sid;

      // Check for Mysticism spells here
      if ( this->spell_school == 3 )
        sid = 678 + i;
      else
        sid = this->spell_school * 100 + i + 1;

      if ( this->has_spellid( sid ) )
        arr->addElement( new BLong( sid ) );
    }
    return arr.release();
    break;
  }
  case MTH_REMOVESPELL:
  {
    int sid;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, sid ) )
    {
      if ( sid <= 0 )
        return new BError( "SpellID must be >= 1" );
      if ( this->remove_spellid( static_cast<unsigned int>( sid ) ) )
        return new BLong( 1 );
      else
        return new BLong( 0 );
    }
    break;
  }
  case MTH_ADDSPELL:
  {
    int sid;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( ex.getParam( 0, sid ) )
    {
      if ( sid <= 0 )
        return new BError( "SpellID must be >= 1" );
      if ( this->add_spellid( static_cast<unsigned int>( sid ) ) )
        return new BLong( 1 );
      else
        return new BLong( 0 );
    }
    break;
  }

  default:
    return nullptr;
  }
  return new BError( "Invalid parameter type" );
}

BObjectImp* Spellbook::script_method( const char* methodname, Core::UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  return nullptr;
}
}  // namespace Core
namespace Multi
{
using namespace Bscript;

BObjectImp* UBoat::make_ref()
{
  return new Module::EUBoatRefObjImp( this );
}

BObjectImp* UMulti::make_ref()
{
  return new Module::EMultiRefObjImp( this );
}

BObjectImp* UBoat::get_script_member_id( const int id ) const
{
  BObjectImp* imp = base::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_TILLERMAN:
  {
    Item* cp = tillerman;
    if ( cp != nullptr )
      return new Module::EItemRefObjImp( cp );
    return new BError( std::string( "This ship doesn't have that component" ) );
    break;
  }
  case MBR_PORTPLANK:
  {
    Item* cp = portplank;
    if ( cp != nullptr )
      return new Module::EItemRefObjImp( cp );
    return new BError( std::string( "This ship doesn't have that component" ) );
    break;
  }
  case MBR_STARBOARDPLANK:
  {
    Item* cp = starboardplank;
    if ( cp != nullptr )
      return new Module::EItemRefObjImp( cp );
    return new BError( std::string( "This ship doesn't have that component" ) );
    break;
  }
  case MBR_HOLD:
  {
    Item* cp = hold;
    if ( cp != nullptr )
      return new Module::EItemRefObjImp( cp );
    return new BError( std::string( "This ship doesn't have that component" ) );
    break;
  }
  case MBR_ROPE:
    return component_list( COMPONENT_ROPE );
    break;
  case MBR_WHEEL:
    return component_list( COMPONENT_WHEEL );
    break;
  case MBR_HULL:
    return component_list( COMPONENT_HULL );
    break;
  case MBR_TILLER:
    return component_list( COMPONENT_TILLER );
    break;
  case MBR_RUDDER:
    return component_list( COMPONENT_RUDDER );
    break;
  case MBR_SAILS:
    return component_list( COMPONENT_SAILS );
    break;
  case MBR_STORAGE:
    return component_list( COMPONENT_STORAGE );
    break;
  case MBR_WEAPONSLOT:
    return component_list( COMPONENT_WEAPONSLOT );
    break;
  case MBR_COMPONENTS:
    return component_list( COMPONENT_ALL );
    break;
  case MBR_ITEMS:
    return items_list();
    break;
  case MBR_MOBILES:
    return mobiles_list();
    break;
  case MBR_HAS_OFFLINE_MOBILES:
    return new BLong( has_offline_mobiles() ? 1 : 0 );
    break;
  case MBR_MULTIID:
    return new BLong( multiid );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* UBoat::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* UBoat::script_method_id( const int id, Core::UOExecutor& ex )
{
  BObjectImp* imp = base::script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MTH_MOVE_OFFLINE_MOBILES:
  {
    if ( ex.numParams() == 3 )
    {
      Core::Pos3d newpos_xyz;
      if ( ex.getPos3dParam( 0, 1, 2, &newpos_xyz, realm() ) )
      {
        set_dirty();
        move_offline_mobiles( Core::Pos4d( newpos_xyz, realm() ) );
        return new BLong( 1 );
      }
      else
        return new BError( "Invalid parameter type" );
    }
    else if ( ex.numParams() == 4 )
    {
      Core::Pos4d newpos;
      if ( ex.getPos4dParam( 0, 1, 2, 3, &newpos ) )
      {
        set_dirty();
        move_offline_mobiles( newpos );
        return new BLong( 1 );
      }
      else
        return new BError( "Invalid parameter type" );
    }
    else
      return new BError( "Not enough parameters" );

    break;
  }
  default:
    return nullptr;
  }
}

BObjectImp* UBoat::script_method( const char* methodname, Core::UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  return nullptr;
}

BObjectImp* UPlank::get_script_member_id( const int id ) const
{
  switch ( id )
  {
  case MBR_MULTI:
    if ( boat_.get() )
      return new Module::EUBoatRefObjImp( boat_.get() );
    return new BError( "No boat attached" );
    break;
  }
  return base::get_script_member_id( id );
}

/* UObject defines a 'multi' also, so we have to trap that here first */
BObjectImp* UPlank::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return base::get_script_member( membername );
}
}  // namespace Multi
namespace Core
{
using namespace Bscript;

BObjectImp* Map::get_script_member_id( const int id ) const
{
  BObjectImp* imp = base::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_XEAST:
    return new BLong( area.se().x() );
    break;
  case MBR_XWEST:
    return new BLong( area.nw().x() );
    break;
  case MBR_YNORTH:
    return new BLong( area.nw().y() );
    break;
  case MBR_YSOUTH:
    return new BLong( area.se().y() );
    break;
  case MBR_GUMPWIDTH:
    return new BLong( gumpwidth );
    break;
  case MBR_GUMPHEIGHT:
    return new BLong( gumpheight );
    break;
  case MBR_FACETID:
    return new BLong( facetid );
    break;
  case MBR_EDITABLE:
    return new BLong( editable ? 1 : 0 );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* Map::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* Map::set_script_member_id( const int id, int value )
{
  BObjectImp* imp = base::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;
  switch ( id )
  {
  case MBR_XEAST:
    area.se( Pos2d( area.se() ).x( static_cast<unsigned short>( value ) ) );
    return new BLong( area.se().x() );
  case MBR_XWEST:
    area.nw( Pos2d( area.nw() ).x( static_cast<unsigned short>( value ) ) );
    return new BLong( area.nw().x() );
  case MBR_YNORTH:
    area.nw( Pos2d( area.nw() ).y( static_cast<unsigned short>( value ) ) );
    return new BLong( area.nw().y() );
  case MBR_YSOUTH:
    area.se( Pos2d( area.se() ).y( static_cast<unsigned short>( value ) ) );
    return new BLong( area.se().y() );
  case MBR_GUMPWIDTH:
    return new BLong( gumpwidth = static_cast<unsigned short>( value ) );
  case MBR_GUMPHEIGHT:
    return new BLong( gumpheight = static_cast<unsigned short>( value ) );
  case MBR_FACETID:
    return new BLong( facetid = static_cast<unsigned short>( value ) );
  case MBR_EDITABLE:
    return new BLong( editable = value ? true : false );
  default:
    return nullptr;
  }
}
BObjectImp* Map::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* UObject::script_method_id( const int id, Core::UOExecutor& ex )
{
  switch ( id )
  {
  case MTH_ISA:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    int isatype;
    if ( ex.getParam( 0, isatype ) )
      return new BLong( script_isa( static_cast<unsigned>( isatype ) ) );
    break;
  }
  case MTH_SET_MEMBER:
  {
    if ( !ex.hasParams( 2 ) )
      return new BError( "Not enough parameters" );
    BObjectImp* objimp;
    const String* mname;
    if ( ex.getStringParam( 0, mname ) && ( objimp = ex.getParamImp( 1 ) ) != nullptr )
    {
      BObjectImp* ret;
      if ( objimp->isa( BObjectImp::OTLong ) )
      {
        BLong* lng = static_cast<BLong*>( objimp );
        ret = set_script_member( mname->value().c_str(), lng->value() );
      }
      else if ( objimp->isa( BObjectImp::OTDouble ) )
      {
        Double* dbl = static_cast<Double*>( objimp );
        ret = set_script_member_double( mname->value().c_str(), dbl->value() );
      }
      else if ( objimp->isa( BObjectImp::OTString ) )
      {
        String* str = static_cast<String*>( objimp );
        ret = set_script_member( mname->value().c_str(), str->value() );
      }
      else
        return new BError( "Invalid value type" );

      if ( ret != nullptr )
        return ret;
      else
      {
        std::string message = std::string( "Member " ) + std::string( mname->value() ) +
                              std::string( " not found on that object" );
        return new BError( message );
      }
    }
    break;
  }
  case MTH_GET_MEMBER:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );

    const String* mname;
    if ( ex.getStringParam( 0, mname ) )
    {
      BObjectImp* ret = get_script_member( mname->value().c_str() );
      if ( ret != nullptr )
        return ret;
      else
      {
        std::string message = std::string( "Member " ) + std::string( mname->value() ) +
                              std::string( " not found on that object" );
        return new BError( message );
      }
    }
    break;
  }
  default:
  {
    bool changed = false;
    BObjectImp* imp = CallPropertyListMethod_id( proplist_, id, ex, changed );
    if ( changed )
      set_dirty();
    return imp;
  }
  }
  return new BError( "Invalid parameter type" );
}


BObjectImp* UObject::script_method( const char* methodname, Core::UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  else
  {
    bool changed = false;
    BObjectImp* imp = CallPropertyListMethod( proplist_, methodname, ex, changed );
    if ( changed )
      set_dirty();

    return imp;
  }
}

BObjectImp* UObject::custom_script_method( const char* methodname, Core::UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  return Core::gamestate.system_hooks.call_script_method( methodname, &ex, this );
}

BObjectImp* UDoor::get_script_member_id( const int id ) const
{
  BObjectImp* imp = ULockable::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_ISOPEN:
    return new BLong( is_open() ? 1 : 0 );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* UDoor::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* UDoor::script_method_id( const int id, Core::UOExecutor& ex )
{
  BObjectImp* imp = base::script_method_id( id, ex );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MTH_OPEN:
    open();
    break;
  case MTH_CLOSE:
    close();
    break;
  case MTH_TOGGLE:
    toggle();
    break;
  default:
    return nullptr;
  }
  return new BLong( 1 );
}

BObjectImp* UDoor::script_method( const char* methodname, Core::UOExecutor& ex )
{
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->script_method_id( objmethod->id, ex );
  return nullptr;
}
}  // namespace Core
namespace Items
{
using namespace Bscript;

BObjectImp* Equipment::get_script_member_id( const int id ) const
{
  BObjectImp* imp = Item::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_INTRINSIC:
    return new BLong( is_intrinsic() );
    break;
  default:
    return nullptr;
  }
}
BObjectImp* Equipment::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* Equipment::set_script_member_id( const int id, int value )
{
  BObjectImp* imp = Item::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  return nullptr;
  /*
  switch(id)
  {
  default: return nullptr;
  }
  */
}
BObjectImp* Equipment::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* Equipment::set_script_member_id_double( const int id, double value )
{
  BObjectImp* imp = Item::set_script_member_id_double( id, value );
  if ( imp != nullptr )
    return imp;

  return nullptr;
  /*
  switch(id)
  {
  default: return nullptr;
  }
  */
}
BObjectImp* Equipment::set_script_member_double( const char* membername, double value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id_double( objmember->id, value );
  return nullptr;
}


BObjectImp* UWeapon::get_script_member_id( const int id ) const
{
  BObjectImp* imp = Equipment::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_DMG_MOD:
    return new BLong( damage_mod() );
    break;
  case MBR_SPEED_MOD:
    return new BLong( speed_mod() );
    break;
  case MBR_ATTRIBUTE:
    return new String( attribute().name );
    break;
  case MBR_HITSCRIPT:
    return new String( hit_script_.relativename( tmpl->pkg ) );
    break;
  default:
    return nullptr;
  }
}
BObjectImp* UWeapon::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* UWeapon::set_script_member_id( const int id, const std::string& value )
{
  BObjectImp* imp = Item::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_HITSCRIPT:
    set_hit_script( value );
    return new BLong( 1 );
  default:
    return nullptr;
  }
}
BObjectImp* UWeapon::set_script_member( const char* membername, const std::string& value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* UWeapon::set_script_member_id( const int id, int value )
{
  if ( is_intrinsic() )
    return new BError( "Cannot alter an instrinsic NPC weapon member values" );  // executor won't
                                                                                 // return this to
                                                                                 // the script
                                                                                 // currently.

  BObjectImp* imp = Equipment::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_DMG_MOD:
    damage_mod( static_cast<s16>( value ) );
    break;
  case MBR_SPEED_MOD:
    speed_mod( static_cast<s16>( value ) );
    break;
  default:
    return nullptr;
  }
  return new BLong( value );
}

BObjectImp* UWeapon::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* UWeapon::set_script_member_id_double( const int id, double value )
{
  if ( is_intrinsic() )
    return new BError( "Cannot alter an instrinsic NPC weapon member values" );  // executor won't
                                                                                 // return this to
                                                                                 // the script
                                                                                 // currently.
  return base::set_script_member_id_double( id, value );
}

BObjectImp* UWeapon::set_script_member_double( const char* membername, double value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id_double( objmember->id, value );
  return base::set_script_member_double( membername, value );
}

BObjectImp* UArmor::get_script_member_id( const int id ) const
{
  BObjectImp* imp = Equipment::get_script_member_id( id );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_AR_MOD:
    return new BLong( ar_mod() );
    break;
  case MBR_AR:
    return new BLong( ar() );
    break;
  case MBR_AR_BASE:
    return new BLong( ar_base() );
    break;
  case MBR_ONHIT_SCRIPT:
    return new String( onhitscript_.relativename( tmpl->pkg ) );
    break;
  default:
    return nullptr;
  }
}

BObjectImp* UArmor::get_script_member( const char* membername ) const
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_script_member_id( objmember->id );
  return nullptr;
}

BObjectImp* UArmor::set_script_member_id( const int id, const std::string& value )
{
  BObjectImp* imp = Item::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;
  switch ( id )
  {
  case MBR_ONHIT_SCRIPT:
    set_onhitscript( value );
    return new BLong( 1 );
  default:
    return nullptr;
  }
}

BObjectImp* UArmor::set_script_member( const char* membername, const std::string& value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}

BObjectImp* UArmor::set_script_member_id( const int id, int value )
{
  BObjectImp* imp = Equipment::set_script_member_id( id, value );
  if ( imp != nullptr )
    return imp;

  switch ( id )
  {
  case MBR_AR_MOD:
    this->ar_mod( static_cast<s16>( value ) );
    if ( container != nullptr )
    {
      if ( Core::IsCharacter( container->serial ) )
      {
        Mobile::Character* chr = container->toplevel_chr();
        if ( chr != nullptr )
          Mobile::ARUpdater::on_change( chr );
      }
    }
    return new BLong( value );
  default:
    return nullptr;
  }
}

BObjectImp* UArmor::set_script_member( const char* membername, int value )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_script_member_id( objmember->id, value );
  return nullptr;
}
}  // namespace Items
namespace Module
{
using namespace Bscript;

const char* EClientRefObjImp::typeOf() const
{
  return "ClientRef";
}

u8 EClientRefObjImp::typeOfInt() const
{
  return OTClientRef;
}

BObjectImp* EClientRefObjImp::copy() const
{
  if ( value().exists() )
    return value()->make_ref();
  return new BError( "Client is disconnected" );
}

bool EClientRefObjImp::isTrue() const
{
  return ( value().exists() && value()->isConnected() );
}

bool EClientRefObjImp::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &eclientrefobjimp_type )
    {
      const EClientRefObjImp* clientref_imp =
          Clib::explicit_cast<const EClientRefObjImp*, const BApplicObjBase*>( aob );

      if ( clientref_imp->value().exists() && value().exists() &&
           ( clientref_imp->value()->chr != nullptr ) && ( value()->chr != nullptr ) )
        return ( clientref_imp->value()->chr->serial == value()->chr->serial );
      return false;
    }
    return false;
  }
  else if ( objimp.isa( Bscript::BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const Bscript::BBoolean&>( objimp ).isTrue();
  return false;
}

BObjectRef EClientRefObjImp::get_member_id( const int id )
{
  if ( ( !obj_.exists() ) || ( !obj_->isConnected() ) )
    return BObjectRef( new BError( "Client not ready or disconnected" ) );

  switch ( id )
  {
  case MBR_ACCTNAME:
    if ( obj_->acct != nullptr )
      return BObjectRef( new String( obj_->acct->name() ) );
    return BObjectRef( new BError( "Not attached to an account" ) );
    break;
  case MBR_IP:
    return BObjectRef( new String( obj_->ipaddrAsString() ) );
    break;
  case MBR_CLIENTVERSION:
    return BObjectRef( new String( obj_->getversion() ) );
    break;
  case MBR_CLIENTVERSIONDETAIL:
  {
    std::unique_ptr<BStruct> info( new BStruct );
    Network::VersionDetailStruct version = obj_->getversiondetail();
    info->addMember( "major", new BLong( version.major ) );
    info->addMember( "minor", new BLong( version.minor ) );
    info->addMember( "rev", new BLong( version.rev ) );
    info->addMember( "patch", new BLong( version.patch ) );
    return BObjectRef( info.release() );
  }
  break;
  case MBR_CLIENTINFO:
    return BObjectRef( obj_->getclientinfo() );
    break;
  case MBR_CLIENTTYPE:
    return BObjectRef( new BLong( obj_->ClientType ) );
    break;
  case MBR_UO_EXPANSION_CLIENT:
    return BObjectRef( new BLong( obj_->UOExpansionFlagClient ) );
    break;
  case MBR_LAST_ACTIVITY_AT:
    return BObjectRef( new BLong( obj_->last_activity_at ) );
    break;
  case MBR_LAST_PACKET_AT:
    return BObjectRef( new BLong( obj_->last_packet_at ) );
    break;
  case MBR_PORT:
    return BObjectRef( new BLong( obj_->listen_port ) );
    break;
  }

  return base::get_member_id( id );
}

BObjectRef EClientRefObjImp::get_member( const char* membername )
{
  if ( ( !obj_.exists() ) || ( !obj_->isConnected() ) )
    return BObjectRef( new BError( "Client not ready or disconnected" ) );
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EClientRefObjImp::set_member( const char* membername, BObjectImp* value, bool copy )
{
  if ( !obj_.exists() || !obj_->isConnected() )
    return BObjectRef( new BError( "Client not ready or disconnected" ) );
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->set_member_id( objmember->id, value, copy );
  return BObjectRef( UninitObject::create() );
}

BObjectRef EClientRefObjImp::set_member_id( const int /*id*/, BObjectImp* /*value*/, bool /*copy*/ )
{
  if ( !obj_.exists() || !obj_->isConnected() )
    return BObjectRef( new BError( "Client not ready or disconnected" ) );
  return BObjectRef( UninitObject::create() );
}

BObjectImp* EClientRefObjImp::call_polmethod( const char* methodname, Core::UOExecutor& ex )
{
  if ( !obj_.exists() || !obj_->isConnected() )
    return new BError( "Client not ready or disconnected" );
  bool forcebuiltin{Executor::builtinMethodForced( methodname )};
  ObjMethod* objmethod = getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return this->call_polmethod_id( objmethod->id, ex, forcebuiltin );
  return Core::gamestate.system_hooks.call_script_method( methodname, &ex, this );
}

BObjectImp* EClientRefObjImp::call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                 bool forcebuiltin )
{
  if ( !obj_.exists() || !obj_->isConnected() )
    return new BError( "Client not ready or disconnected" );

  ObjMethod* mth = getObjMethod( id );
  if ( mth->overridden && !forcebuiltin )
  {
    auto* imp = Core::gamestate.system_hooks.call_script_method( mth->code, &ex, this );
    if ( imp )
      return imp;
  }
  switch ( id )
  {
  case MTH_COMPAREVERSION:
  {
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    const String* pstr;
    if ( ex.getStringParam( 0, pstr ) )
      return new BLong( obj_->compareVersion( pstr->getStringRep() ) ? 1 : 0 );
    return new BError( "Invalid parameter type" );
  }
  }

  return base::call_polmethod_id( id, ex );
}

SourcedEvent::SourcedEvent( Core::EVENTID type, Mobile::Character* source )
{
  addMember( "type", new BLong( type ) );
  addMember( "source", new Module::EOfflineCharacterRefObjImp( source ) );
}

SpeechEvent::SpeechEvent( Mobile::Character* speaker, const std::string& speech,
                          const char* texttype, std::string lang, Bscript::ObjArray* speechtokens )
{
  addMember( "type", new BLong( Core::EVID_SPOKE ) );
  addMember( "source", new Module::EOfflineCharacterRefObjImp( speaker ) );
  addMember( "text", new String( speech ) );
  addMember( "texttype", new String( texttype ) );
  if ( !lang.empty() )
    addMember( "langcode", new String( lang ) );
  if ( speechtokens != nullptr )
    addMember( "tokens", speechtokens );
}

DamageEvent::DamageEvent( Mobile::Character* source, unsigned short damage )
{
  addMember( "type", new BLong( Core::EVID_DAMAGED ) );

  if ( source != nullptr )
    addMember( "source", new Module::EOfflineCharacterRefObjImp( source ) );
  else
    addMember( "source", new BLong( 0 ) );

  addMember( "damage", new BLong( damage ) );
}

ItemGivenEvent::ItemGivenEvent( Mobile::Character* chr_givenby, Items::Item* item_given,
                                Mobile::NPC* chr_givento )
    : SourcedEvent( Core::EVID_ITEM_GIVEN, chr_givenby ), given_by_( nullptr )
{
  addMember( "item", new EItemRefObjImp( item_given ) );

  given_time_ = Core::read_gameclock();
  item_.set( item_given );
  cont_.set( item_given->container );
  given_by_.set( chr_givenby );

  item_given->setprop( "GivenBy", BLong::pack( chr_givenby->serial ) );
  item_given->setprop( "GivenTo", BLong::pack( chr_givento->serial ) );
  item_given->setprop( "GivenTime", BLong::pack( given_time_ ) );
}

ItemGivenEvent::~ItemGivenEvent()
{
  /* See if the item is still in the container it was in
       This means the AI script didn't do anything with it.
       */
  Items::Item* item = item_.get();
  Core::UContainer* cont = cont_.get();
  Mobile::Character* chr = given_by_.get();

  std::string given_time_str;
  if ( !item->getprop( "GivenTime", given_time_str ) )
    given_time_str = "";

  item->eraseprop( "GivenBy" );
  item->eraseprop( "GivenTo" );
  item->eraseprop( "GivenTime" );

  Bscript::BObject given_value( BObjectImp::unpack( given_time_str.c_str() ) );
  int gts = static_cast<BLong*>( given_value.impptr() )->value();

  if ( item->orphan() || cont->orphan() || chr->orphan() )
    return;

  if ( item->container == cont && Clib::tostring( given_time_ ) == Clib::tostring( gts ) )
  {
    Core::UContainer* backpack = chr->backpack();
    if ( backpack != nullptr && !chr->dead() )
    {
      if ( backpack->can_add( *item ) )
      {
        cont->remove( item );
        u8 newSlot = 1;
        if ( !backpack->can_add_to_slot( newSlot ) || !item->slot_index( newSlot ) )
        {
          const Core::Pos4d newpos = chr->pos();
          const Core::Pos4d oldpos = item->pos();
          item->setposition( newpos );
          add_item_to_world( item );
          register_with_supporting_multi( item );
          move_item( item, newpos, oldpos );
          return;
        }
        backpack->add( item );
        update_item_to_inrange( item );
        return;
      }
    }
    cont->remove( item );

    const Core::Pos4d newpos = chr->pos();
    const Core::Pos4d oldpos = item->pos();
    item->setposition( newpos );
    add_item_to_world( item );
    register_with_supporting_multi( item );
    move_item( item, newpos, oldpos );
  }
}
}  // namespace Module
namespace Core
{
bool UObject::script_isa( unsigned isatype ) const
{
  return ( isatype == POLCLASS_OBJECT );
}

bool ULockable::script_isa( unsigned isatype ) const
{
  return ( isatype == POLCLASS_LOCKABLE ) || base::script_isa( isatype );
}

bool UContainer::script_isa( unsigned isatype ) const
{
  return ( isatype == POLCLASS_CONTAINER ) || base::script_isa( isatype );
}

bool UCorpse::script_isa( unsigned isatype ) const
{
  return ( isatype == POLCLASS_CORPSE ) || base::script_isa( isatype );
}

bool UDoor::script_isa( unsigned isatype ) const
{
  return ( isatype == POLCLASS_DOOR ) || base::script_isa( isatype );
}

bool Spellbook::script_isa( unsigned isatype ) const
{
  return ( isatype == POLCLASS_SPELLBOOK ) || base::script_isa( isatype );
}

bool Map::script_isa( unsigned isatype ) const
{
  return ( isatype == POLCLASS_MAP ) || base::script_isa( isatype );
}
}  // namespace Core

namespace Items
{
bool Item::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_ITEM ) || base::script_isa( isatype );
}

bool Equipment::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_EQUIPMENT ) || base::script_isa( isatype );
}

bool UArmor::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_ARMOR ) || base::script_isa( isatype );
}

bool UWeapon::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_WEAPON ) || base::script_isa( isatype );
}
}  // namespace Items

namespace Mobile
{
bool Character::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_MOBILE ) || base::script_isa( isatype );
}

bool NPC::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_NPC ) || base::script_isa( isatype );
}
}  // namespace Mobile
namespace Multi
{
bool UMulti::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_MULTI ) || base::script_isa( isatype );
}

bool UBoat::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_BOAT ) || base::script_isa( isatype );
}

bool UHouse::script_isa( unsigned isatype ) const
{
  return ( isatype == Core::POLCLASS_HOUSE ) || base::script_isa( isatype );
}
}  // namespace Multi
}  // namespace Pol
