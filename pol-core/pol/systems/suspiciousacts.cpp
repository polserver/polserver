#include "suspiciousacts.h"

#include "clib/logfacility.h"
#include "clib/rawtypes.h"
#include "plib/systemstate.h"

#include "accounts/account.h"
#include "mobile/charactr.h"
#include "network/client.h"

using namespace Pol;

void SuspiciousActs::UnexpectedGumpResponse( Network::Client* client, u32 gumpid, u32 buttonid )
{
  if ( Plib::systemstate.config.show_warning_gump )
  {
    POLLOG_INFO.Format(
        "\nWarning: Character 0x{:X} sent an unexpected gump menu selection. Gump ID 0x{:X}, "
        "button ID 0x{:X}\n" )
        << client->chr->serial << gumpid << buttonid;
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