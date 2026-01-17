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
struct IPRule
{
  unsigned int ipMatch;
  unsigned int ipMask;
};
}  // namespace Pol::Network


#endif
