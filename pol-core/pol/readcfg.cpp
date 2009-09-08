/*
History
=======


Notes
=======

*/

#include "clib/stl_inc.h"
#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "clib/cfgelem.h"
#include "clib/cfgfile.h"
#include "clib/cistring.h"
#include "clib/clib.h"
#include "clib/fileutil.h"
#include "clib/stlutil.h"

#include "item/itemdesc.h"
#include "menu.h"
#include "polcfg.h"

void read_menus( void )
{
	if ( !FileExists("config/menus.cfg") )
	{
		if ( config.loglevel > 1 )
			cout << "File config/menus.cfg not found, skipping.\n";
		return;
	}

	ConfigFile cf( "config/menus.cfg" );

	ConfigElem elem;

    while (cf.read( elem ))
	{
		string menu_name;
		string menu_title;
        std::string propname;
        std::string value;

		if (stricmp( elem.type(), "ItemMenu" ) != 0)
			continue;

		menu_name = elem.remove_string( "NAME" );
		menu_title = elem.remove_string( "TITLE" );
		
        menus.push_back( Menu() );
        Menu* menu = &menus.back();

		menu->menu_id = static_cast<unsigned short>(menus.size());
		strzcpy( menu->name, menu_name.c_str(), sizeof menu->name );
		strzcpy( menu->title, menu_title.c_str(), sizeof menu->title );
		
		while (elem.remove_first_prop( &propname, &value ))
		{
			if (stricmp( propname.c_str(), "SubMenu" ) == 0 ||
				stricmp( propname.c_str(), "Entry" ) == 0)
			{
                string submenu_name;
                string objtype_str;
                string title;

				/*
                char *submenu_name = NULL;
				char *objtype_str = NULL;
				char *title = NULL;
				*/

				if (stricmp( propname.c_str(), "SubMenu" ) == 0)
				{
                    ISTRINGSTREAM is(value);
                    is >> submenu_name >> objtype_str;

                    // skipws
                    // is.eatwhite();

                    std::getline( is, title ); // title.getline( is );
                    
                    // FIXME: remove leading spaces (better way? please? anyone?)
                    while (isspace(title[0]))
                        title.erase(0, 1);

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
					ISTRINGSTREAM is(value);
                    is >> objtype_str; // FIXME objtype_str ends up having the "," on the end
                    
                    std::getline( is, title );
                    // FIXME: remove leading spaces (better way? please? anyone?)
                    while (isspace(title[0]))
                        title.erase(0, 1);
				}
				
				if (objtype_str == "")
				{
					cerr << "Entry in menu " << menu->name 
                         << " must provide at least an object type" << endl;
					throw runtime_error( "Data error in MENUS.CFG" );
				}
				u16 objtype = (u16) strtoul( objtype_str.c_str(), NULL, 0 );
				if (objtype == 0) // 0 specified, or text
				{
					cerr << "Entry in menu " << menu->name 
                         << " cannot specify [" << objtype_str << "] as an Object Type." << endl;
					throw runtime_error( "Data error in MENUS.CFG" );
				}
				if ((stricmp( propname.c_str(), "SubMenu" ) == 0) && 
					(submenu_name == ""))
				{
					cerr << "SubMenu in menu " << menu->name
                         << " needs format: Objtype, Title, SubMenuName [got '" << value << "']" << endl;
					throw runtime_error( "Data error in MENUS.CFG" );
				}
				if ((stricmp( propname.c_str(), "Entry" ) == 0) &&
					(submenu_name != ""))
				{
					cerr << "Entry in menu " << menu->name
                         << " must not specify SubMenuName [got '" << value << "']" << endl;
					throw runtime_error( "Data error in MENUS.CFG" );
				}
				
				menu->menuitems_.push_back( MenuItem() );
                MenuItem* mi = &menu->menuitems_.back();
				
				mi->objtype_ = objtype;
                mi->graphic_ = getgraphic( objtype );
                mi->color_ = getcolor( objtype );
				
				if (title != "")
					strzcpy( mi->title, title.c_str(), sizeof mi->title );
				else
					mi->title[0] = '\0';
				
				if (submenu_name != "")
					strzcpy( mi->submenu_name, submenu_name.c_str(), sizeof mi->submenu_name );
				else
					mi->submenu_name[0] = '\0';

				mi->submenu_id = 0;
			}
			else
			{
				cerr << "Unexpected property in menu " << menu->name << ": " << propname << endl;
				throw runtime_error( "Data error in MENUS.CFG" );
			}
		}
	}


	for( unsigned menuidx = 0; menuidx < menus.size(); menuidx++ )
	{
		Menu *menu = &menus[ menuidx ];
		for( unsigned itemidx = 0; itemidx < menu->menuitems_.size(); itemidx++ )
		{
			MenuItem *mi = &menu->menuitems_[ itemidx ];
			
			if (mi->submenu_name[0])
			{
				Menu* found = find_menu( mi->submenu_name );
				if (found)
				{
					mi->submenu_id = found->menu_id;
				}
				else
				{
					cout << "Unable to find SubMenu " << mi->submenu_name << endl;
				}
			}
		}
	}

	//TailoringMenu = find_menu( "Tailoring" );
	//ItemCreationMenu = find_menu( "ItemCreation" );
}
