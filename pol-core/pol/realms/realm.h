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

#include "plib/mapshape.h"
#include "plib/maptile.h"
#include "plib/realmdescriptor.h"
#include "plib/uconst.h"
#include "plib/udatfile.h"
#include "plib/mapcell.h"

#include "base/position.h"
#include "base/range.h"
#include "realms/WorldChangeReasons.h"
#include "zone.h"

namespace Pol
{
namespace Core
{
class ItemsVector;
class ULWObject;
}  // namespace Core
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
}  // namespace Plib
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
  Core::Range2d area() const;
  Core::Range2d gridarea() const;

  Core::Zone& getzone_grid( unsigned short x, unsigned short y ) const;  // TODO Pos
  Core::Zone& getzone_grid( const Core::Pos2d& pos ) const;
  Core::Zone& getzone( unsigned short x, unsigned short y ) const;
  Core::Zone& getzone( const Core::Pos2d& p ) const;

  unsigned season() const;

  bool valid( unsigned short x, unsigned short y, short /*z*/ ) const
  {
    return valid( Core::Pos2d( x, y ) );
  };  // TODO Pos
  bool valid( const Core::Pos2d& p ) const;
  bool valid( const Core::Pos3d& p ) const;
  const std::string name() const;

  // functions to broadcast entered- and leftarea events to items and npcs in the realm
  void notify_moved( Mobile::Character& whomoved );
  void notify_unhid( Mobile::Character& whounhid );
  void notify_resurrected( Mobile::Character& whoressed );
  void notify_entered( Mobile::Character& whoentered );
  void notify_left( Mobile::Character& wholeft );

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
                   Plib::MOVEMODE movemode, short* gradual_boost = nullptr )  // TODO Pos
  {
    return walkheight( Core::Pos2d( x, y ), oldz, newz, pmulti, pwalkon, doors_block, movemode,
                       gradual_boost );
  }
  bool walkheight( const Core::Pos2d& p, short oldz, short* newz, Multi::UMulti** pmulti,
                   Items::Item** pwalkon, bool doors_block, Plib::MOVEMODE movemode,
                   short* gradual_boost = nullptr );
  bool walkheight( const Mobile::Character* chr, unsigned short x, unsigned short y, short oldz,
                   short* newz, Multi::UMulti** pmulti, Items::Item** pwalkon,
                   short* gradual_boost = nullptr )  // TODO Pos
  {
    return walkheight( chr, Core::Pos2d( x, y ), oldz, newz, pmulti, pwalkon, gradual_boost );
  }
  bool walkheight( const Mobile::Character* chr, const Core::Pos2d& p, short oldz, short* newz,
                   Multi::UMulti** pmulti, Items::Item** pwalkon, short* gradual_boost = nullptr );

  bool lowest_walkheight( unsigned short x, unsigned short y, short oldz, short* newz,
                          Multi::UMulti** pmulti, Items::Item** pwalkon, bool doors_block,
                          Plib::MOVEMODE movemode, short* gradual_boost = nullptr )  // TODO Pos
  {
    return lowest_walkheight( Core::Pos2d( x, y ), oldz, newz, pmulti, pwalkon, doors_block,
                              movemode, gradual_boost );
  }
  bool lowest_walkheight( const Core::Pos2d& p, short oldz, short* newz, Multi::UMulti** pmulti,
                          Items::Item** pwalkon, bool doors_block, Plib::MOVEMODE movemode,
                          short* gradual_boost = nullptr );

  [[nodiscard]] std::vector<std::tuple<short, Multi::UMulti *, Items::Item *>>
  get_walkheights(unsigned short x, unsigned short y, short minz, short maxz,
                   Plib::MOVEMODE movemode, bool doors_block) const
  {
    return get_walkheights(Core::Pos2d(x, y), minz, maxz, movemode, doors_block);
  }

  [[nodiscard]] std::vector<std::tuple<short, Multi::UMulti *, Items::Item *>>
  get_walkheights(const Core::Pos2d& pos, short minz, short maxz,
                   Plib::MOVEMODE movemode, bool doors_block) const;

  bool dropheight( unsigned short dropx, unsigned short dropy, short dropz, short chrz, short* newz,
                   Multi::UMulti** pmulti )  // TODO Pos
  {
    return dropheight( Core::Pos3d( dropx, dropy, (s8)dropz ), chrz, newz, pmulti );
  }
  bool dropheight( const Core::Pos3d& drop, short chrz, short* newz, Multi::UMulti** pmulti );

  bool has_los( const Core::ULWObject& att, const Core::ULWObject& tgt ) const;

  bool navigable( unsigned short x, unsigned short y, short z, short height ) const  // TODO Pos
  {
    return navigable( Core::Pos3d( x, y, (s8)z ), height );
  }
  bool navigable( const Core::Pos3d& p, short height ) const;

  Multi::UMulti* find_supporting_multi( unsigned short x, unsigned short y,
                                        short z ) const  // TODO Pos
  {
    return find_supporting_multi( Core::Pos3d( x, y, (s8)z ) );
  }
  Multi::UMulti* find_supporting_multi( const Core::Pos3d& pos ) const;

  bool lowest_standheight( unsigned short x, unsigned short y, short* z ) const  // TODO Pos
  {
    return lowest_standheight( Core::Pos2d( x, y ), z );
  }
  bool lowest_standheight( const Core::Pos2d& pos, short* z ) const;
  bool findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const  // TODO Pos
  {
    return findstatic( Core::Pos2d( x, y ), objtype );
  }
  bool findstatic( const Core::Pos2d& pos, unsigned short objtype ) const;
  void getstatics( Plib::StaticEntryList& statics, unsigned short x, unsigned short y ) const
  {
    return getstatics( statics, Core::Pos2d( x, y ) );
  }
  void getstatics( Plib::StaticEntryList& statics, const Core::Pos2d& pos ) const;
  bool groundheight( unsigned short x, unsigned short y, short* z ) const  // TODO Pos
  {
    return groundheight( Core::Pos2d( x, y ), z );
  }
  bool groundheight( const Core::Pos2d& pos, short* z ) const;
  Plib::MAPTILE_CELL getmaptile( unsigned short x, unsigned short y ) const  // TODO Pos
  {
    return getmaptile( Core::Pos2d( x, y ) );
  }
  Plib::MAPTILE_CELL getmaptile( const Core::Pos2d& pos ) const;
  void getmapshapes( Plib::MapShapeList& shapes, unsigned short x, unsigned short y,
                     unsigned int anyflags ) const  // TODO Pos
  {
    return getmapshapes( shapes, Core::Pos2d( x, y ), anyflags );
  }
  void getmapshapes( Plib::MapShapeList& shapes, const Core::Pos2d& pos,
                     unsigned int anyflags ) const;
  void readmultis( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                   unsigned int flags ) const  // TODO Pos
  {
    return readmultis( vec, Core::Pos2d( x, y ), flags );
  }
  void readmultis( Plib::MapShapeList& vec, const Core::Pos2d& pos, unsigned int flags ) const;
  void readmultis( Plib::MapShapeList& vec, unsigned short x, unsigned short y, unsigned int flags,
                   MultiList& mvec ) const  // TODO Pos
  {
    return readmultis( vec, Core::Pos2d( x, y ), flags, mvec );
  }
  void readmultis( Plib::MapShapeList& vec, const Core::Pos2d& pos, unsigned int flags,
                   MultiList& mvec ) const;
  void readmultis( Plib::StaticList& vec, unsigned short x, unsigned short y ) const  // TODO Pos
  {
    return readmultis( vec, Core::Pos2d( x, y ) );
  }
  void readmultis( Plib::StaticList& vec, const Core::Pos2d& pos ) const;

  void readdynamics( Plib::MapShapeList& vec, const Core::Pos2d& pos,
                     Core::ItemsVector& walkon_items, bool doors_block, unsigned int flags ) const;

  void readdynamics( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                     Core::ItemsVector& walkon_items, bool doors_block, unsigned int flags ) const
  {
    return readdynamics( vec, Core::Pos2d( x, y ), walkon_items, doors_block, flags );
  }

  void readdynamics( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                     Core::ItemsVector& walkon_items, bool doors_block ) const
  {
    return readdynamics( vec, Core::Pos2d( x, y ), walkon_items, doors_block,
                         Plib::FLAG::WALKBLOCK );
  }

  void readdynamics( Plib::MapShapeList& vec, const Core::Pos2d& pos,
                     Core::ItemsVector& walkon_items, bool doors_block ) const
  {
    return readdynamics( vec, pos, walkon_items, doors_block, Plib::FLAG::WALKBLOCK );
  }

  std::set<unsigned int> global_hulls;  // xy-smashed together
  unsigned getUOMapID() const;
  unsigned getNumStaticPatches() const;
  unsigned getNumMapPatches() const;
  static unsigned int encode_global_hull( unsigned short ax, unsigned short ay );  // TODO Pos
  static unsigned int encode_global_hull( const Core::Pos2d& pos );

protected:
  struct LosCache
  {
    LosCache() : last_pos(), shapes(), dyn_items(){};
    Core::Pos2d last_pos;
    Plib::MapShapeList shapes;
    std::vector<Items::Item*> dyn_items;
  };

  static void standheight( Plib::MOVEMODE movemode, Plib::MapShapeList& shapes, short oldz,
                           bool* result, short* newz, short* gradual_boost = nullptr );

  static void lowest_standheight( Plib::MOVEMODE movemode, Plib::MapShapeList& shapes, short oldz,
                                  bool* result, short* newz, short* gradual_boost = nullptr );

  static Plib::MapShapeList get_standheights( Plib::MOVEMODE movemode, Plib::MapShapeList shapes,
                                           short minz, short maxz );

  static bool dropheight( Plib::MapShapeList& shapes, short dropz, short chrz, short* newz );

  static bool dynamic_item_blocks_los( const Core::Pos3d& pos, LosCache& cache );
  bool static_item_blocks_los( const Core::Pos3d& pos, LosCache& cache ) const;
  bool los_blocked( const Core::ULWObject& att, const Core::ULWObject& target,
                    const Core::Pos3d& pos, LosCache& cache ) const;

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
  Core::Zone** zone;  // y first
  Core::Range2d _area;
  Core::Range2d _gridarea;

public:
  size_t sizeEstimate() const;
  Realm& operator=( const Realm& ) = delete;
  Realm( const Realm& ) = delete;
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
inline unsigned int Realm::encode_global_hull( const Core::Pos2d& pos )
{
  return ( static_cast<unsigned int>( pos.x() ) << 16 ) | pos.y();
}

inline unsigned short Realm::width() const
{
  return _descriptor.width;
}
inline unsigned short Realm::height() const
{
  return _descriptor.height;
}
inline Core::Range2d Realm::area() const
{
  return _area;
}
inline Core::Range2d Realm::gridarea() const
{
  return _gridarea;
}

inline Core::Zone& Realm::getzone_grid( unsigned short x, unsigned short y ) const
{
  return zone[y][x];
}
inline Core::Zone& Realm::getzone_grid( const Core::Pos2d& p ) const
{
  return zone[p.y()][p.x()];
}
inline Core::Zone& Realm::getzone( unsigned short x, unsigned short y ) const
{
  return getzone_grid( x >> Plib::WGRID_SHIFT, y >> Plib::WGRID_SHIFT );
}
inline Core::Zone& Realm::getzone( const Core::Pos2d& p ) const
{
  return getzone_grid( Core::Pos2d( p.x() >> Plib::WGRID_SHIFT, p.y() >> Plib::WGRID_SHIFT ) );
}


}  // namespace Realms
}  // namespace Pol
#endif
