/** @file
 *
 * @par History
 */


#ifndef H_STORAGE_H
#define H_STORAGE_H

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "clib/maputil.h"
#include "pol/saveparallel.h"

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
  /// This file's contribution to a parallel save: its pieces, and how to format any range of
  /// them. A world save folds this in with the other big files so they share the thread pool.
  SavePart save_part( Clib::StreamWriter& sw ) const;
  /// The same bytes print() writes, built on this thread alone. print() splits the work across
  /// the task pool once a world is big enough to be worth it and falls back to this otherwise;
  /// the test suite holds the two against each other.
  void print_single_threaded( Clib::StreamWriter& sw ) const;
  void read( Clib::ConfigFile& cf );
  void clear();
  size_t estimateSize() const;

private:
  /// One indivisible piece of the storage file, in the order it has to be written: either the
  /// header that opens an area, or one root item with everything inside it.
  struct PrintUnit
  {
    const std::string* area_name;  // set on the unit that opens an area
    const Items::Item* item;       // set on a root item unit
  };
  /// The whole file as an ordered list of pieces, which is what makes it splittable.
  std::vector<PrintUnit> collect_print_units() const;
  static void print_unit( const PrintUnit& unit, Clib::StreamWriter& sw );

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
