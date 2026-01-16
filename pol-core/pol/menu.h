/** @file
 *
 * @par History
 */


#ifndef __MENU_H
#define __MENU_H

#ifndef __CLIB_RAWTYPES_H
#include "../clib/rawtypes.h"
#endif
#include <cstddef>
#include <vector>

#include "../clib/weakptr.h"


namespace Pol::Core
{
// FIXME these are horribly wasteful.  Probably should be using <string> or something.
// also, kinda bad for each MenuItem to have a submenu_name, when most won't be submenus...sigh.

class MenuItem
{
public:
  MenuItem();

  // memberwise copy is okay for these:
  // MenuItem( const MenuItem& );
  // MenuItem& operator=( const MenuItem& );

  u32 objtype_;
  u16 graphic_;
  u16 color_;
  char title[80];
  char submenu_name[80];
  unsigned short submenu_id;

private:  // not implemented
};

class Menu
{
public:
  Menu();

  // Those are needed to update the weak_ptr_owner
  Menu( const Menu& );
  Menu& operator=( const Menu& );

  unsigned short menu_id;
  char name[80];
  char title[80];

  std::vector<MenuItem> menuitems_;
  size_t estimateSize() const;

  static void read_menus();
  static Menu* find_menu( const char* name );
  static Menu* find_menu( unsigned short menu_id );

  weak_ptr<Menu> getWeakPtr() const { return weakptr; }

private:
  weak_ptr_owner<Menu> weakptr;
};
}  // namespace Pol::Core

#endif
