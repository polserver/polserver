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
    POLLOG_INFOLN(
        "Warning: Character {:#x} sent an unexpected gump menu selection. Gump ID {:#x}, "
        "button ID {:#x}",
        client->chr->serial, gumpid, buttonid );
  }
}

void SuspiciousActs::GumpResponseHasTooManyInts( Network::Client* client )
{
  // TODO: report the extra ints?
  if ( Plib::systemstate.config.show_warning_gump )
  {
    ERROR_PRINTLN(
        "Client (Account {}, Character {}) Blech! B1 message specified more ints than it can hold!",
        client->acct->name(), client->chr->name() );
  }
}

void SuspiciousActs::GumpResponseHasTooManyIntsOrStrings( Network::Client* client )
{
  // TODO: report the extra ints/strings?
  if ( Plib::systemstate.config.show_warning_gump )
  {
    ERROR_PRINTLN(
        "Client (Account {}, Character {}) Blech! B1 message specified too many ints and/or "
        "strings!",
        client->acct->name(), client->chr->name() );
  }
}

void SuspiciousActs::GumpResponseOverflows( Network::Client* client )
{
  // TODO: report by how much?
  if ( Plib::systemstate.config.show_warning_gump )
  {
    ERROR_PRINTLN(
        "Client (Account {}, Character {}) Blech! B1 message strings overflow the message buffer!",
        client->acct->name(), client->chr->name() );
  }
}

void SuspiciousActs::DropItemButNoneGotten( Network::Client* client, u32 dropped_item_serial )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERRORLN( "Character {:#x} tried to drop item {:#x}, but had not gotten an item.",
                    client->chr->serial, dropped_item_serial );
  }
}

void SuspiciousActs::DropItemOtherThanGotten( Network::Client* client, u32 dropped_item_serial,
                                              u32 gotten_item_serial )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERRORLN( "Character {:#x} tried to drop item {:#x}, but instead had gotten item {:#x}.",
                    client->chr->serial, dropped_item_serial, gotten_item_serial );
  }
}

void SuspiciousActs::EquipItemButNoneGotten( Network::Client* client, u32 equipped_item_serial )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERRORLN(
        "Character {:#x} tried to equip item {:#x}, which did not exist in gotten_items.",
        client->chr->serial, equipped_item_serial );
  }
}

void SuspiciousActs::EquipItemOtherThanGotten( Network::Client* client, u32 equipped_item_serial,
                                               u32 gotten_item_serial )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERRORLN( "Character {:#x} tried to equip item {:#x}, but had gotten item {:#x}",
                    client->chr->serial, equipped_item_serial, gotten_item_serial );
  }
}

void SuspiciousActs::OutOfSequenceCursor( Network::Client* client )
{
  Mobile::Character* targetter = client->chr;

  if ( Plib::systemstate.config.show_warning_cursor_seq )
  {
    POLLOG_ERRORLN( "{}/{} used out of sequence cursor.", targetter->acct->name(),
                    targetter->name() );
  }
}

void SuspiciousActs::DropItemOutOfRange( Network::Client* client, u32 )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERRORLN( "Client (Character {}) tried to drop an item out of range.",
                    client->chr->name() );
  }
}

void SuspiciousActs::DropItemOutAtBlockedLocation( Network::Client* client, u32,
                                                   const Core::Pos3d& pos )
{
  if ( Plib::systemstate.config.show_warning_item )
  {
    POLLOG_ERRORLN(
        "Client (Character {}) tried to drop an item at {}, which is a blocked location",
        client->chr->name(), pos );
  }
}

void SuspiciousActs::BoatMoveNoMulti( Network::Client* client )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERRORLN( "{}/{} tried to use a boat movement packet without being on a multi.",
                    client->acct->name(), client->chr->name() );
  }
}

void SuspiciousActs::BoatMoveNotBoatMulti( Network::Client* client )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERRORLN( "{}/{} tried to use a boat movement packet without being on a boat multi.",
                    client->acct->name(), client->chr->name() );
  }
}

void SuspiciousActs::BoatMoveNotPilot( Network::Client* client, u32 multi_serial )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERRORLN(
        "{}/{} tried to use a boat movement packet on a boat multi (serial {:#x}) that they are "
        "not the pilot of.",
        client->acct->name(), client->chr->name(), multi_serial );
  }
}

void SuspiciousActs::BoatMoveOutOfRangeParameters( Network::Client* client, u32 multi_serial,
                                                   u8 direction, u8 speed )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERRORLN(
        "{}/{} tried to use a boat movement packet on a boat multi (serial {:#x}) with "
        "out-of-range parameters (direction = {}, speed = {}).",
        client->acct->name(), client->chr->name(), multi_serial, direction, speed );
  }
}

void SuspiciousActs::CharacterMovementWhilePiloting( Network::Client* client )
{
  if ( Plib::systemstate.config.show_warning_boat_move )
  {
    POLLOG_ERRORLN( "{}/{} tried to move their character while piloting a boat.",
                    client->acct->name(), client->chr->name() );
  }
}
