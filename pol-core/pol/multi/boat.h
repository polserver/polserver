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
#include <stddef.h>
#include <string>
#include <vector>

#include "../../bscript/bobject.h"
#include "../../clib/rawtypes.h"
#include "../../plib/poltype.h"
#include "../../plib/uconst.h"
#include "../reftypes.h"
#include "../vector.h"
#include "multi.h"

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
    unsigned short xdelta;
    unsigned short ydelta;
    signed short zdelta;
    ComponentShape( const std::string& str, const std::string& altstr, unsigned char type );
    ComponentShape( const std::string& str, unsigned char type );
  };
  std::vector<ComponentShape> Componentshapes;

  static bool objtype_is_component( unsigned int objtype );
  BoatShape( Clib::ConfigElem& elem );
  BoatShape();
  size_t estimateSize() const;
};


class UBoat final : public UMulti
{
  typedef UMulti base;

  class BoatContext
  {
    const MultiDef& mdef;
    Core::Pos2d xy;

    explicit BoatContext( const UBoat& ub ) : mdef( ub.multidef() ), xy( ub.pos().getPos2d() ){};
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

  bool move( Plib::UFACING dir, u8 speed, bool relative );
  bool move_xy( unsigned short x, unsigned short y, int flags, Realms::Realm* oldrealm );

  enum RELATIVE_DIR
  {
    NO_TURN,
    RIGHT,
    AROUND,
    LEFT
  };
  bool turn( RELATIVE_DIR dir );

  virtual void register_object( Core::UObject* obj ) override;
  virtual void unregister_object( Core::UObject* obj ) override;
  Plib::UFACING boat_facing() const;

  void send_smooth_move( Network::Client* client, Plib::UFACING move_dir, u8 speed, u16 newx,
                         u16 newy, bool relative );
  void send_smooth_move_to_inrange( Plib::UFACING move_dir, u8 speed, u16 newx, u16 newy,
                                    bool relative );
  void send_display_boat( Network::Client* client );
  void send_display_boat_to_inrange( u16 oldx = USHRT_MAX, u16 oldy = USHRT_MAX );
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

  static Bscript::BObjectImp* scripted_create( const Items::ItemDesc& descriptor, u16 x, u16 y,
                                               s8 z, Realms::Realm* realm, int flags );

  virtual Bscript::BObjectImp* make_ref() override;
  virtual bool get_method_hook( const char* methodname, Bscript::Executor* ex,
                                Core::ExportScript** hook, unsigned int* PC ) const override;
  static bool navigable( const MultiDef&, unsigned short x, unsigned short y, short z,
                         Realms::Realm* realm );
  void realm_changed();
  void adjust_traveller_z( s8 delta_z );

  virtual void on_color_changed() override;


  // Should these be arrays instead to support customizable boats with certain component choises ?
  Items::Item* tillerman;
  Items::Item* portplank;
  Items::Item* starboardplank;
  Items::Item* hold;

protected:
  void move_travellers( enum Plib::UFACING move_dir, const BoatContext& oldlocation,
                        unsigned short x = USHRT_MAX, unsigned short y = USHRT_MAX,
                        Realms::Realm* oldrealm = nullptr );
  void turn_travellers( RELATIVE_DIR dir, const BoatContext& oldlocation );
  void turn_traveller_coords( Mobile::Character* chr, RELATIVE_DIR dir );
  static bool on_ship( const BoatContext& bc, const Core::UObject* obj );
  void move_offline_mobiles( Core::xcoord new_x, Core::ycoord new_y, Core::zcoord new_z,
                             Realms::Realm* new_realm );
  const MultiDef& multi_ifturn( RELATIVE_DIR dir );
  unsigned short multiid_ifturn( RELATIVE_DIR dir );


  void do_tellmoves();
  const BoatShape& boatshape() const;
  void rescan_components();
  void reread_components();
  void transform_components( const BoatShape& old_boatshape, Realms::Realm* oldrealm );
  void move_components( Realms::Realm* oldrealm );

  explicit UBoat( const Items::ItemDesc& descriptor );
  virtual void fixInvalidGraphic() override;
  virtual void readProperties( Clib::ConfigElem& elem ) override;
  virtual void printProperties( Clib::StreamWriter& sw ) const override;
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
  void create_components();
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
