/** @file
 *
 * @par History
 */


#ifndef UMAP_H
#define UMAP_H

#include "../bscript/bobject.h"
#include "../clib/rawtypes.h"
#ifndef ITEM_H
#include "item/item.h"
#endif
#include "base/position.h"
#include "base/range.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Items
{
class ItemDesc;
}  // namespace Items
namespace Network
{
class Client;
}  // namespace Network
}  // namespace Pol
}  // namespace Bscript
namespace Items
{
class MapDesc;
}
namespace Core
{
struct PKTBI_56;
class ExportScript;
class UOExecutor;

class Map final : public Items::Item
{
  typedef Items::Item base;

public:
  Pos2d gumpsize;
  bool editable;
  bool plotting;
  typedef std::vector<Pos2d> PinPoints;
  PinPoints pin_points;

  Range2d getrange() const;

  Items::Item* clone() const override;  // dave 12-20
  ~Map() override = default;
  Map( const Map& map ) = delete;
  Map& operator=( const Map& map ) = delete;
  size_t estimatedSize() const override;

protected:
  Map( const Items::MapDesc& mapdesc );
  friend Items::Item* Items::Item::create( const Items::ItemDesc& itemdesc, u32 serial );

  void builtin_on_use( Network::Client* client ) override;
  Bscript::BObjectImp* script_method( const char* methodname, UOExecutor& ex ) override;
  Bscript::BObjectImp* script_method_id( const int id, UOExecutor& ex ) override;
  Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  // virtual Bscript::BObjectImp* set_script_member( const char *membername, const std::string&
  // value );
  Bscript::BObjectImp* set_script_member( const char* membername, int value ) override;
  Bscript::BObjectImp* set_script_member_id( const int id,
                                             int value ) override;  // id test
  bool get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                        unsigned int* PC ) const override;
  // virtual Bscript::BObjectImp* set_script_member_double( const char *membername, double value );
  bool script_isa( unsigned isatype ) const override;
  void printProperties( Clib::StreamWriter& sw ) const override;
  void printPinPoints( Clib::StreamWriter& sw ) const;
  void readProperties( Clib::ConfigElem& elem ) override;

  friend void handle_map_pin( Network::Client* client, PKTBI_56* msg );

private:
  bool msgCoordsInBounds( PKTBI_56* msg, const Range2d& area ) const;
  Pos2d gumpToWorld( const Pos2d& gump, const Range2d& area ) const;
  Pos2d worldToGump( const Pos2d& world, const Range2d& area ) const;

  // due to script access with single writable attributes a direct store as Range2d is not possible
  u16 xwest;
  u16 xeast;
  u16 ynorth;
  u16 ysouth;
  u16 facetid;
};
}  // namespace Core
}  // namespace Pol
#endif
