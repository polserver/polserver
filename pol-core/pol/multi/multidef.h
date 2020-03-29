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

#include "../../clib/rawtypes.h"
#include "../../plib/udatfile.h"
#include "../vector.h"

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
  unsigned short objtype;
  s16 x;
  s16 y;
  s16 z;
  bool is_static;
};

class MultiDef
{
public:
  explicit MultiDef( Clib::ConfigElem& elem, u16 multiid );
  ~MultiDef();

  u16 multiid;
  enum HOUSETYPE : u8
  {
    UNKNOWN,
    BOAT,
    HOUSE,
    STAIRS
  } type;

  std::vector<MULTI_ELEM> elems;

  short xbase;  // x[0] is really x[xbase]
  short xsize;
  short ybase;
  short ysize;

  typedef std::vector<const MULTI_ELEM*> HullList;
  HullList hull;
  HullList internal_hull;
  typedef std::set<unsigned short> HullList2;
  HullList2 hull2;
  HullList2 internal_hull2;

  typedef std::multimap<unsigned short, const MULTI_ELEM*> Components;
  typedef std::pair<Components::const_iterator, Components::const_iterator> ItrPair;

  short minrx, minry, minrz;  // minimum relative distances
  short maxrx, maxry, maxrz;
  Components components;

  static short global_minrx;
  static short global_minry;
  static short global_minrz;
  static short global_maxrx;
  static short global_maxry;
  static short global_maxrz;

  ItrPair findcomponents( short rx, short ry );

  bool findcomponents( Components::const_iterator& beg, Components::const_iterator& end, short rx,
                       short ry ) const;

  static unsigned short getkey( const Core::Vec2d& rxy );

  // returns true if it finds anything at this rx,ry
  bool readobjects( Plib::StaticList& vec, short rx, short ry, short zbase ) const;
  bool readshapes( Plib::MapShapeList& vec, short rx, short ry, short zbase,
                   unsigned int anyflags ) const;

  bool body_contains( const Core::Vec2d& rxy ) const;
  const MULTI_ELEM* find_component( short rx, short ry ) const;

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

bool MultiDefByMultiIDExists( u16 multiid );
const MultiDef* MultiDefByMultiID( u16 multiid );

inline unsigned short MultiDef::getkey( const Core::Vec2d& rxy )
{
  unsigned char crx = static_cast<unsigned char>( rxy.getX() );
  unsigned char cry = static_cast<unsigned char>( rxy.getY() );

  unsigned short key = ( crx << 8 ) | cry;
  return key;
}
}  // namespace Multi
}  // namespace Pol
#endif
