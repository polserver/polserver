/** @file
 *
 * @par History
 * - 2011/11/10 Tomi:    Added file
 * - 2011/12/13 Tomi:    Added rope, wheel, hull, sails, tiller, rudder, storage and weaponslot
 * objtypes for new boat support
 */


#ifndef EXTOBJ_H
#define EXTOBJ_H

namespace Pol::Core
{
struct ExternalObject
{
  unsigned int tillerman;
  unsigned int port_plank;
  unsigned int starboard_plank;
  unsigned int hold;
  unsigned int rope;
  unsigned int wheel;
  unsigned int hull;
  unsigned int tiller;
  unsigned int rudder;
  unsigned int sails;
  unsigned int storage;
  unsigned int weaponslot;
  unsigned int boatmount;

  unsigned int wrestling;
  unsigned int mount;
  unsigned int shield;

  unsigned int secure_trade_container;
  unsigned int wornitems_container;
};

void read_extobj();
}  // namespace Pol::Core

#endif
