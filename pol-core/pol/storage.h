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
  size_t root_item_count() const { return _items.size(); }

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

  /// This file's contribution to a parallel save: its pieces, and how to format any range of
  /// them. A world save folds this in with the other big files so they share the thread pool.
  SavePart save_part( Clib::StreamWriter& sw ) const;
  void read( Clib::ConfigFile& cf );
  void clear();
  size_t estimateSize() const;

private:
  /// One indivisible piece of the storage file: a root item with everything inside it, or - with
  /// no item - the piece that exists only so an empty area still names itself in the file.
  struct StoragePiece
  {
    const std::string* area;
    const Items::Item* item;
  };
  /// The whole file as a list of pieces, areas grouped, which is what makes it splittable.
  std::vector<StoragePiece> collect_pieces() const;
  /// Write piece `i`, opening its storage area first where the loader needs to be told.
  static void print_piece( const std::vector<StoragePiece>& pieces, size_t i, bool first_of_run,
                           Clib::StreamWriter& sw );

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
