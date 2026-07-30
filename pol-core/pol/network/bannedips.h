/** @file
 *
 * @par History
 * - 2009/01/24 MuadDib   Creation of bannedips.h. This is for the new upcoming bannedips.cpp/h
 * setup.
 *                        This will work by storing the ip/mask in a vectored struct so
 *                        it's not reloading EVERY client connection! Will need to set a
 *                        clearing/reset with reloadcfg for this new system when done.
 */

#ifndef BANNEDIPS_H
#define BANNEDIPS_H


namespace Pol::Network
{
class Client;

bool is_banned_ip( Client* client );
void read_bannedips_config( bool initial_load );
}  // namespace Pol::Network


#endif
