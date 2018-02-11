/** @file
 *
 * @par History
 * - 2005/06/01 Shinigami: added getmapshapes - to get access to mapshapes
 * - added getstatics - to fill a list with statics
 * - 2005/06/06 Shinigami: added readmultis derivative - to get a list of statics
 */


#ifndef POL_REALM_H
#define POL_REALM_H

#include <memory>
#include <set>
#include <stddef.h>
#include <string>
#include <vector>

#include "../../plib/mapshape.h"
#include "../../plib/realmdescriptor.h"
#include "../uconst.h"
#include "../udatfile.h"
#include "WorldChangeReasons.h"


namespace Pol
{
namespace Core
{
class ItemsVector;
class ULWObject;
struct Zone;
}
namespace Mobile
{
class Character;
}
namespace Items
{
class Item;
}
namespace Multi
{
class UMulti;
}
namespace Plib
{
class MapServer;
class MapTileServer;
class StaticEntryList;
class StaticServer;
struct MAPTILE_CELL;
}
namespace Realms
{
typedef std::vector<Multi::UMulti*> MultiList;

class Realm
{
public:
  explicit Realm( const std::string& realm_name, const std::string& realm_path = "" );
  explicit Realm( const std::string& realm_name, Realm* realm );
  ~Realm();
  bool is_shadowrealm;
  unsigned int shadowid;
  Realm* baserealm;
  const std::string shadowname;

  unsigned short width() const;
  unsigned short height() const;
  unsigned short grid_width() const;
  unsigned short grid_height() const;

  unsigned season() const;

  bool valid( unsigned short x, unsigned short y, short z ) const;
  const std::string name() const;

  void add_mobile( const Mobile::Character& chr, WorldChangeReason reason );
  void remove_mobile( const Mobile::Character& chr, WorldChangeReason reason );

  void add_toplevel_item( const Items::Item& item );
  void remove_toplevel_item( const Items::Item& item );

  void add_multi( const Multi::UMulti& multi );
  void remove_multi( const Multi::UMulti& multi );

  unsigned int mobile_count() const;
  unsigned int offline_mobile_count() const;
  unsigned int toplevel_item_count() const;
  unsigned int multi_count() const;

  bool walkheight( unsigned short x, unsigned short y, short oldz, short* newz,
                   Multi::UMulti** pmulti, Items::Item** pwalkon, bool doors_block,
                   Core::MOVEMODE movemode, short* gradual_boost = NULL );
  bool walkheight( const Mobile::Character* chr, unsigned short x, unsigned short y, short oldz,
                   short* newz, Multi::UMulti** pmulti, Items::Item** pwalkon,
                   short* gradual_boost = NULL );

  bool lowest_walkheight( unsigned short x, unsigned short y, short oldz, short* newz,
                          Multi::UMulti** pmulti, Items::Item** pwalkon, bool doors_block,
                          Core::MOVEMODE movemode, short* gradual_boost = NULL );

  bool dropheight( unsigned short dropx, unsigned short dropy, short dropz, short chrz, short* newz,
                   Multi::UMulti** pmulti );

  bool has_los( const Core::ULWObject& att, const Core::ULWObject& tgt ) const;

  bool navigable( unsigned short x, unsigned short y, short z, short height ) const;

  Multi::UMulti* find_supporting_multi( unsigned short x, unsigned short y, short z ) const;

  bool lowest_standheight( unsigned short x, unsigned short y, short* z ) const;
  bool findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const;
  void getstatics( Plib::StaticEntryList& statics, unsigned short x, unsigned short y ) const;
  bool groundheight( unsigned short x, unsigned short y, short* z ) const;
  Plib::MAPTILE_CELL getmaptile( unsigned short x, unsigned short y ) const;
  void getmapshapes( Plib::MapShapeList& shapes, unsigned short x, unsigned short y,
                     unsigned int anyflags ) const;
  void readmultis( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                   unsigned int flags ) const;
  void readmultis( Plib::MapShapeList& vec, unsigned short x, unsigned short y, unsigned int flags,
                   MultiList& mvec ) const;
  void readmultis( Core::StaticList& vec, unsigned short x, unsigned short y ) const;

  Core::Zone** zone;
  std::set<unsigned int> global_hulls;  // xy-smashed together
  unsigned getUOMapID() const;
  unsigned getNumStaticPatches() const;
  unsigned getNumMapPatches() const;
  static unsigned int encode_global_hull( unsigned short ax, unsigned short ay );

protected:
  struct LosCache
  {
    LosCache() : last_x( 0 ), last_y( 0 ), shapes(), dyn_items(){};
    unsigned short last_x;
    unsigned short last_y;
    Plib::MapShapeList shapes;
    std::vector<Items::Item*> dyn_items;
  };

  static void standheight( Core::MOVEMODE movemode, Plib::MapShapeList& shapes, short oldz,
                           bool* result, short* newz, short* gradual_boost = NULL );

  static void lowest_standheight( Core::MOVEMODE movemode, Plib::MapShapeList& shapes, short oldz,
                                  bool* result, short* newz, short* gradual_boost = NULL );

  static bool dropheight( Plib::MapShapeList& shapes, short dropz, short chrz, short* newz );

  void readdynamics( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                     Core::ItemsVector& walkon_items, bool doors_block );

  static bool dynamic_item_blocks_los( unsigned short x, unsigned short y, short z,
                                       LosCache& cache );
  bool static_item_blocks_los( unsigned short x, unsigned short y, short z, LosCache& cache ) const;
  bool los_blocked( const Core::ULWObject& att, const Core::ULWObject& target, unsigned short x,
                    unsigned short y, short z, LosCache& cache ) const;

  Multi::UMulti* find_supporting_multi( MultiList& mvec, short z ) const;

private:
  const Plib::RealmDescriptor _descriptor;
  unsigned int _mobile_count;
  unsigned int _offline_count;
  unsigned int _toplevel_item_count;
  unsigned int _multi_count;
  std::unique_ptr<Plib::MapServer> _mapserver;
  std::unique_ptr<Plib::StaticServer> _staticserver;
  std::unique_ptr<Plib::MapTileServer> _maptileserver;

private:
  // not implemented:
  Realm& operator=( const Realm& );
  Realm( const Realm& );

public:
  size_t sizeEstimate() const;
};


inline unsigned int Realm::mobile_count() const
{
  return _mobile_count;
}
inline unsigned int Realm::offline_mobile_count() const
{
  return _offline_count;
}
inline unsigned int Realm::toplevel_item_count() const
{
  return _toplevel_item_count;
}
inline unsigned int Realm::multi_count() const
{
  return _multi_count;
}

inline void Realm::add_toplevel_item( const Items::Item& /*item*/ )
{
  ++_toplevel_item_count;
}
inline void Realm::remove_toplevel_item( const Items::Item& /*item*/ )
{
  --_toplevel_item_count;
}

inline void Realm::add_multi( const Multi::UMulti& /*multi*/ )
{
  ++_multi_count;
}
inline void Realm::remove_multi( const Multi::UMulti& /*multi*/ )
{
  --_multi_count;
}

inline unsigned Realm::getUOMapID() const
{
  return _descriptor.uomapid;
};
inline unsigned Realm::getNumStaticPatches() const
{
  return _descriptor.num_static_patches;
};
inline unsigned Realm::getNumMapPatches() const
{
  return _descriptor.num_map_patches;
};
inline unsigned int Realm::encode_global_hull( unsigned short ax, unsigned short ay )
{
  return ( static_cast<unsigned int>( ax ) << 16 ) | ay;
}

inline unsigned short Realm::width() const
{
  return _descriptor.width;
}
inline unsigned short Realm::height() const
{
  return _descriptor.height;
}
}
}
#endif
