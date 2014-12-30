/*
History
=======


Notes
=======

*/

#include "menu.h"

#include "../clib/logfacility.h"

#include "../clib/clib.h"
#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/stlutil.h"
#include "../clib/fileutil.h"
#include "../clib/passert.h"

#include "../plib/systemstate.h"

#include "item/itemdesc.h"
#include "globals/uvars.h"

#include <cstring>

#ifdef _MSC_VER
#pragma warning(disable:4996) // deprecation warning stricmp
#endif

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

	size_t Menu::estimateMenuSize()
	{
	  size_t size = 3 * sizeof( Menu* );
	  for ( const auto& menu : gamestate.menus)
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
	Menu *Menu::find_menu( const char *name )
	{
	  for ( unsigned idx = 0; idx < gamestate.menus.size(); idx++ )
	  {
		Menu *menu = &gamestate.menus[idx];
		if ( stricmp( menu->name, name ) == 0 )
		  return menu;
	  }
	  return NULL;
	}

	// currently a menu ID is its location in the array.
	// we assume the code somehow validates menu choices.  For example, 
	// this is currently done by remembering which menu we sent a client.
	// and only allowing the client to use that particular menu.
	Menu *Menu::find_menu( unsigned short menu_id )
	{
	  passert( menu_id > 0 && menu_id <= gamestate.menus.size() );

	  return &gamestate.menus[menu_id - 1];
	}

	void Menu::read_menus()
	{
	  if ( !Clib::FileExists( "config/menus.cfg" ) )
	  {
		if ( Plib::systemstate.config.loglevel > 1 )
          INFO_PRINT << "File config/menus.cfg not found, skipping.\n";
		return;
	  }

      Clib::ConfigFile cf( "config/menus.cfg" );

      Clib::ConfigElem elem;

	  while ( cf.read( elem ) )
	  {
		std::string menu_name;
        std::string menu_title;
		std::string propname;
		std::string value;

		if ( stricmp( elem.type(), "ItemMenu" ) != 0 )
		  continue;

		menu_name = elem.remove_string( "NAME" );
		menu_title = elem.remove_string( "TITLE" );

		gamestate.menus.push_back( Menu() );
		Menu* menu = &gamestate.menus.back();

		menu->menu_id = static_cast<unsigned short>( gamestate.menus.size() );
		strzcpy( menu->name, menu_name.c_str(), sizeof menu->name );
		strzcpy( menu->title, menu_title.c_str(), sizeof menu->title );

		while ( elem.remove_first_prop( &propname, &value ) )
		{
		  if ( stricmp( propname.c_str(), "SubMenu" ) == 0 ||
			   stricmp( propname.c_str(), "Entry" ) == 0 )
		  {
			std::string submenu_name;
            std::string objtype_str;
            std::string title;

			/*
			char *submenu_name = NULL;
			char *objtype_str = NULL;
			char *title = NULL;
			*/

			if ( stricmp( propname.c_str(), "SubMenu" ) == 0 )
			{
			  ISTRINGSTREAM is( value );
			  is >> submenu_name >> objtype_str;

			  // skipws
			  // is.eatwhite();

			  std::getline( is, title ); // title.getline( is );

			  // FIXME: remove leading spaces (better way? please? anyone?)
			  while ( isspace( title[0] ) )
				title.erase( 0, 1 );

			  //char s[100];
			  //is.ignore( std::numeric_limits<int>::max(), ' ' );
			  //is.ignore( std::numeric_limits<int>::max(), '\t' );
			  //is.getline( s, sizeof s );
			  //title = s;
			  /*
			  submenu_name = strtok( value, " \t," );
			  objtype_str = strtok( NULL, " \t," );
			  title = strtok( NULL, "" ); // title is the rest of the line
			  while ((title != NULL) && *title && isspace(*title)) // skip leading spaces
			  title++;
			  */
			}
			else // Entry
			{
			  submenu_name = "";
			  ISTRINGSTREAM is( value );
			  is >> objtype_str; // FIXME objtype_str ends up having the "," on the end

			  std::getline( is, title );
			  // FIXME: remove leading spaces (better way? please? anyone?)
			  while ( isspace( title[0] ) )
				title.erase( 0, 1 );
			}

			if ( objtype_str == "" )
			{
              ERROR_PRINT << "Entry in menu " << menu->name
                << " must provide at least an object type\n";
			  throw std::runtime_error( "Data error in MENUS.CFG" );
			}
			u32 objtype = (u32)strtoul( objtype_str.c_str(), NULL, 0 );
			if ( objtype == 0 ) // 0 specified, or text
			{
              ERROR_PRINT << "Entry in menu " << menu->name
                << " cannot specify [" << objtype_str << "] as an Object Type.\n";
			  throw std::runtime_error( "Data error in MENUS.CFG" );
			}
			if ( ( stricmp( propname.c_str(), "SubMenu" ) == 0 ) &&
				 ( submenu_name == "" ) )
			{
              ERROR_PRINT << "SubMenu in menu " << menu->name
                << " needs format: Objtype, Title, SubMenuName [got '" << value << "']\n";
              throw std::runtime_error("Data error in MENUS.CFG");
			}
			if ( ( stricmp( propname.c_str(), "Entry" ) == 0 ) &&
				 ( submenu_name != "" ) )
			{
              ERROR_PRINT << "Entry in menu " << menu->name
                << " must not specify SubMenuName [got '" << value << "']\n";
              throw std::runtime_error("Data error in MENUS.CFG");
			}

			menu->menuitems_.push_back( MenuItem() );
			MenuItem* mi = &menu->menuitems_.back();

			mi->objtype_ = objtype;
			mi->graphic_ = Items::getgraphic( objtype );
			mi->color_ = Items::getcolor( objtype );

			if ( title != "" )
			  strzcpy( mi->title, title.c_str(), sizeof mi->title );
			else
			  mi->title[0] = '\0';

			if ( submenu_name != "" )
			  strzcpy( mi->submenu_name, submenu_name.c_str(), sizeof mi->submenu_name );
			else
			  mi->submenu_name[0] = '\0';

			mi->submenu_id = 0;
		  }
		  else
		  {
            ERROR_PRINT << "Unexpected property in menu " << menu->name << ": " << propname << "\n";
            throw std::runtime_error("Data error in MENUS.CFG");
		  }
		}
	  }


	  for ( unsigned menuidx = 0; menuidx < gamestate.menus.size(); menuidx++ )
	  {
		Menu *menu = &gamestate.menus[menuidx];
		for ( unsigned itemidx = 0; itemidx < menu->menuitems_.size(); itemidx++ )
		{
		  MenuItem *mi = &menu->menuitems_[itemidx];

		  if ( mi->submenu_name[0] )
		  {
			Menu* found = find_menu( mi->submenu_name );
			if ( found )
			{
			  mi->submenu_id = found->menu_id;
			}
			else
			{
              INFO_PRINT << "Unable to find SubMenu " << mi->submenu_name << "\n";
			}
		  }
		}
	  }
	}
  }
}