/** @file
 *
 * @par History
 * - 2005/11/26 MuadDib:   Added update code for updating highlight packet to client.
 * - 2005/11/26 Shinigami: added Character check
 * - 2005/11/25 MuadDib:   Added distance check in handle_attack.
 * - 2006/03/07 MuadDib:   Added Justice Region NoCombat check to attack request.
 * - 2009/01/03 MuadDib:   Removed logging of null defenders. Who cares if it's empty :o
 */


#include <cstdio>

#include "../../clib/clib_endian.h"
#include "../../clib/rawtypes.h"
#include "../cmbtcfg.h"
#include "../globals/settings.h"
#include "../item/item.h"
#include "../network/client.h"
#include "../network/pktin.h"
#include "../ufunc.h"
#include "charactr.h"
#include "regions/guardrgn.h"

namespace Pol
{
namespace Mobile
{
/*Attackable::Attackable( Character* chr ) : _opp( chr ) {}
Attackable::Attackable( Items::Item* item ) : _opp( item )
{
  if ( !item->is_attackable() )
    _opp = nullptr;
}*/
Attackable::Attackable( Core::UObject* obj ) : _opp( obj )
{
  if ( obj->ismobile() )
    return;
  if ( auto* item_ = item(); item_ && item_->is_attackable() )
    return;
  _opp = nullptr;
}

Character* Attackable::mobile() const
{
  if ( _opp && _opp->ismobile() )
    return static_cast<Character*>( _opp );
  return nullptr;
}
Items::Item* Attackable::item() const
{
  if ( _opp && _opp->isitem() )
    return static_cast<Items::Item*>( _opp );
  return nullptr;
}
void Attackable::remove_opponent_of( const Attackable& other )
{
  // Items have no need to store multiple opponents
  auto* mob = mobile();
  if ( !mob )
    return;
  mob->remove_opponent_of( other );
}
void Attackable::add_opponent_of( Attackable other )
{
  // Items have no need to store multiple opponents
  auto* mob = mobile();
  if ( !mob )
    return;
  mob->add_opponent_of( std::move( other ) );
}
void Attackable::inform_disengaged( const Attackable& disengaged )
{
  if ( auto* mob = mobile() )
    mob->inform_disengaged( disengaged );
  else if ( auto* item_ = item() )
    ;  // TODO send event
}
void Attackable::inform_engaged( const Attackable& engaged )
{
  if ( auto* mob = mobile() )
    mob->inform_engaged( engaged );
  else if ( auto* item_ = item() )
    ;  // TODO send event
}

void handle_attack( Network::Client* client, Core::PKTIN_05* msg )
{
  if ( client->chr->dead() )
  {
    private_say_above( client->chr, client->chr, "I am dead and cannot do that." );
    return;
  }

  u32 serial = cfBEu32( msg->serial );
  Character* defender = Core::find_character( serial );
  if ( defender == nullptr )
    return;
  if ( !( Core::settingsManager.combat_config.attack_self ) )
  {
    if ( defender->serial == client->chr->serial )
    {
      client->chr->send_highlight();
      return;
    }
  }

  if ( !client->chr->is_visible_to_me( defender ) )
  {
    client->chr->send_highlight();
    return;
  }

  if ( defender->acct != nullptr )
  {
    if ( Core::JusticeRegion::RunNoCombatCheck( defender->client ) == true )
    {
      client->chr->send_highlight();
      Core::send_sysmessage( client, "Combat is not allowed in this area." );
      return;
    }
  }
  client->chr->select_opponent( serial );
}
}  // namespace Mobile
}  // namespace Pol
