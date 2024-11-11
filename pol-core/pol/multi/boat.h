/** @file
 *
 * @par History
 * - 2005/04/02 Shinigami: move_offline_mobiles - added realm param
 * - 2009/09/03 MuadDib:   Changes for account related source file relocation
 *                         Changes for multi related source file relocation
 */


#ifndef BOAT_H
#define BOAT_H

#include <climits>
#include <optional>
#include <stddef.h>
#include <string>
#include <vector>

#include "base/position.h"
#include "bscript/bobject.h"
#include "clib/rawtypes.h"
#include "multi.h"
#include "plib/poltype.h"
#include "plib/uconst.h"
#include "reftypes.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Core
{
class UObject;
class UOExecutor;
class ExportScript;
}  // namespace Core
namespace Items
{
class Item;
class ItemDesc;
}  // namespace Items
namespace Mobile
{
class Character;
}  // namespace Mobile
namespace Network
{
class Client;
}  // namespace Network
namespace Realms
{
class Realm;
}
namespace Module
{
class EUBoatRefObjImp;
}
}  // namespace Pol

namespace Pol
{
namespace Multi
{
class MultiDef;

struct BoatShape
{
  struct ComponentShape
  {
    unsigned int objtype;
    unsigned short graphic;
    unsigned short altgraphic;
    Core::Vec3d delta;
    ComponentShape( const std::string& str, const std::string& altstr, unsigned char type );
    ComponentShape( const std::string& str, unsigned char type );
  };
  std::vector<ComponentShape> Componentshapes;

  static bool objtype_is_component( unsigned int objtype );
  BoatShape( Clib::ConfigElem& elem );
  BoatShape() = default;
  size_t estimateSize() const;
};


class UBoat final : public UMulti
{
  typedef UMulti base;

  class BoatContext
  {
    const MultiDef& mdef;
    Core::Pos4d oldpos;

    explicit BoatContext( const UBoat& ub ) : mdef( ub.multidef() ), oldpos( ub.pos() ){};
    friend class UBoat;
    BoatContext& operator=( const BoatContext& ) { return *this; }
  };

public:
  struct BoatMoveGuard
  {
    UBoat* _boat;
    BoatMoveGuard( UBoat* boat ) : _boat( boat )
    {
      if ( boat != nullptr )
        boat->unregself();
    };
    ~BoatMoveGuard()
    {
      if ( _boat != nullptr )
        _boat->regself();
    };
  };

  virtual UBoat* as_boat() override;
  virtual ~UBoat(){};
  virtual size_t estimatedSize() const override;

  bool move( Core::UFACING dir, u8 speed, bool relative );
  bool move_to( const Core::Pos4d& newpos, int flags );

  enum RELATIVE_DIR  // order matters! facing = ( ( dir * 2 ) + facing ) & 7;
  {
    NO_TURN,
    RIGHT,
    AROUND,
    LEFT
  };
  bool turn( RELATIVE_DIR dir );

  virtual void register_object( Core::UObject* obj ) override;
  virtual void unregister_object( Core::UObject* obj ) override;
  Core::UFACING boat_facing() const;

  void send_display_boat( Network::Client* client );
  void send_display_boat_to_inrange( const std::optional<Core::Pos4d>& oldpos );
  void send_boat( Network::Client* client );
  void send_boat_old( Network::Client* client );
  void send_boat_newly_inrange( Network::Client* client );
  void send_remove_boat( Network::Client* client );

  void cleanup_deck();
  bool hold_empty() const;
  bool deck_empty() const;
  bool has_offline_mobiles() const;
  void remove_orphans();
  void destroy_components();
  void regself();
  void unregself();

  static Bscript::BObjectImp* scripted_create( const Items::ItemDesc& descriptor,
                                               const Core::Pos4d& pos, int flags );

  virtual Bscript::BObjectImp* make_ref() override;
  virtual bool get_method_hook( const char* methodname, Bscript::Executor* ex,
                                Core::ExportScript** hook, unsigned int* PC ) const override;
  static bool navigable( const MultiDef& md, const Core::Pos4d& desired_pos );
  static bool can_fit_at_location( const MultiDef& md, const Core::Pos4d& desired_pos );
  static bool objtype_passable( unsigned short graphic );

  virtual void on_color_changed() override;

  Mobile::Character* pilot() const;
  Bscript::BObjectImp* set_pilot( Mobile::Character* chr );
  void clear_pilot();

  // Should these be arrays instead to support customizable boats with certain component choises ?
  Items::Item* tillerman;
  Items::Item* portplank;
  Items::Item* starboardplank;
  Items::Item* hold;

  virtual void readProperties( Clib::ConfigElem& elem ) override;
  virtual void printProperties( Clib::StreamWriter& sw ) const override;

protected:
  Core::ItemRef mountpiece;
  void move_travellers( const BoatContext& oldlocation );
  void turn_travellers( RELATIVE_DIR dir, const BoatContext& oldlocation );
  static bool on_ship( const BoatContext& bc, const Core::UObject* obj );
  void move_offline_mobiles( const Core::Pos4d& newpos );
  const MultiDef& multi_ifturn( RELATIVE_DIR dir );
  unsigned short multiid_ifturn( RELATIVE_DIR dir );


  void do_tellmoves();
  const BoatShape& boatshape() const;
  void rescan_components();
  void reread_components();
  void transform_components( const BoatShape& old_boatshape );
  void move_components();

  explicit UBoat( const Items::ItemDesc& descriptor );
  virtual void fixInvalidGraphic() override;
  friend class UMulti;

  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  virtual Bscript::BObjectImp* script_method( const char* methodname,
                                              Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* script_method_id( const int id, Core::UOExecutor& ex ) override;
  virtual bool script_isa( unsigned isatype ) const override;
  Bscript::BObjectImp* items_list() const;
  Bscript::BObjectImp* mobiles_list() const;
  Bscript::BObjectImp* component_list( unsigned char type ) const;

  friend class Module::EUBoatRefObjImp;
  friend struct BoatMoveGuard;

private:
  void send_smooth_move( Network::Client* client, Core::UFACING move_dir, u8 speed,
                         bool relative ) const;

  Core::Pos4d turn_coords( const Core::Pos4d& oldpos, RELATIVE_DIR dir ) const;
  u8 turn_facing( u8 oldfacing, RELATIVE_DIR dir ) const;
  void create_components();
  void move_boat_item( Items::Item* item, const Core::Pos4d& newpos );
  void move_boat_mobile( Mobile::Character* chr, const Core::Pos4d& newpos );
  typedef Core::UObjectRef Traveller;
  typedef std::vector<Traveller> Travellers;
  Travellers travellers_;

  // Components are not removed from the list when destroyed by the core,
  // so you should always check if the component is an orphan.
  typedef Core::ItemRef Component;
  std::vector<Component> Components;
};

enum BOAT_COMPONENT
{
  COMPONENT_TILLERMAN = 0,
  COMPONENT_PORT_PLANK = 1,
  COMPONENT_STARBOARD_PLANK = 2,
  COMPONENT_HOLD = 3,
  COMPONENT_ROPE = 4,
  COMPONENT_WHEEL = 5,
  COMPONENT_HULL = 6,
  COMPONENT_TILLER = 7,
  COMPONENT_RUDDER = 8,
  COMPONENT_SAILS = 9,
  COMPONENT_STORAGE = 10,
  COMPONENT_WEAPONSLOT = 11,
  COMPONENT_ALL = 12
};

Bscript::BObjectImp* destroy_boat( UBoat* boat );
unsigned int get_component_objtype( unsigned char type );
bool BoatShapeExists( u16 multiid );
void clean_boatshapes();
}  // namespace Multi
}  // namespace Pol
#endif
