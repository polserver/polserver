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
  unsigned season() const;

  bool valid( unsigned short x, unsigned short y, short z ) const;
  bool valid( const Core::Pos3d& p ) const { return valid( p.x(), p.y(), p.z() ); }  // TODO Pos
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
                   Plib::MOVEMODE movemode, short* gradual_boost = nullptr );
  bool walkheight( const Core::Pos2d& p, short oldz, short* newz, Multi::UMulti** pmulti,
                   Items::Item** pwalkon, bool doors_block, Plib::MOVEMODE movemode,
                   short* gradual_boost = nullptr )  // TODO Pos
  {
    return walkheight( p.x(), p.y(), oldz, newz, pmulti, pwalkon, doors_block, movemode,
                       gradual_boost );
  }
  bool walkheight( const Mobile::Character* chr, unsigned short x, unsigned short y, short oldz,
                   short* newz, Multi::UMulti** pmulti, Items::Item** pwalkon,
                   short* gradual_boost = nullptr );
  bool walkheight( const Mobile::Character* chr, const Core::Pos2d& p, short oldz, short* newz,
                   Multi::UMulti** pmulti, Items::Item** pwalkon,
                   short* gradual_boost = nullptr )  // TODO Pos
  {
    return walkheight( chr, p.x(), p.y(), oldz, newz, pmulti, pwalkon, gradual_boost );
  }

  bool lowest_walkheight( unsigned short x, unsigned short y, short oldz, short* newz,
                          Multi::UMulti** pmulti, Items::Item** pwalkon, bool doors_block,
                          Plib::MOVEMODE movemode, short* gradual_boost = nullptr );
  bool lowest_walkheight( const Core::Pos2d& p, short oldz, short* newz, Multi::UMulti** pmulti,
                          Items::Item** pwalkon, bool doors_block, Plib::MOVEMODE movemode,
                          short* gradual_boost = nullptr )  // TODO Pos
  {
    return lowest_walkheight( p.x(), p.y(), oldz, newz, pmulti, pwalkon, doors_block, movemode,
                              gradual_boost );
  }

  bool dropheight( unsigned short dropx, unsigned short dropy, short dropz, short chrz, short* newz,
                   Multi::UMulti** pmulti );
  bool dropheight( const Core::Pos3d& drop, short chrz, short* newz,
                   Multi::UMulti** pmulti )  // TODO Pos
  {
    return dropheight( drop.x(), drop.y(), drop.z(), chrz, newz, pmulti );
  }

  bool has_los( const Core::ULWObject& att, const Core::ULWObject& tgt ) const;

  bool navigable( unsigned short x, unsigned short y, short z, short height ) const;
  bool navigable( const Core::Pos3d& p, short height ) const  // TODO Pos
  {
    return navigable( p.x(), p.y(), p.z(), height );
  }

  Multi::UMulti* find_supporting_multi( unsigned short x, unsigned short y, short z ) const;
  Multi::UMulti* find_supporting_multi( const Core::Pos3d& pos ) const  // TODO Pos
  {
    return find_supporting_multi( pos.x(), pos.y(), pos.z() );
  }

  bool lowest_standheight( unsigned short x, unsigned short y, short* z ) const;
  bool lowest_standheight( const Core::Pos2d& pos, short* z ) const  // TODO Pos
  {
    return lowest_standheight( pos.x(), pos.y(), z );
  }
  bool findstatic( unsigned short x, unsigned short y, unsigned short objtype ) const;
  bool findstatic( const Core::Pos2d& pos, unsigned short objtype ) const  // TODO Pos
  {
    return findstatic( pos.x(), pos.y(), objtype );
  }
  void getstatics( Plib::StaticEntryList& statics, unsigned short x, unsigned short y ) const;
  void getstatics( Plib::StaticEntryList& statics, const Core::Pos2d& pos ) const  // TODO Pos
  {
    return getstatics( statics, pos.x(), pos.y() );
  }
  bool groundheight( unsigned short x, unsigned short y, short* z ) const;
  bool groundheight( const Core::Pos2d& pos, short* z ) const  // TODO Pos
  {
    return groundheight( pos.x(), pos.y(), z );
  }
  Plib::MAPTILE_CELL getmaptile( unsigned short x, unsigned short y ) const;
  Plib::MAPTILE_CELL getmaptile( const Core::Pos2d& pos ) const  // TODO Pos
  {
    return getmaptile( pos.x(), pos.y() );
  }
  void getmapshapes( Plib::MapShapeList& shapes, unsigned short x, unsigned short y,
                     unsigned int anyflags ) const;
  void getmapshapes( Plib::MapShapeList& shapes, const Core::Pos2d& pos,
                     unsigned int anyflags ) const  // TODO Pos
  {
    return getmapshapes( shapes, pos.x(), pos.y(), anyflags );
  }
  void readmultis( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                   unsigned int flags ) const;
  void readmultis( Plib::MapShapeList& vec, const Core::Pos2d& pos,
                   unsigned int flags ) const  // TODO Pos
  {
    return readmultis( vec, pos.x(), pos.y(), flags );
  }
  void readmultis( Plib::MapShapeList& vec, unsigned short x, unsigned short y, unsigned int flags,
                   MultiList& mvec ) const;
  void readmultis( Plib::MapShapeList& vec, const Core::Pos2d& pos, unsigned int flags,
                   MultiList& mvec ) const  // TODO Pos
  {
    return readmultis( vec, pos.x(), pos.y(), flags, mvec );
  }
  void readmultis( Plib::StaticList& vec, unsigned short x, unsigned short y ) const;
  void readmultis( Plib::StaticList& vec, const Core::Pos2d& pos ) const  // TODO Pos
  {
    return readmultis( vec, pos.x(), pos.y() );
  }

  std::set<unsigned int> global_hulls;  // xy-smashed together
  unsigned getUOMapID() const;
  unsigned getNumStaticPatches() const;
  unsigned getNumMapPatches() const;
  static unsigned int encode_global_hull( unsigned short ax, unsigned short ay );
  static unsigned int encode_global_hull( const Core::Pos2d& pos )  // TODO Pos
  {
    return encode_global_hull( pos.x(), pos.y() );
  }

protected:
  struct LosCache
  {
    LosCache() : last_x( 0 ), last_y( 0 ), shapes(), dyn_items(){};
    unsigned short last_x;
    unsigned short last_y;
    Plib::MapShapeList shapes;
    std::vector<Items::Item*> dyn_items;
  };

  static void standheight( Plib::MOVEMODE movemode, Plib::MapShapeList& shapes, short oldz,
                           bool* result, short* newz, short* gradual_boost = nullptr );

  static void lowest_standheight( Plib::MOVEMODE movemode, Plib::MapShapeList& shapes, short oldz,
                                  bool* result, short* newz, short* gradual_boost = nullptr );

  static bool dropheight( Plib::MapShapeList& shapes, short dropz, short chrz, short* newz );

  void readdynamics( Plib::MapShapeList& vec, unsigned short x, unsigned short y,
                     Core::ItemsVector& walkon_items, bool doors_block );
  void readdynamics( Plib::MapShapeList& vec, const Core::Pos2d& pos,
                     Core::ItemsVector& walkon_items, bool doors_block )  // TODO Pos
  {
    return readdynamics( vec, pos.x(), pos.y(), walkon_items, doors_block );
  }

  static bool dynamic_item_blocks_los( unsigned short x, unsigned short y, short z,
                                       LosCache& cache );
  static bool dynamic_item_blocks_los( const Core::Pos3d& pos,
                                       LosCache& cache )  // TODO Pos
  {
    return dynamic_item_blocks_los( pos.x(), pos.y(), pos.z(), cache );
  }
  bool static_item_blocks_los( unsigned short x, unsigned short y, short z, LosCache& cache ) const;
  bool static_item_blocks_los( const Core::Pos3d& pos, LosCache& cache ) const  // TODO Pos
  {
    return static_item_blocks_los( pos.x(), pos.y(), pos.z(), cache );
  }
  bool los_blocked( const Core::ULWObject& att, const Core::ULWObject& target, unsigned short x,
                    unsigned short y, short z, LosCache& cache ) const;
  bool los_blocked( const Core::ULWObject& att, const Core::ULWObject& target,
                    const Core::Pos3d& pos, LosCache& cache ) const  // TODO Pos
  {
    return los_blocked( att, target, pos.x(), pos.y(), pos.z(), cache );
  }

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
  return Core::Range2d( Core::Pos2d( 0, 0 ), Core::Pos2d( width() - 1, height() - 1 ), nullptr );
}
inline Core::Range2d Realm::gridarea() const
{
  return Core::Range2d( Core::Pos2d( 0, 0 ), Core::Pos2d( grid_width() - 1, grid_height() - 1 ),
                        nullptr );
}

inline Core::Zone& Realm::getzone_grid( unsigned short x, unsigned short y ) const
{
  return zone[y][x];
}
inline Core::Zone& Realm::getzone_grid( const Core::Pos2d& p ) const
{
  return zone[p.y()][p.x()];
}
}  // namespace Realms
}  // namespace Pol
#endif
