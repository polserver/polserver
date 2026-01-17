/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "pol_global_config.h"

#include "partymod.h"
#include <stddef.h>

#include "../../bscript/berror.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
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
#include "../partyscrobj.h"
#include "../polobject.h"
#include "../syshook.h"
#include "../uoexec.h"
#include "../uoscrobj.h"
#include <module_defs/party.h>

namespace Pol
{
namespace Core
{
class UOExecutor;

}
namespace Module
{
using namespace Bscript;
using UOExecutor = Core::UOExecutor;

PartyExecutorModule::PartyExecutorModule( Executor& exec )
    : TmplExecutorModule<PartyExecutorModule, Core::PolModule>( exec )
{
}


BObjectImp* CreatePartyRefObjImp( Core::Party* party )
{
  return new EPartyRefObjImp( ref_ptr<Core::Party>( party ) );
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
  if ( ( getCharacterParam( 0, leader ) ) && ( getCharacterParam( 1, firstmem ) ) )
  {
    if ( leader->has_party() )
      return new BError( "Leader is already in a party" );
    if ( leader->has_candidate_of() )
      return new BError( "Leader is already candidate of a party" );
    if ( leader->has_offline_mem_of() )
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
    Core::gamestate.parties.emplace_back( party );
    leader->party( party );

    if ( party->add_member( firstmem->serial ) )
    {
      firstmem->party( party );
      if ( Core::settingsManager.party_cfg.Hooks.OnPartyCreate )
        Core::settingsManager.party_cfg.Hooks.OnPartyCreate->call( CreatePartyRefObjImp( party ) );
      party->send_msg_to_all( Core::CLP_Added );  // You have been added to the party.
      if ( leader->has_active_client() )
        Core::send_sysmessage_cl_affix( leader->client, Core::CLP_Joined, firstmem->name(),
                                        true );  //  : joined the party.
      if ( firstmem->has_active_client() )
        Core::send_sysmessage_cl_affix( firstmem->client, Core::CLP_Joined, leader->name(), true );
      party->send_member_list( nullptr );
      party->send_stats_on_add( firstmem );
    }

    return new BLong( 1 );
  }
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
  return err;
}

BObjectImp* PartyExecutorModule::mf_SendPartyMsg()
{
  Core::Party* party;
  BError* err;
  if ( getPartyParam( exec, 0, party, err ) )
  {
    const String* text;
    Mobile::Character* chr;
    if ( ( getCharacterParam( 1, chr ) ) && ( getUnicodeStringParam( 2, text ) ) )
    {
      if ( text->length() > SPEECH_MAX_LEN )
        return new BError( "Text exceeds maximum size." );
      if ( Core::settingsManager.party_cfg.Hooks.OnPublicChat )
        Core::settingsManager.party_cfg.Hooks.OnPublicChat->call( chr->make_ref(),
                                                                  new String( *text ) );

      party->send_member_msg_public( chr, text->value() );
      return new BLong( 1 );
    }
    return new BError( "Invalid parameter type" );
  }
  return err;
}

BObjectImp* PartyExecutorModule::mf_SendPrivatePartyMsg()
{
  Core::Party* party;
  BError* err;
  if ( getPartyParam( exec, 0, party, err ) )
  {
    const String* text;
    Mobile::Character* chr;
    Mobile::Character* tochr;
    if ( ( getCharacterParam( 1, chr ) ) && ( getCharacterParam( 2, tochr ) ) &&
         ( getUnicodeStringParam( 3, text ) ) )
    {
      if ( text->length() > SPEECH_MAX_LEN )
        return new BError( "Text exceeds maximum size." );
      if ( Core::settingsManager.party_cfg.Hooks.OnPrivateChat )
        Core::settingsManager.party_cfg.Hooks.OnPrivateChat->call(
            chr->make_ref(), tochr->make_ref(), new String( *text ) );

      party->send_member_msg_private( chr, tochr, text->value() );
      return new BLong( 1 );
    }
    return new BError( "Invalid parameter type" );
  }
  return err;
}
}  // namespace Module
}  // namespace Pol
