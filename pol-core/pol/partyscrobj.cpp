#include "partyscrobj.h"

#include <stddef.h>
#include <string>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/dict.h"
#include "../bscript/executor.h"
#include "../bscript/impstr.h"
#include "../bscript/objmembers.h"
#include "../bscript/objmethods.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../plib/systemstate.h"
#include "clfunc.h"
#include "fnsearch.h"
#include "globals/uvars.h"
#include "module/polsystemmod.h"
#include "module/uomod.h"
#include "party.h"
#include "polcfg.h"
#include "uoexec.h"
#include "uoscrobj.h"

namespace Pol
{
namespace Module
{
using namespace Bscript;

BApplicObjType party_type;

EPartyRefObjImp::EPartyRefObjImp( Core::PartyRef pref )
    : PolApplicObj<Core::PartyRef>( &party_type, pref ){};

const char* EPartyRefObjImp::typeOf() const
{
  return "PartyRef";
}
u8 EPartyRefObjImp::typeOfInt() const
{
  return OTPartyRef;
}

BObjectImp* EPartyRefObjImp::copy() const
{
  return new EPartyRefObjImp( obj_ );
}

bool EPartyRefObjImp::isTrue() const
{
  return ( obj_->test_size() );
}

bool EPartyRefObjImp::operator==( const BObjectImp& objimp ) const
{
  if ( objimp.isa( BObjectImp::OTApplicObj ) )
  {
    const BApplicObjBase* aob =
        Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

    if ( aob->object_type() == &party_type )
    {
      const EPartyRefObjImp* partyref_imp =
          Clib::explicit_cast<const EPartyRefObjImp*, const BApplicObjBase*>( aob );

      return ( partyref_imp->obj_->leader() == obj_->leader() );
    }
    else
      return false;
  }
  else if ( objimp.isa( Bscript::BObjectImp::OTBoolean ) )
    return isTrue() == static_cast<const Bscript::BBoolean&>( objimp ).isTrue();
  else
    return false;
}


BObjectRef EPartyRefObjImp::get_member( const char* membername )
{
  ObjMember* objmember = getKnownObjMember( membername );
  if ( objmember != nullptr )
    return this->get_member_id( objmember->id );
  else
    return BObjectRef( UninitObject::create() );
}

BObjectRef EPartyRefObjImp::get_member_id( const int id )  // id test
{
  switch ( id )
  {
  case MBR_MEMBERS:
  {
    std::unique_ptr<ObjArray> arr( new ObjArray );
    auto itr = obj_->_member_serials.begin();
    while ( itr != obj_->_member_serials.end() )
    {
      Mobile::Character* chr = Core::system_find_mobile( *itr );
      if ( chr != nullptr )
      {
        arr->addElement( new EOfflineCharacterRefObjImp( chr ) );
        ++itr;
      }
      else
        itr = obj_->_member_serials.erase( itr );
    }
    return BObjectRef( arr.release() );
  }

  case MBR_LEADER:
  {
    Mobile::Character* chr = Core::system_find_mobile( obj_->_leaderserial );
    if ( chr != nullptr )
      return BObjectRef( new EOfflineCharacterRefObjImp( chr ) );
    else
      return BObjectRef( new BLong( 0 ) );
  }

  case MBR_CANDIDATES:
  {
    std::unique_ptr<ObjArray> arr( new ObjArray );
    auto itr = obj_->_candidates_serials.begin();
    while ( itr != obj_->_candidates_serials.end() )
    {
      Mobile::Character* chr = Core::system_find_mobile( *itr );
      if ( chr != nullptr )
      {
        arr->addElement( new EOfflineCharacterRefObjImp( chr ) );
        ++itr;
      }
      else
        itr = obj_->_candidates_serials.erase( itr );
    }
    return BObjectRef( arr.release() );
  }

  default:
    return BObjectRef( UninitObject::create() );
  }
}

BObjectImp* EPartyRefObjImp::call_polmethod( const char* methodname, Core::UOExecutor& ex )
{
  bool forcebuiltin{Executor::builtinMethodForced( methodname )};
  Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return call_polmethod_id( objmethod->id, ex, forcebuiltin );
  auto* res = Core::gamestate.system_hooks.call_script_method( methodname, &ex, this );
  if ( res )
    return res;
  bool changed = false;
  return CallPropertyListMethod( obj_->_proplist, methodname, ex, changed );
}

BObjectImp* EPartyRefObjImp::call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                bool forcebuiltin )
{
  ObjMethod* mth = getObjMethod( id );
  if ( mth->overridden && !forcebuiltin )
  {
    auto* result = Core::gamestate.system_hooks.call_script_method( mth->code, &ex, this );
    if ( result )
      return result;
  }
  switch ( id )
  {
  case MTH_ADDMEMBER:
  {
    Mobile::Character* chr;
    bool is_candidate = false;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( !ex.getCharacterParam( 0, chr ) )
      return new BError( "Invalid parameter type" );
    if ( chr->has_party() )
      return new BError( "Character is already in a party" );
    if ( chr->has_candidate_of() )
    {
      is_candidate = chr->candidate_of() == obj_.get();
      if ( !is_candidate )
        return new BError( "Character is already candidate of a party" );
    }
    else if ( chr->has_offline_mem_of() )
      return new BError( "Character is already offline member of a party" );
    if ( !obj_->can_add() )
      return new BError( "Party is already full" );
    if ( !chr->has_active_client() )
      return new BError( "Character is offline" );
    if ( obj_->add_member( chr->serial ) )
    {
      if ( is_candidate )
        obj_->remove_candidate( chr->serial );
      chr->party( obj_.get() );
      if ( Core::settingsManager.party_cfg.Hooks.OnAddToParty )
        Core::settingsManager.party_cfg.Hooks.OnAddToParty->call( chr->make_ref() );
      if ( chr->has_active_client() )
        Core::send_sysmessage_cl( chr->client,
                                  Core::CLP_Added );  // You have been added to the party.
      obj_->send_msg_to_all( Core::CLP_Joined, chr->name(), chr );  //  : joined the party.
      obj_->send_member_list( nullptr );
      obj_->send_stats_on_add( chr );
    }

    return new BLong( static_cast<int>( obj_->_member_serials.size() ) );
  }

  case MTH_REMOVEMEMBER:
  {
    Mobile::Character* chr;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( !ex.getCharacterParam( 0, chr ) )
      return new BError( "Invalid parameter type" );
    if ( !obj_->is_member( chr->serial ) )
      return new BError( "Character is not in this party" );
    if ( obj_->is_leader( chr->serial ) )
      return new BError( "Character is leader of this party" );

    if ( obj_->remove_member( chr->serial ) )
    {
      bool disband;
      obj_->send_remove_member( chr, &disband );
      chr->party( nullptr );
      if ( Core::settingsManager.party_cfg.Hooks.OnLeaveParty )
        Core::settingsManager.party_cfg.Hooks.OnLeaveParty->call( chr->make_ref(), new BLong( 0 ) );
      if ( chr->has_active_client() )
      {
        Core::send_sysmessage_cl( chr->client,
                                  Core::CLP_Removed );  // You have been removed from the party.
        Core::send_empty_party( chr );
      }
      if ( disband )
        Core::disband_party( obj_->leader() );

      return new BLong( 1 );
    }
    return new BLong( 0 );
  }

  case MTH_SETLEADER:
  {
    Mobile::Character* chr;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( !ex.getCharacterParam( 0, chr ) )
      return new BError( "Invalid parameter type" );
    if ( !obj_->is_member( chr->serial ) )
      return new BError( "Character is not in this party" );
    if ( obj_->is_leader( chr->serial ) )
      return new BError( "Character is already leader of this party" );

    obj_->set_leader( chr->serial );
    obj_->send_member_list( nullptr );
    return new BLong( 1 );
  }

  case MTH_ADDCANDIDATE:
  {
    Mobile::Character* chr;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( !ex.getCharacterParam( 0, chr ) )
      return new BError( "Invalid parameter type" );
    if ( chr->has_party() )
      return new BError( "Character is already in a party" );
    else if ( chr->has_candidate_of() )
      return new BError( "Character is already candidate of a party" );
    else if ( chr->has_offline_mem_of() )
      return new BError( "Character is already offline member of a party" );
    if ( !obj_->can_add() )
      return new BError( "Party is already full" );
    if ( !chr->has_active_client() )
      return new BError( "Character is offline" );

    if ( Core::settingsManager.party_cfg.General.DeclineTimeout > 0 )
      chr->set_party_invite_timeout();

    if ( obj_->add_candidate( chr->serial ) )
    {
      Mobile::Character* leader = Core::system_find_mobile( obj_->leader() );
      if ( leader != nullptr )
      {
        chr->candidate_of( obj_.get() );
        Core::send_invite( chr, leader );
        return new BLong( 1 );
      }
    }
    return new BLong( 0 );
  }

  case MTH_REMOVECANDIDATE:
  {
    Mobile::Character* chr;
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( !ex.getCharacterParam( 0, chr ) )
      return new BError( "Invalid parameter type" );
    if ( !obj_->is_candidate( chr->serial ) )
      return new BError( "Character is not candidate of this party" );

    if ( obj_->remove_candidate( chr->serial ) )
    {
      chr->cancel_party_invite_timeout();
      Mobile::Character* leader = Core::system_find_mobile( obj_->leader() );
      chr->candidate_of( nullptr );
      if ( chr->has_active_client() )
        Core::send_sysmessage_cl(
            chr->client,
            Core::CLP_Decline );  // You notify them that you do not wish to join the party.
      if ( leader != nullptr )
      {
        if ( leader->has_active_client() )
          Core::send_sysmessage_cl_affix( leader->client, Core::CLP_Notify_Decline, chr->name(),
                                          true );  //: Does not wish to join the party.
      }

      if ( !obj_->test_size() )
        Core::disband_party( obj_->leader() );
      return new BLong( 1 );
    }
    return new BLong( 0 );
  }

  default:
    bool changed = false;
    return CallPropertyListMethod_id( obj_->_proplist, id, ex, changed );
  }
}

}  // namespace Module
}  // namespace Pol
