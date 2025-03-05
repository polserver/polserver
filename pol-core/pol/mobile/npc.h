/** @file
 *
 * @par History
 * - 2006/09/17 Shinigami: send_event() will return error "Event queue is full, discarding event"
 * - 2009/09/22 MuadDib:   Rewrite for Character/NPC to use ar(), ar_mod(), ar_mod(newvalue)
 * virtuals.
 * - 2009/11/16 Turley:    added NpcPropagateEnteredArea()/inform_enteredarea() for event on
 * resurrection
 * - 2010/02/03 Turley:    MethodScript support for mobiles
 */


#ifndef H_NPC_H
#define H_NPC_H

#ifndef __CHARACTR_H
#include "charactr.h"
#endif

#ifndef ARMOR_H
#include "../item/armor.h"
#endif

#include <iosfwd>
#include <map>
#include <memory>
#include <stddef.h>
#include <string>

#include "../../bscript/bobject.h"
#include "../../clib/boostutils.h"
#include "../../clib/rawtypes.h"
#include "../../plib/uconst.h"
#include "../base/position.h"
#include "../dynproperties.h"
#include "../eventid.h"
#include "../item/weapon.h"
#include "../npctmpl.h"
#include "../reftypes.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
namespace Clib
{
class StreamWriter;
}  // namespace Clib
namespace Items
{
class Item;
}  // namespace Items
}  // namespace Pol
template <class T>
class ref_ptr;

namespace Pol
{
namespace Clib
{
class ConfigElem;
}
namespace Module
{
class NPCExecutorModule;
}
namespace Core
{
class RepSystem;
class UOExecutor;
class ExportScript;
}  // namespace Core
namespace Mobile
{
struct Anchor
{
  Anchor() : enabled( false ), pos( 0, 0 ), dstart( 0 ), psub( 0 ){};
  bool enabled;
  Core::Pos2d pos;
  unsigned short dstart;
  unsigned short psub;
};

class NPC final : public Character
{
  typedef Character base;

public:
  explicit NPC( u32 objtype, const Clib::ConfigElem& elem );
  virtual ~NPC();

private:  // not implemented
  NPC( const NPC& npc );
  NPC& operator=( const NPC& npc );

  // UOBJECT INTERFACE
public:
  virtual size_t estimatedSize() const override;

  virtual void destroy() override;

  virtual void readProperties( Clib::ConfigElem& elem ) override;

  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const override;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const override;  /// id test
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value ) override;
  virtual Bscript::BObjectImp* set_script_member( const char* membername,
                                                  const std::string& ) override;
  virtual Bscript::BObjectImp* set_script_member_id( const int id,
                                                     int value ) override;  // id test
  virtual Bscript::BObjectImp* set_script_member_id( const int id,
                                                     const std::string& ) override;  // id test
  virtual Bscript::BObjectImp* script_method( const char* methodname,
                                              Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* script_method_id( const int id, Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* custom_script_method( const char* methodname,
                                                     Core::UOExecutor& ex ) override;
  virtual bool script_isa( unsigned isatype ) const override;
  virtual bool get_method_hook( const char* methodname, Bscript::Executor* executor,
                                Core::ExportScript** hook, unsigned int* PC ) const override;

protected:
  virtual const char* classname() const override;
  virtual void printOn( Clib::StreamWriter& sw ) const override;
  virtual void printSelfOn( Clib::StreamWriter& sw ) const override;
  virtual void printProperties( Clib::StreamWriter& sw ) const override;
  virtual void printDebugProperties( Clib::StreamWriter& sw ) const override;

  // NPC INTERFACE
public:
  virtual Items::UWeapon* intrinsic_weapon() override;

  virtual void inform_disengaged( const Attackable& disengaged ) override;
  virtual void inform_engaged( const Attackable& engaged ) override;
  virtual void inform_criminal( Character* thecriminal ) override;
  virtual void inform_leftarea( Character* wholeft ) override;
  virtual void inform_enteredarea( Character* whoentered ) override;
  virtual void inform_moved( Character* moved ) override;
  virtual void inform_imoved( Character* chr ) override;
  virtual double armor_absorb_damage( double damage ) override;
  virtual void get_hitscript_params( double damage, Items::UArmor** parmor,
                                     unsigned short* rawdamage ) override;
  virtual unsigned short ar() const override;
  virtual void refresh_ar() override;

  virtual void apply_raw_damage_hundredths( unsigned int damage, Character* source,
                                            bool userepsys = true,
                                            bool send_damage_packet = false ) override;
  virtual void on_death( Items::Item* corpse ) override;

  virtual bool can_be_renamed_by( const Character* chr ) const override;

  virtual void repsys_on_attack( Character* defender ) override;
  virtual void repsys_on_damage( Character* defender ) override;
  virtual void repsys_on_help( Character* recipient ) override;
  virtual unsigned char hilite_color_idx( const Character* seen_by ) const override;
  virtual unsigned short name_color( const Character* seen_by ) const override;

protected:
  virtual u16 get_damaged_sound() const override;
  // MOVEMENT
public:
  bool use_adjustments() const;
  void use_adjustments( bool newvalue );
  bool could_move( Core::UFACING dir ) const;
  bool anchor_allows_move( Core::UFACING dir ) const;
  bool npc_path_blocked( Core::UFACING dir ) const;

  // EVENTS
public:
  void on_pc_spoke( Character* src_chr, const std::string& speech, u8 texttype,
                    const std::string& lang = "", Bscript::ObjArray* speechtokens = nullptr );
  void on_ghost_pc_spoke( Character* src_chr, const std::string& speech, u8 texttype,
                          const std::string& lang = "", Bscript::ObjArray* speechtokens = nullptr );
  bool can_accept_event( Core::EVENTID eventid );
  bool can_accept_area_event_by( const Character* who ) const;
  Bscript::BObjectImp* send_event_script( Bscript::BObjectImp* event );
  bool send_event( Bscript::BObjectImp* event );

  // SCRIPT
public:
  void restart_script();
  void start_script();

protected:
  void stop_scripts();

  // SERIALIZATION
public:
  void readPropertiesForNewNPC( Clib::ConfigElem& elem );

private:
  void readNpcProperties( Clib::ConfigElem& elem );
  void loadEquipablePropertiesNPC( Clib::ConfigElem& elem );
  void readNewNpcAttributes( Clib::ConfigElem& elem );

  // ATTRIBUTES / VITALS
private:
  DYN_PROPERTY( orig_fire_resist, s16, Core::PROP_ORIG_RESIST_FIRE, 0 );
  DYN_PROPERTY( orig_cold_resist, s16, Core::PROP_ORIG_RESIST_COLD, 0 );
  DYN_PROPERTY( orig_energy_resist, s16, Core::PROP_ORIG_RESIST_ENERGY, 0 );
  DYN_PROPERTY( orig_poison_resist, s16, Core::PROP_ORIG_RESIST_POISON, 0 );
  DYN_PROPERTY( orig_physical_resist, s16, Core::PROP_ORIG_RESIST_PHYSICAL, 0 );

  DYN_PROPERTY( orig_fire_damage, s16, Core::PROP_ORIG_DAMAGE_FIRE, 0 );
  DYN_PROPERTY( orig_cold_damage, s16, Core::PROP_ORIG_DAMAGE_COLD, 0 );
  DYN_PROPERTY( orig_energy_damage, s16, Core::PROP_ORIG_DAMAGE_ENERGY, 0 );
  DYN_PROPERTY( orig_poison_damage, s16, Core::PROP_ORIG_DAMAGE_POISON, 0 );
  DYN_PROPERTY( orig_physical_damage, s16, Core::PROP_ORIG_DAMAGE_PHYSICAL, 0 );


  DYN_PROPERTY( orig_lower_reagent_cost, s16, Core::PROP_ORIG_LOWER_REAG_COST, 0 );
  DYN_PROPERTY( orig_spell_damage_increase, s16, Core::PROP_ORIG_SPELL_DAMAGE_INCREASE, 0 );
  DYN_PROPERTY( orig_faster_casting, s16, Core::PROP_ORIG_FASTER_CASTING, 0 );
  DYN_PROPERTY( orig_faster_cast_recovery, s16, Core::PROP_ORIG_FASTER_CAST_RECOVERY, 0 );
  DYN_PROPERTY( orig_defence_increase, s16, Core::PROP_ORIG_DEFENCE_INCREASE, 0 );
  DYN_PROPERTY( orig_defence_increase_cap, s16, Core::PROP_ORIG_DEFENCE_INCREASE_CAP, 0 );
  DYN_PROPERTY( orig_lower_mana_cost, s16, Core::PROP_ORIG_LOWER_MANA_COST, 0 );
  DYN_PROPERTY( orig_hit_chance, s16, Core::PROP_ORIG_HIT_CHANCE, 0 );
  DYN_PROPERTY( orig_fire_resist_cap, s16, Core::PROP_ORIG_RESIST_FIRE_CAP, 0 );
  DYN_PROPERTY( orig_cold_resist_cap, s16, Core::PROP_ORIG_RESIST_COLD_CAP, 0 );
  DYN_PROPERTY( orig_energy_resist_cap, s16, Core::PROP_ORIG_RESIST_ENERGY_CAP, 0 );
  DYN_PROPERTY( orig_poison_resist_cap, s16, Core::PROP_ORIG_RESIST_POISON_CAP, 0 );
  DYN_PROPERTY( orig_physical_resist_cap, s16, Core::PROP_ORIG_RESIST_PHYSICAL_CAP, 0 );
  DYN_PROPERTY( orig_luck, s16, Core::PROP_ORIG_LUCK, 0 );
  DYN_PROPERTY( orig_swing_speed_increase, s16, Core::PROP_ORIG_SWING_SPEED_INCREASE, 0 );
  DYN_PROPERTY( orig_min_attack_range_increase, s16, Core::PROP_ORIG_MIN_ATTACK_RANGE_INCREASE, 0 );
  DYN_PROPERTY( orig_max_attack_range_increase, s16, Core::PROP_ORIG_MAX_ATTACK_RANGE_INCREASE, 0 );

  void resetEquipablePropertiesNPC();

  // MISC
public:
  Character* master() const;
  Core::NpcTemplate::ALIGNMENT alignment() const;

  bool no_drop_exception() const;
  void no_drop_exception( bool newvalue );

  std::string templatename() const;
  // ==========================================================
  // DATA:
  // ==========================================================
  friend class Core::RepSystem;
  friend class Module::NPCExecutorModule;
  friend class ref_ptr<NPC>;

  // UOBJECT INTERFACE
  // NPC INTERFACE
private:
  unsigned short npc_ar_;
  // MOVEMENT
public:
  unsigned short run_speed;

private:
  Anchor anchor;
  // EVENTS
private:
  DYN_PROPERTY( speech_color, u16, Core::PROP_SPEECH_COLOR, Plib::DEFAULT_TEXT_COLOR );
  DYN_PROPERTY( speech_font, u16, Core::PROP_SPEECH_FONT, Plib::DEFAULT_TEXT_FONT );
  // SCRIPT
private:
  boost_utils::script_name_flystring script;

protected:
  Core::UOExecutor* ex;
  // SERIALIZATION
  // ATTRIBUTES / VITALS
  // MISC
public:
  unsigned short damaged_sound;

protected:
  boost_utils::npctemplate_name_flystring template_name;

private:
  Core::CharacterRef master_;
  std::shared_ptr<Core::NpcTemplate> template_;
};

inline Character* NPC::master() const
{
  return master_.get();
}

inline Core::NpcTemplate::ALIGNMENT NPC::alignment() const
{
  return template_->alignment;
}

inline unsigned short NPC::ar() const
{
  if ( ar_ == 0 )
  {
    if ( shield != nullptr && shield->is_intrinsic() )
      return npc_ar_ + shield->ar();
    return npc_ar_;
  }
  else
    return ar_;
}
}  // namespace Mobile
}  // namespace Pol
#endif
