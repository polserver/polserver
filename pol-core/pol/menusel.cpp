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
#include "network/cgdata.h"
#include "network/client.h"
#include "network/pktin.h"
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
    POLLOGLN( "{} tried to use a menu without being in the game.", client->acct->name() );
    return;
  }


  Menu* active_menu = nullptr;
  if ( client->gd->menu.exists() )
  {
    active_menu = client->gd->menu.get_weakptr();
  }

  if ( active_menu == nullptr )
  {
    POLLOGLN( "{}/{} tried to use a menu, but none was active.", client->acct->name(),
              client->chr->name() );
    return;
  }

  u16 menu_id = cfBEu16( msg->menu_id );
  if ( active_menu->menu_id != menu_id )
  {
    INFO_PRINTLN( "Client tried to use a menu he wasn't entitled to" );
    // LOGME illegal menu selection
    client->chr->cancel_menu();
    return;
  }

  client->gd->menu.clear();

  if ( msg->choice == 0 )  // client cancelled menu
  {
    client->chr->cancel_menu();
    return;
  }

  u16 choice = cfBEu16( msg->choice );
  if ( choice == 0 || choice > active_menu->menuitems_.size() )
  {
    INFO_PRINTLN( "Client menu choice out of range" );
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
    client->gd->menu = submenu->getWeakPtr();
    send_menu( client, submenu );
  }
  else
  {
    passert( client->gd->on_menu_selection );
    client->gd->on_menu_selection( client, mi, msg );
  }
}
}  // namespace Core
}  // namespace Pol
