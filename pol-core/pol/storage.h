/** @file
 *
 * @par History
 */


#ifndef H_STORAGE_H
#define H_STORAGE_H

#include <functional>
#include <map>
#include <string>

#include "clib/maputil.h"

namespace Pol
{
namespace Items
{
class Item;
}
namespace Realms
{
class Realm;
}
namespace Clib
{
class ConfigFile;
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Core
{
class StorageArea
{
public:
  StorageArea( std::string name );
  ~StorageArea();

  Items::Item* find_root_item( const std::string& name );
  void insert_root_item( Items::Item* item );
  bool delete_root_item( const std::string& name );
  /**
   * Unlink a root item without destroying it, for a move to some other home.
   *
   * Takes the key the item was filed under rather than deriving it from the item, because
   * renaming a root item leaves the map key behind and the item unreachable by its new name.
   */
  bool remove_root_item( const std::string& key, Items::Item* item );

  /// Visit each root item with the key it is filed under, which is not always its current name.
  void for_each_root_item( const std::function<void( const std::string&, Items::Item* )>& f ) const;

  void print( Clib::StreamWriter& sw ) const;
  void load_item( Clib::ConfigElem& elem );
  size_t estimateSize() const;

private:
  std::string _name;

  // TODO: ref_ptr<Item> ?
  using Cont = std::map<std::string, Items::Item*, Clib::ci_cmp_pred>;
  Cont _items;  // owns its items.

  friend class StorageAreaImp;
  friend class StorageAreaIterator;
};

class Storage
{
public:
  StorageArea* find_area( const std::string& name );
  StorageArea* create_area( const std::string& name );
  StorageArea* create_area( Clib::ConfigElem& elem );

  void for_each_area( const std::function<void( StorageArea& )>& f ) const;

  void print( Clib::StreamWriter& sw ) const;
  void read( Clib::ConfigFile& cf );
  void clear();
  size_t estimateSize() const;

private:
  // TODO: investigate if this could store objects. Does find()
  // return object copies, or references?
  using AreaCont = std::map<std::string, StorageArea*>;
  AreaCont areas;

  friend class StorageAreasImp;
  friend class StorageAreasIterator;
};
}  // namespace Core
}  // namespace Pol
#endif
