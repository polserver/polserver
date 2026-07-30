/** @file
 *
 * @par History
 */

#pragma once

namespace Pol
{
namespace Network
{
class Client;
}
namespace Mobile
{
class Character;
}
namespace Items
{
class Item;
}
namespace Core
{
void send_full_statmsg( Network::Client* client, Mobile::Character* chr );
void send_short_statmsg( Network::Client* client, Mobile::Character* chr );
void send_update_hits_to_inrange( Mobile::Character* chr );
void send_stat_locks( Network::Client* client, Mobile::Character* chr );
void send_short_statmsg( Network::Client* client, Items::Item* item );
}  // namespace Core
}  // namespace Pol
