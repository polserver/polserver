/** @file
 *
 * @par History
 * - 2009/09/01 Turley:    VS2005/2008 Support moved inline MultiDef::getkey to .h
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#ifndef MULTIDEF_H
#define MULTIDEF_H

// also consider: multimap<unsigned int, unsigned int>
// unsigned int is (x << 16) | y
// unsigned int is z << 16 | objtype
// hell, even multimap<unsigned short,unsigned int>
// unsigned short is (x<<8)|y
// (relative signed x/y, -128 to +127
// unsigned int is z << 16 | objtype


#include <cstdio>
#include <map>
#include <set>
#include <vector>

#include "clib/rawtypes.h"
#include "plib/udatfile.h"
#include "pol/base/vector.h"  // uotool also uses this file :(

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Plib
{
class MapShapeList;
}
namespace Multi
{
extern void read_multidefs();

struct MULTI_ELEM
{
  Core::Vec3d relpos;
  unsigned short objtype;
  bool is_static;
};

class MultiDef
{
public:
  explicit MultiDef( Clib::ConfigElem& elem, u16 multiid );
  ~MultiDef() = default;

  u16 multiid;
  bool is_boat;

  std::vector<MULTI_ELEM> elems;

  using HullList = std::vector<const MULTI_ELEM*>;
  HullList hull;
  HullList internal_hull;
  using HullList2 = std::set<unsigned short>;
  HullList2 hull2;
  HullList2 internal_hull2;

  using Components = std::multimap<unsigned short, const MULTI_ELEM*>;
  using ItrPair = std::pair<Components::const_iterator, Components::const_iterator>;

  Core::Vec3d minrxyz;  // minimum relative distances
  Core::Vec3d maxrxyz;
  u8 max_radius;
  Components components;

  ItrPair findcomponents( const Core::Vec2d& rxy );

  bool findcomponents( Components::const_iterator& beg, Components::const_iterator& end,
                       const Core::Vec2d& rxy ) const;

  static unsigned short getkey( const Core::Vec2d& rxy );

  // returns true if it finds anything at this rx,ry
  bool readobjects( Plib::StaticList& vec, const Core::Vec2d& rxy, short zbase ) const;
  bool readshapes( Plib::MapShapeList& vec, const Core::Vec2d& rxy, short zbase,
                   unsigned int anyflags ) const;

  bool body_contains( const Core::Vec2d& rxy ) const;
  bool within_multi( const Core::Vec2d& relxy ) const;
  const MULTI_ELEM* find_component( const Core::Vec2d& rxy ) const;

  void add_to_hull( const MULTI_ELEM* elem );
  void add_to_internal_hull( const MULTI_ELEM* elem );
  void add_row_tohull( short y );
  void add_body_tohull();
  void eliminate_hull_dupes();
  void computehull();
  void addrec( const MULTI_ELEM* elem );
  void fill_hull2();

  void init();

  size_t estimateSize() const;
};

void read_multidefs();

bool MultiDefByMultiIDExists( u16 multiid );
const MultiDef* MultiDefByMultiID( u16 multiid );

inline unsigned short MultiDef::getkey( const Core::Vec2d& rxy )
{
  unsigned char crx = static_cast<unsigned char>( rxy.x() );
  unsigned char cry = static_cast<unsigned char>( rxy.y() );

  unsigned short key = ( crx << 8 ) | cry;
  return key;
}
}  // namespace Multi
}  // namespace Pol
#endif
