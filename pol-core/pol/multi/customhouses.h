/** @file
 *
 * @par History
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#ifndef _CUSTOMHOUSES_H
#define _CUSTOMHOUSES_H

#include <cstddef>  // for size_t
#include <iosfwd>   // for testprint()
#include <list>
#include <string>
#include <vector>

#include "clib/rawtypes.h"

#include "base/position.h"
#include "base/vector.h"

namespace Pol
{
namespace Bscript
{
class ObjArray;
}
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Network
{
class Client;
}
namespace Items
{
class Item;
}
namespace Mobile
{
class Character;
}
namespace Multi
{
class UHouse;
}
namespace Core
{
struct PKTBI_D7;
}
namespace Multi
{
#define CUSTOM_HOUSE_NUM_PLANES 6
#define STAIR_MULTIID_MIN 0x1DB0
#define STAIR_MULTIID_MAX 0x1DF3
#define DIRTY_TILE 0x31F4
#define TELEPORTER_START 0x181D
#define TELEPORTER_END 0x1828

class UHouse;

enum
{
  HOUSE_DESIGN_CURRENT = 1,
  HOUSE_DESIGN_WORKING
};

struct CUSTOM_HOUSE_ELEMENT
{
  u8 z;
  u16 graphic;
  Core::Vec2d offset;
};

typedef std::vector<std::vector<std::list<CUSTOM_HOUSE_ELEMENT>>> HouseFloor;  // vector of N-S rows
typedef std::vector<std::list<CUSTOM_HOUSE_ELEMENT>> HouseFloorRow;  // vector of Z columns
typedef std::list<CUSTOM_HOUSE_ELEMENT> HouseFloorZColumn;
// [][][][][][] - Xoffset vector
// | | | | | |  - vec of yoffsets
// v v v v v v
// [][][][][][] - lists of zoffsets
// [][][][][][]

class CustomHouseElements
{
public:
  CustomHouseElements();
  ~CustomHouseElements() = default;

  void SetSize( Core::Pos2d size );
  void SetOffset( Core::Vec2d off );
  size_t estimatedSize() const;
  HouseFloorZColumn* GetElementsAt( const Core::Vec2d& off );
  HouseFloorZColumn* GetElementsAt( const Core::Pos2d& p );
  HouseFloor& GetFloor();
  const HouseFloor& GetFloor() const;
  void AddElement( CUSTOM_HOUSE_ELEMENT& elem );

private:
  HouseFloor data;
  Core::Pos2d size;
  Core::Vec2d offset;
};

class CustomHouseDesign
{
public:
  CustomHouseDesign();
  CustomHouseDesign( Core::Pos2d size, Core::Vec2d offset );
  ~CustomHouseDesign() = default;
  size_t estimatedSize() const;
  void InitDesign( Core::Pos2d size, Core::Vec2d offset );

  CustomHouseDesign& operator=( const CustomHouseDesign& design );
  void Add( CUSTOM_HOUSE_ELEMENT& elem );
  void AddOrReplace( CUSTOM_HOUSE_ELEMENT& elem );
  void AddMultiAtOffset( u16 multiid, const Core::Vec3d& off );

  bool Erase( const Core::Pos3d& offset, int minheight = 0 );
  bool EraseGraphicAt( u16 graphic, const Core::Pos3d& off );

  void ReplaceDirtFloor( const Core::Pos2d& pos );
  void Clear();
  bool IsEmpty() const;

  unsigned char* Compress( int floor, u32* uncompr_length, u32* compr_length );

  unsigned int TotalSize() const;
  unsigned char NumUsedPlanes() const;

  void readProperties( Clib::ConfigElem& elem, const std::string& prefix );
  void printProperties( Clib::StreamWriter& sw, const std::string& prefix ) const;

  int floor_sizes[CUSTOM_HOUSE_NUM_PLANES];

  bool DeleteStairs( u16 id, Core::Pos3d pos );

  // assumes x,y already added with xoff and yoff
  inline bool ValidLocation( const Core::Pos2d& p )
  {
    return !( p.x() >= _size.x() || p.y() >= _size.y() );
  }
  static bool IsStair( u16 id, int& dir );
  static bool IsStairBlock( u16 id );

  Core::Pos2d _size;    // total sizes including front steps
  Core::Vec2d _offset;  // offsets from center west-most and north-most indicies are 0
  CustomHouseElements Elements[CUSTOM_HOUSE_NUM_PLANES];  // 5 planes

  static const char custom_house_z_xlate_table[CUSTOM_HOUSE_NUM_PLANES];
  // for testing
  void testprint( std::ostream& os ) const;

  void AddComponents( UHouse* house );
  void FillComponents( UHouse* house, bool add_as_component = true );
  void ClearComponents( UHouse* house );
  Bscript::ObjArray* list_parts() const;

private:
  bool isEditableItem( UHouse* house, Items::Item* item );
  static char z_to_custom_house_table( char z );
};

// House Tool Command Implementations:
void CustomHousesAdd( Core::PKTBI_D7* msg );
void CustomHousesAddMulti( Core::PKTBI_D7* msg );
void CustomHousesErase( Core::PKTBI_D7* msg );
void CustomHousesClear( Core::PKTBI_D7* msg );
void CustomHousesQuit( Core::PKTBI_D7* msg );
void CustomHousesCommit( Core::PKTBI_D7* msg );
void CustomHousesSelectFloor( Core::PKTBI_D7* msg );
void CustomHousesBackup( Core::PKTBI_D7* msg );
void CustomHousesRestore( Core::PKTBI_D7* msg );
void CustomHousesSynch( Core::PKTBI_D7* msg );
void CustomHousesRevert( Core::PKTBI_D7* msg );
void CustomHousesRoofSelect( Core::PKTBI_D7* msg );
void CustomHousesRoofRemove( Core::PKTBI_D7* msg );
void CustomHousesSendFull( UHouse* house, Network::Client* client,
                           int design = HOUSE_DESIGN_CURRENT );
void CustomHousesSendFullToInRange( UHouse* house, int design, int range );
void CustomHousesSendShort( UHouse* house, Network::Client* client );
void CustomHouseStopEditing( Mobile::Character* chr, UHouse* house, bool send_pkts = true );
}  // namespace Multi
}  // namespace Pol
#endif
