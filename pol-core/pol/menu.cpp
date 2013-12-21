/*
History
=======


Notes
=======

*/

#include "../clib/stl_inc.h"
#include <string.h>
#include <assert.h>

#include "../clib/clib.h"
#include "../clib/passert.h"

#include "menu.h"
namespace Pol {
  namespace Core {

	MenuItem::MenuItem() :
	  objtype_( 0 ),
	  graphic_( 0 ),
	  color_( 0 ),
	  submenu_id( 0 )
	{
	  title[0] = '\0';
	  submenu_name[0] = '\0';
	}

	Menu::Menu() :
	  menu_id( 0 )
	{
	  name[0] = '\0';
	  title[0] = '\0';
	}


	std::vector<Menu> menus;



	/*
	u16 GetMenuId( void )
	{
	static u16 next_menuid =
	}
	*/

	Menu *find_menu( const char *name )
	{
	  for ( unsigned idx = 0; idx < menus.size(); idx++ )
	  {
		Menu *menu = &menus[idx];
		if ( stricmp( menu->name, name ) == 0 )
		  return menu;
	  }
	  return NULL;
	}

	// currently a menu ID is its location in the array.
	// we assume the code somehow validates menu choices.  For example, 
	// this is currently done by remembering which menu we sent a client.
	// and only allowing the client to use that particular menu.
	Menu *find_menu( unsigned short menu_id )
	{
	  passert( menu_id > 0 && menu_id <= menus.size() );

	  return &menus[menu_id - 1];
	}
  }
}