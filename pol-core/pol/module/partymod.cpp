/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "pol_global_config.h"

#include "partymod.h"
#include "modules/party-tbl.h"

#include <stddef.h>

#include "../../bscript/berror.h"
#include "../../bscript/executor.h"
#include "../../bscript/objmembers.h"
#include "../../bscript/objmethods.h"
#include "../../clib/rawtypes.h"
#include "../../clib/stlutil.h"
#include "../clfunc.h"
#include "../fnsearch.h"
#include "../globals/settings.h"
#include "../globals/uvars.h"
#include "../mobile/charactr.h"
#include "../network/pktdef.h"
#include "../party.h"
#include "../party_cfg.h"
#include "../syshook.h"
#include "../unicode.h"
#include "../uoexhelp.h"
#include "../uoscrobj.h"

namespace Pol
{
namespace Module
{
using namespace Bscript;

PartyExecutorModule::PartyExecutorModule( Executor& exec )
    : TmplExecutorModule<PartyExecutorModule>( exec )
{
}

class EPartyRefObjImp final : public BApplicObj<Core::PartyRef>
{
public:
  EPartyRefObjImp( Core::PartyRef pref );
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual BObjectImp* copy() const override;
  virtual bool isTrue() const override;
  virtual bool operator==( const BObjectImp& objimp ) const override;

  virtual BObjectRef get_member( const char* membername ) override;
  virtual BObjectRef get_member_id( const int id ) override;  // id test
  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) override;
  virtual BObjectImp* call_method_id( const int id, Executor& ex,
                                      bool forcebuiltin = false ) override;
};
BApplicObjType party_type;
EPartyRefObjImp::EPartyRefObjImp( Core::PartyRef pref )
    : BApplicObj<Core::PartyRef>( &party_type, pref ){};

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

BObjectImp* CreatePartyRefObjImp( Core::Party* party )
{
  return new EPartyRefObjImp( ref_ptr<Core::Party>( party ) );
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

BObjectImp* EPartyRefObjImp::call_method( const char* methodname, Executor& ex )
{
  bool forcebuiltin{Executor::builtinMethodForced( methodname )};
  Bscript::ObjMethod* objmethod = Bscript::getKnownObjMethod( methodname );
  if ( objmethod != nullptr )
    return call_method_id( objmethod->id, ex, forcebuiltin );
  auto* res = Core::gamestate.system_hooks.call_script_method( methodname, &ex, this );
  if ( res )
    return res;
  bool changed = false;
  return CallPropertyListMethod( obj_->_proplist, methodname, ex, changed );
}

BObjectImp* EPartyRefObjImp::call_method_id( const int id, Executor& ex, bool forcebuiltin )
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
    if ( !ex.hasParams( 1 ) )
      return new BError( "Not enough parameters" );
    if ( !getCharacterParam( ex, 0, chr ) )
      return new BError( "Invalid parameter type" );
    if ( chr->has_party() )
      return new BError( "Character is already in a party" );
    else if ( chr->has_candidate_of() )
      return new BError( "Character is already candidate of a party" );
    else if ( chr->has_offline_mem_of() )
      return new BError( "Character is already offline member of a party" );
    if ( !obj_->can_add() )
      return new BError( "Party is already full" );
    if ( obj_->add_member( chr->serial ) )
    {
      chr->party( obj_.get() );
      if ( Core::settingsManager.party_cfg.Hooks.OnAddToParty )
        Core::settingsManager.party_cfg.Hooks.OnAddToParty->call( chr->make_ref() );
      if ( chr->has_active_client() )
        Core::send_sysmessage_cl( chr->client,
                                  Core::CLP_Added );  // You have been added to the party.
      obj_->send_msg_to_all( Core::CLP_Joined, chr->name().c_str(), chr );  //  : joined the party.
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
    if ( !getCharacterParam( ex, 0, chr ) )
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
    if ( !getCharacterParam( ex, 0, chr ) )
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
    if ( !getCharacterParam( ex, 0, chr ) )
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
    if ( !getCharacterParam( ex, 0, chr ) )
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
          Core::send_sysmessage_cl_affix( leader->client, Core::CLP_Notify_Decline,
                                          chr->name().c_str(),
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

// party.em Functions:
bool getPartyParam( Executor& exec, unsigned param, Core::Party*& party, BError*& err )
{
  BApplicObjBase* aob = nullptr;
  if ( exec.hasParams( param + 1 ) )
    aob = exec.getApplicObjParam( param, &party_type );

  if ( aob == nullptr )
  {
    err = new BError( "Invalid parameter type" );
    return false;
  }

  EPartyRefObjImp* pr = static_cast<EPartyRefObjImp*>( aob );
  party = pr->value().get();
  if ( Core::system_find_mobile( party->leader() ) == nullptr )
  {
    err = new BError( "Party has no leader" );
    return false;
  }
  return true;
}

BObjectImp* PartyExecutorModule::mf_CreateParty()
{
  Mobile::Character* leader;
  Mobile::Character* firstmem;
  if ( ( getCharacterParam( exec, 0, leader ) ) && ( getCharacterParam( exec, 1, firstmem ) ) )
  {
    if ( leader->has_party() )
      return new BError( "Leader is already in a party" );
    else if ( leader->has_candidate_of() )
      return new BError( "Leader is already candidate of a party" );
    else if ( leader->has_offline_mem_of() )
      return new BError( "Leader is already offline member of a party" );
    else if ( leader == firstmem )
      return new BError( "Leader and Firstmember are the same" );
    else if ( firstmem->has_party() )
      return new BError( "First Member is already in a party" );
    else if ( firstmem->has_candidate_of() )
      return new BError( "First Member is already candidate of a party" );
    else if ( firstmem->has_offline_mem_of() )
      return new BError( "First Member is already offline member of a party" );

    Core::Party* party = new Core::Party( leader->serial );
    Core::gamestate.parties.push_back( ref_ptr<Core::Party>( party ) );
    leader->party( party );

    if ( party->add_member( firstmem->serial ) )
    {
      firstmem->party( party );
      if ( Core::settingsManager.party_cfg.Hooks.OnPartyCreate )
        Core::settingsManager.party_cfg.Hooks.OnPartyCreate->call( CreatePartyRefObjImp( party ) );
      party->send_msg_to_all( Core::CLP_Added );  // You have been added to the party.
      if ( leader->has_active_client() )
        Core::send_sysmessage_cl_affix( leader->client, Core::CLP_Joined, firstmem->name().c_str(),
                                        true );  //  : joined the party.
      if ( firstmem->has_active_client() )
        Core::send_sysmessage_cl_affix( firstmem->client, Core::CLP_Joined, leader->name().c_str(),
                                        true );
      party->send_member_list( nullptr );
      party->send_stats_on_add( firstmem );
    }

    return new BLong( 1 );
  }
  else
    return new BError( "Invalid parameter type" );
}

BObjectImp* PartyExecutorModule::mf_DisbandParty()
{
  Core::Party* party;
  BError* err;
  if ( getPartyParam( exec, 0, party, err ) )
  {
    Core::disband_party( party->leader() );
    return new BLong( 1 );
  }
  else
    return err;
}

BObjectImp* PartyExecutorModule::mf_SendPartyMsg()
{
  Core::Party* party;
  BError* err;
  if ( getPartyParam( exec, 0, party, err ) )
  {
    ObjArray* oText;
    Mobile::Character* chr;
    if ( ( getCharacterParam( exec, 1, chr ) ) && ( getObjArrayParam( 2, oText ) ) )
    {
      size_t textlen = oText->ref_arr.size();
      u16 gwtext[( SPEECH_MAX_LEN + 1 )];
      if ( textlen > SPEECH_MAX_LEN )
        return new BError( "Unicode array exceeds maximum size." );
      if ( !Core::convertArrayToUC( oText, gwtext, textlen, true ) )
        return new BError( "Invalid value in Unicode array." );

      if ( Core::settingsManager.party_cfg.Hooks.OnPublicChat )
        Core::settingsManager.party_cfg.Hooks.OnPublicChat->call( chr->make_ref(), oText );

      party->send_member_msg_public( chr, gwtext, textlen );
      return new BLong( 1 );
    }
    else
      return new BError( "Invalid parameter type" );
  }
  else
    return err;
}

BObjectImp* PartyExecutorModule::mf_SendPrivatePartyMsg()
{
  Core::Party* party;
  BError* err;
  if ( getPartyParam( exec, 0, party, err ) )
  {
    ObjArray* oText;
    Mobile::Character* chr;
    Mobile::Character* tochr;
    if ( ( getCharacterParam( exec, 1, chr ) ) && ( getCharacterParam( exec, 2, tochr ) ) &&
         ( getObjArrayParam( 3, oText ) ) )
    {
      size_t textlen = oText->ref_arr.size();
      u16 gwtext[( SPEECH_MAX_LEN + 1 )];
      if ( textlen > SPEECH_MAX_LEN )
        return new BError( "Unicode array exceeds maximum size." );
      if ( !Core::convertArrayToUC( oText, gwtext, textlen, true ) )
        return new BError( "Invalid value in Unicode array." );

      if ( Core::settingsManager.party_cfg.Hooks.OnPrivateChat )
        Core::settingsManager.party_cfg.Hooks.OnPrivateChat->call( chr->make_ref(),
                                                                   tochr->make_ref(), oText );

      party->send_member_msg_private( chr, tochr, gwtext, textlen );
      return new BLong( 1 );
    }
    else
      return new BError( "Invalid parameter type" );
  }
  else
    return err;
}
}  // namespace Module
}  // namespace Pol
