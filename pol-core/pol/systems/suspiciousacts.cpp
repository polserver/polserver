#include "suspiciousacts.h"

#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "plib/systemstate.h"

#include "accounts/account.h"
#include "mobile/charactr.h"
#include "network/client.h"

using namespace Pol;

void SuspiciousActs::GumpResponseWasUnexpected( Network::Client* client, u32 gumpid, u32 buttonid )
{
  if ( Plib::systemstate.config.show_warning_gump )
  {
    POLLOG_INFO.Format(
        "\nWarning: Character 0x{:X} sent an unexpected gump menu selection. Gump ID 0x{:X}, "
        "button ID 0x{:X}\n" )
        << client->chr->serial << gumpid << buttonid;
  }
}

void SuspiciousActs::GumpResponseHasTooManyInts( Network::Client* client )
{
  // TODO: report the extra ints?
  if ( Plib::systemstate.config.show_warning_gump )
  {
    ERROR_PRINT << "Client (Account " << client->acct->name() << ", Character "
                << client->chr->name()
                << ") Blech! B1 message specified more ints than it can hold!\n";
  }
}

void SuspiciousActs::GumpResponseHasTooManyIntsOrStrings( Network::Client* client )
{
  // TODO: report the extra ints/strings?
  if ( Plib::systemstate.config.show_warning_gump )
  {
    ERROR_PRINT << "Client (Account " << client->acct->name() << ", Character "
                << client->chr->name()
                << ") Blech! B1 message specified too many ints and/or strings!\n";
  }
}

void SuspiciousActs::GumpResponseOverflows( Network::Client* client )
{
  // TODO: report by how much?
  if ( Plib::systemstate.config.show_warning_gump )
  {
    ERROR_PRINT << "Client (Account " << client->acct->name() << ", Character "
                << client->chr->name()
                << ") Blech! B1 message strings overflow the message buffer!\n";
  }
}

void SuspiciousActs::DropItemButNoneGotten( Network::Client* client, u32 dropped_item_serial )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERROR.Format(
        "Character 0x{:X} tried to drop item 0x{:X}, but had not gotten an item.\n" )
        << client->chr->serial << dropped_item_serial;
  }
}

void SuspiciousActs::DropItemOtherThanGotten( Network::Client* client, u32 dropped_item_serial,
                                              u32 gotten_item_serial )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERROR.Format(
        "Character 0x{:X} tried to drop item 0x{:X}, but instead had gotten item 0x{:X}.\n" )
        << client->chr->serial << dropped_item_serial << gotten_item_serial;
  }
}

void SuspiciousActs::EquipItemButNoneGotten( Network::Client* client, u32 equipped_item_serial )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERROR.Format(
        "Character 0x{:X} tried to equip item 0x{:X}, which did not exist in gotten_items.\n" )
        << client->chr->serial << equipped_item_serial;
  }
}

void SuspiciousActs::EquipItemOtherThanGotten( Network::Client* client, u32 equipped_item_serial,
                                               u32 gotten_item_serial )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERROR.Format(
        "Character 0x{:X} tried to equip item 0x{:X}, but had gotten item 0x{:X}\n" )
        << client->chr->serial << equipped_item_serial << gotten_item_serial;
  }
}

void SuspiciousActs::OutOfSequenceCursor( Network::Client* client )
{
  Mobile::Character* targetter = client->chr;

  if ( Plib::systemstate.config.show_warning_cursor_seq )
  {
    POLLOG_ERROR << targetter->acct->name() << "/" << targetter->name()
                 << " used out of sequence cursor.\n";
  }
}

void SuspiciousActs::DropItemOutOfRange( Network::Client* client, u32 )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERROR.Format( "Client (Character {}) tried to drop an item out of range.\n" )
        << client->chr->name();
  }
}

void SuspiciousActs::DropItemOutAtBlockedLocation( Network::Client* client, u32,
                                                   const Core::Pos3d& pos )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERROR.Format(
        "Client (Character {}) tried to drop an item at ({},{},{}), which is a blocked "
        "location.\n" )
        << client->chr->name() << pos.x() << pos.y() << (int)pos.z();
  }
}

void SuspiciousActs::BoatMoveNoMulti( Network::Client* client )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERROR.Format( "{}/{} tried to use a boat movement packet without being on a multi.\n" )
        << client->acct->name() << client->chr->name();
  }
}

void SuspiciousActs::BoatMoveNotBoatMulti( Network::Client* client )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERROR.Format(
        "{}/{} tried to use a boat movement packet without being on a boat multi.\n" )
        << client->acct->name() << client->chr->name();
  }
}

void SuspiciousActs::BoatMoveMultiNoRunningScript( Network::Client* client, u32 multi_serial )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERROR.Format(
        "{}/{} tried to use a boat movement packet on a boat multi (serial 0x{:X}) that has no "
        "running script.\n" )
        << client->acct->name() << client->chr->name() << multi_serial;
  }
}
void SuspiciousActs::BoatMoveOutOfRangeParameters( Network::Client* client, u32 multi_serial,
                                                   u8 direction, u8 speed )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERROR.Format(
        "{}/{} tried to use a boat movement packet on a boat multi (serial 0x{:X}) with "
        "out-of-range parameters (direction = {}, speed = {})\n." )
        << client->acct->name() << client->chr->name() << multi_serial << direction << speed;
  }
}
