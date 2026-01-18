/** @file
 *
 * @par History
 * - 2008/12/17 MuadDub:   Added item.tile_layer - returns layer entry from tiledata/tiles.cfg
 * - 2009/08/06 MuadDib:   Added gotten_by for character ref.
 * - 2009/08/06 MuadDib:   Added gotten_by code for items.
 */

#ifndef ITEM_H
#define ITEM_H

#include "pol_global_config.h"

#include <stddef.h>
#include <string>

#include "../../clib/boostutils.h"
#include "../../clib/rawtypes.h"
#include "../baseobject.h"
#include "../dynproperties.h"
#include "../globals/settings.h"
#include "../layers.h"
#include "../uobject.h"


#ifndef BSCRIPT_BOBJECT_H
#include "../../bscript/bobject.h"
#endif

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
}  // namespace Pol

namespace Pol
{
namespace Module
{
class UOExecutorModule;
}
namespace Core
{
class UContainer;
class UOExecutor;

std::string format_description( unsigned int polflags, const std::string& descdef,
                                unsigned short amount, const std::string suffix );
}  // namespace Core
namespace Mobile
{
class Character;
}
namespace Multi
{
class UHouse;
class UMulti;
}  // namespace Multi
namespace Network
{
class Client;
}
namespace Items
{
class ItemDesc;

/**
 * @warning All derived classes should have protected constructors,
 *          and make Item a friend.
 */
class Item : public Core::UObject
{
  using base = Core::UObject;

public:
  bool stackable() const;

  ~Item() override;
  size_t estimatedSize() const override;

  virtual void double_click( Network::Client* client );
  void snoop( Network::Client* client, Mobile::Character* owner );
  virtual void builtin_on_use( Network::Client* client );
  virtual void walk_on( Mobile::Character* chr );

  bool is_visible_to_me( const Mobile::Character* chr ) const;
  virtual void inform_leftarea( Mobile::Character* wholeft );
  virtual void inform_enteredarea( Mobile::Character* whoentered );
  virtual void inform_moved( Mobile::Character* moved );

  const ItemDesc& itemdesc() const;

  virtual u16 get_senditem_amount() const;
  u16 getamount() const;
  void setamount( u16 amount );
  void subamount( u16 amount_subtract );

  bool movable() const;
  void movable( bool newvalue );
  void on_movable_changed();

  bool default_movable() const;
  bool default_invisible() const;

  bool inuse() const;
  void inuse( bool newvalue );

  bool invisible() const;
  void invisible( bool newvalue );
  void on_invisible_changed();

  void set_decay_after( unsigned int seconds );
  bool should_decay( unsigned int gameclock ) const;
  void restart_decay_timer();
  void disable_decay();
  bool can_decay() const;

  bool setlayer( unsigned char layer );
  bool setgraphic( u16 newobjtype ) override;
  bool setcolor( u16 newcolor ) override;
  void on_color_changed() override;
  virtual void spill_contents( Multi::UMulti* supporting_multi );

  void setfacing( u8 newfacing ) override;
  void on_facing_changed() override;

  std::string description() const override;
  std::string merchant_description() const;


  std::string get_use_script_name() const;
  u32 sellprice() const;
  void sellprice( u32 );
  u32 buyprice() const;
  void buyprice( u32 );
  bool getbuyprice( u32& buyprice ) const;

  bool newbie() const;
  bool default_newbie() const;
  void newbie( bool newvalue );

  bool insured() const;
  bool default_insured() const;
  void insured( bool newvalue );
  bool use_insurance();

  bool default_cursed() const;
  bool cursed() const;
  void cursed( bool newvalue );

  bool no_drop() const;
  bool default_no_drop() const;
  void no_drop( bool newvalue );

  u8 slot_index() const;
  bool slot_index( u8 newvalue );
  void reset_slot();

  virtual unsigned int item_count() const;
  unsigned int weight_of( unsigned short amount ) const;  // uses weight_multiplier_mod
  unsigned int weight() const override;

  std::string name() const override;

  UObject* owner() override;
  const UObject* owner() const override;
  UObject* toplevel_owner() override;
  const UObject* toplevel_owner() const override;

  bool can_add_to_self( unsigned short amount, bool force_stacking ) const;
  bool can_add_to_self( const Item& item, bool force_stacking ) const;
  bool has_only_default_cprops( const ItemDesc* compare = nullptr ) const;
  void add_to_self( Item*& item );  // deletes the item passed

  void increv_send_object_recursive();

#ifdef PERGON
  void ct_merge_stacks_pergon(
      Item*& item_sub );  // Pergon: Re-Calculate Property CreateTime after Merging of two Stacks
  void ct_merge_stacks_pergon(
      u16 amount_sub );  // Pergon: Re-Calculate Property CreateTime after Adding Items to a Stack
#endif

  bool amount_to_remove_is_partial( u16 amount_to_remove ) const;

  /**
   * @param this_item_new_amount is the amount that this item will have.
   * @return A new item will be created, whos amount is the remainder. This is the returned item.
   */
  Item* slice_stacked_item( u16 this_item_new_amount );

  Item* remove_part_of_stack( u16 amount_to_remove );

  void set_use_script( const std::string& scriptname );
  void extricate();

  bool has_equip_script() const;
  Bscript::BObjectImp* run_equip_script( Mobile::Character* chr, bool startup );
  Bscript::BObjectImp* run_equip_script( Mobile::Character* equip_on, Mobile::Character* equip_by,
                                         bool startup );
  bool check_equip_script( Mobile::Character* equip_on, Mobile::Character* equip_by, bool startup );
  bool check_equip_script( Mobile::Character* chr, bool startup );
  Bscript::BObjectImp* run_unequip_script( Mobile::Character* unequip_on,
                                           Mobile::Character* unequip_by );
  bool check_unequip_script( Mobile::Character* unequip_by = nullptr );

  bool check_test_scripts( Mobile::Character* on_chr, Mobile::Character* by_chr,
                           const std::string& script_ecl, bool startup );
  bool check_equiptest_scripts( Mobile::Character* equip_on, Mobile::Character* equip_by,
                                bool startup = false );
  bool check_equiptest_scripts( Mobile::Character* chr, bool startup = false );
  bool check_unequiptest_scripts( Mobile::Character* unequip_on, Mobile::Character* unequip_by );
  bool check_unequiptest_scripts( Mobile::Character* unequip_by = nullptr );
  bool start_control_script( const ItemDesc& itemdesc );
  bool start_control_script();
  bool stop_control_script();

  /**
   * Shortcut function to get a pointer to the owner character
   *
   * @author DAVE, 11/17
   */
  Mobile::Character* GetCharacterOwner() const;
  /**
   * I'm using the named constructor idiom for creation,
   * so that the right kind of object gets created
   * and so that object serial numbers get generated.
   * If no serial passed, one will be generated.
   */
  static Item* create( u32 objtype, u32 serial = 0 );
  static Item* create( const ItemDesc& itemdesc, u32 serial = 0 );
  void readProperties( Clib::ConfigElem& elem ) override;
  virtual Item* clone() const;
  // virtual class BObjectImp* script_member( const char *membername );
  Bscript::BObjectImp* make_ref() override;
  Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  Bscript::BObjectImp* set_script_member( const char* membername,
                                          const std::string& value ) override;
  Bscript::BObjectImp* set_script_member( const char* membername, int value ) override;
  Bscript::BObjectImp* set_script_member_id( const int id,
                                             const std::string& value ) override;  /// id test
  Bscript::BObjectImp* set_script_member_id( const int id,
                                             int value ) override;  /// id test
  Bscript::BObjectImp* set_script_member_double( const char* membername, double value ) override;
  Bscript::BObjectImp* set_script_member_id_double( const int id,
                                                    double value ) override;  // id test
  Bscript::BObjectImp* script_method( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* script_method_id( const int id, Core::UOExecutor& ex ) override;

  bool script_isa( unsigned isatype ) const override;
  const char* target_tag() const override;
  const char* classname() const override;
  Bscript::BObjectImp* custom_script_method( const char* methodname,
                                             Core::UOExecutor& ex ) override;
  Bscript::BObject call_custom_method( const char* methodname, Bscript::BObjectImpRefVec& pmore );
  Bscript::BObject call_custom_method( const char* methodname );
  bool get_method_hook( const char* methodname, Bscript::Executor* ex, Core::ExportScript** hook,
                        unsigned int* PC ) const override;

protected:  // only derived classes need the constructor
  void printProperties( Clib::StreamWriter& sw ) const override;
  void printDebugProperties( Clib::StreamWriter& sw ) const override;

  Item( const ItemDesc& itemdesc, Core::UOBJ_CLASS uobj_class );

private:
  double getItemdescQuality() const;

public:
  Core::UContainer* container;

protected:
  Core::UOExecutor* uoexec_control();

  boost_utils::script_name_flystring on_use_script_;
  boost_utils::script_name_flystring equip_script_;
  boost_utils::script_name_flystring unequip_script_;
  boost_utils::script_name_flystring snoop_script_;
  mutable const ItemDesc* _itemdesc;

  unsigned int decayat_gameclock_;
  u16 amount_;
  u8 slot_index_;

public:
  u8 layer;
  u8 tile_layer;
  unsigned short hp_;
  unsigned short maxhp() const;

  DYN_PROPERTY( maxhp_mod, s16, Core::PROP_MAXHP_MOD, 0 );
  DYN_PROPERTY( name_suffix, std::string, Core::PROP_NAME_SUFFIX, "" );
  DYN_PROPERTY_POINTER( gotten_by, Mobile::Character*, Core::PROP_GOTTEN_BY );
  DYN_PROPERTY_POINTER( process, Module::UOExecutorModule*, Core::PROP_PROCESS );
  /** Tells of which house this item is a component, if any */
  DYN_PROPERTY_POINTER( house, Multi::UHouse*, Core::PROP_HOUSE );

  DYN_PROPERTY( quality, double, Core::PROP_QUALITY, getItemdescQuality() );
  DYN_PROPERTY( weight_multiplier_mod, double, Core::PROP_WEIGHT_MULTIPLIER_MOD, 1.0 );

private:
  /// sell and buyprice generated functions only private! (additional logic needed)
  DYN_PROPERTY( sellprice_, u32, Core::PROP_SELLPRICE, SELLPRICE_DEFAULT );
  DYN_PROPERTY( buyprice_, u32, Core::PROP_BUYPRICE, BUYPRICE_DEFAULT );

protected:
  static const u32 SELLPRICE_DEFAULT;  // means use the itemdesc value
  static const u32 BUYPRICE_DEFAULT;
};

inline u16 Item::getamount() const
{
  return amount_;
}

inline bool Item::movable() const
{
  return flags_.get( Core::OBJ_FLAGS::MOVABLE );
}

inline void Item::movable( bool newvalue )
{
  if ( movable() != newvalue )
  {
    flags_.change( Core::OBJ_FLAGS::MOVABLE, newvalue );
    on_movable_changed();
  }
}

inline bool Item::inuse() const
{
  return flags_.get( Core::OBJ_FLAGS::IN_USE );
}

inline void Item::inuse( bool newvalue )
{
  flags_.change( Core::OBJ_FLAGS::IN_USE, newvalue );
}

inline bool Item::cursed() const
{
  return flags_.get( Core::OBJ_FLAGS::CURSED );
}

inline void Item::cursed( bool newvalue )
{
  flags_.change( Core::OBJ_FLAGS::CURSED, newvalue );
}

inline bool Item::invisible() const
{
  return flags_.get( Core::OBJ_FLAGS::INVISIBLE );
}

inline void Item::invisible( bool newvalue )
{
  if ( invisible() != newvalue )
  {
    flags_.change( Core::OBJ_FLAGS::INVISIBLE, newvalue );
    on_invisible_changed();
  }
}

inline bool Item::newbie() const
{
  return flags_.get( Core::OBJ_FLAGS::NEWBIE );
}

inline void Item::newbie( bool newvalue )
{
  flags_.change( Core::OBJ_FLAGS::NEWBIE, newvalue );
}

inline bool Item::insured() const
{
  return flags_.get( Core::OBJ_FLAGS::INSURED );
}

inline void Item::insured( bool newvalue )
{
  flags_.change( Core::OBJ_FLAGS::INSURED, newvalue );
}

inline u8 Item::slot_index() const
{
  return slot_index_;
}

inline void Item::reset_slot()
{
  slot_index_ = 0;
}

inline bool Item::slot_index( u8 newvalue )
{
  if ( Core::settingsManager.ssopt.use_slot_index )
  {
    if ( newvalue < Core::settingsManager.ssopt.default_max_slots )
    {
      slot_index_ = newvalue;
      return true;
    }
    return false;
  }
  return true;
}

inline bool valid_equip_layer( int layer )
{
  return layer >= Core::LAYER_INFO::LOWEST_LAYER && layer <= Core::LAYER_INFO::HIGHEST_LAYER;
}

inline bool valid_equip_layer( const Item* item )
{
  return valid_equip_layer( item->tile_layer );
}
}  // namespace Items
}  // namespace Pol
#endif
