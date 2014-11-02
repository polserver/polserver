/*
History
=======


Notes
=======

*/

#include "menu.h"

#include "../clib/clib.h"
#include "../clib/passert.h"

#include <cstring>

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
	size_t Menu::estimateSize() const
	{
	  size_t size = sizeof( Menu );
	  size += 3*sizeof(MenuItem*)+menuitems_.capacity()*sizeof( MenuItem );
	  return size;
	}


	std::vector<Menu> menus;

	size_t estimateMenuSize()
	{
	  size_t size = 3 * sizeof( Menu* );
	  for ( const auto& menu : menus)
	  {
		size += menu.estimateSize();
	  }
	  return size;
	}



	/*
	u16 GetMenuId( void )
	{
	static u16 next_menuid =
	}
	*/

    // TODO:: rewrite using std::find() and stlutil.h case insensitive string cmp
    //         -- leaving the warning here as a reminder --
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