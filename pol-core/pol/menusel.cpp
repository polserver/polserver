/** @file
 *
 * @par History
 */


#include "../clib/clib_endian.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "accounts/account.h"
#include "menu.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "pktin.h"
#include "ufunc.h"

namespace Pol
{
namespace Core
{
void handle_menu_selection( Network::Client* client, PKTIN_7D* msg )
{
  passert_paranoid( client );

  if ( !client->chr )
  {
    POLLOG.Format( "{} tried to use a menu without being in the game.\n" ) << client->acct->name();
    return;
  }


  Menu* active_menu = nullptr;
  if ( client->chr->menu.exists() )
  {
    active_menu = client->chr->menu.get_weakptr();
  }

  if ( active_menu == nullptr )
  {
    POLLOG.Format( "{}/{} tried to use a menu, but none was active.\n" )
        << client->acct->name() << client->chr->name();
    return;
  }

  u16 menu_id = cfBEu16( msg->menu_id );
  if ( active_menu->menu_id != menu_id )
  {
    INFO_PRINT << "Client tried to use a menu he wasn't entitled to\n";
    // LOGME illegal menu selection
    client->chr->cancel_menu();
    return;
  }

  client->chr->menu.clear();

  if ( msg->choice == 0 )  // client cancelled menu
  {
    client->chr->cancel_menu();
    return;
  }

  u16 choice = cfBEu16( msg->choice );
  if ( choice == 0 || choice > active_menu->menuitems_.size() )
  {
    INFO_PRINT << "Client menu choice out of range\n";
    client->chr->cancel_menu();
    return;
  }
  // Wow, client chose something valid from the menu, even.
  // Note, the incoming message specified object type, color, and used_item_serial.
  // Since we'd just have to verify these anyway, I'm just going
  // to ignore them, and use the information in the MenuItem.
  // If this turns out not to be workable, we'll have to validate those
  // input fields, too.

  MenuItem* mi = &active_menu->menuitems_[choice - 1];
  if ( mi->submenu_id )
  {
    Menu* submenu = Menu::find_menu( mi->submenu_id );
    client->chr->menu = submenu->getWeakPtr();
    send_menu( client, submenu );
  }
  else
  {
    passert( client->chr->on_menu_selection );
    client->chr->on_menu_selection( client, mi, msg );
  }
}
}
}
