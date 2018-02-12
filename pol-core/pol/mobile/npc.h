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
#include <stddef.h>
#include <string>

#include "../../bscript/bobject.h"
#include "../../clib/boostutils.h"
#include "../../clib/compilerspecifics.h"
#include "../../clib/rawtypes.h"
#include "../dynproperties.h"
#include "../eventid.h"
#include "../item/weapon.h"
#include "../npctmpl.h"
#include "../reftypes.h"
#include "../uconst.h"

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
}
namespace Mobile
{
struct Anchor
{
  Anchor() : enabled( false ), x( 0 ), y( 0 ), dstart( 0 ), psub( 0 ){};
  bool enabled;
  unsigned short x;
  unsigned short y;
  unsigned short dstart;
  unsigned short psub;
};

class NPC : public Character
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
  virtual size_t estimatedSize() const POL_OVERRIDE;

  virtual void destroy() POL_OVERRIDE;

  virtual void readProperties( Clib::ConfigElem& elem ) POL_OVERRIDE;

  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const POL_OVERRIDE;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const POL_OVERRIDE;  /// id test
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member( const char* membername,
                                                  const std::string& ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* set_script_member_id( const int id,
                                                     int value ) POL_OVERRIDE;  // id test
  virtual Bscript::BObjectImp* set_script_member_id( const int id,
                                                     const std::string& ) POL_OVERRIDE;  // id test
  virtual Bscript::BObjectImp* script_method( const char* methodname,
                                              Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* script_method_id( const int id, Bscript::Executor& ex ) POL_OVERRIDE;
  virtual Bscript::BObjectImp* custom_script_method( const char* methodname,
                                                     Bscript::Executor& ex ) POL_OVERRIDE;
  virtual bool script_isa( unsigned isatype ) const POL_OVERRIDE;

protected:
  virtual const char* classname() const POL_OVERRIDE;
  virtual void printOn( Clib::StreamWriter& sw ) const POL_OVERRIDE;
  virtual void printSelfOn( Clib::StreamWriter& sw ) const POL_OVERRIDE;
  virtual void printProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;
  virtual void printDebugProperties( Clib::StreamWriter& sw ) const POL_OVERRIDE;

  // NPC INTERFACE
public:
  virtual Items::UWeapon* intrinsic_weapon() POL_OVERRIDE;

  virtual void inform_disengaged( Character* disengaged ) POL_OVERRIDE;
  virtual void inform_engaged( Character* engaged ) POL_OVERRIDE;
  virtual void inform_criminal( Character* thecriminal ) POL_OVERRIDE;
  virtual void inform_leftarea( Character* wholeft ) POL_OVERRIDE;
  virtual void inform_enteredarea( Character* whoentered ) POL_OVERRIDE;
  virtual void inform_moved( Character* moved ) POL_OVERRIDE;
  virtual void inform_imoved( Character* chr ) POL_OVERRIDE;
  virtual double armor_absorb_damage( double damage ) POL_OVERRIDE;
  virtual void get_hitscript_params( double damage, Items::UArmor** parmor,
                                     unsigned short* rawdamage ) POL_OVERRIDE;
  virtual unsigned short ar() const POL_OVERRIDE;
  virtual void refresh_ar() POL_OVERRIDE;

  virtual void apply_raw_damage_hundredths( unsigned int damage, Character* source,
                                            bool userepsys = true,
                                            bool send_damage_packet = false ) POL_OVERRIDE;
  virtual void on_death( Items::Item* corpse ) POL_OVERRIDE;

  virtual bool can_be_renamed_by( const Character* chr ) const POL_OVERRIDE;

  virtual void repsys_on_attack( Character* defender ) POL_OVERRIDE;
  virtual void repsys_on_damage( Character* defender ) POL_OVERRIDE;
  virtual void repsys_on_help( Character* recipient ) POL_OVERRIDE;
  virtual unsigned char hilite_color_idx( const Character* seen_by ) const POL_OVERRIDE;
  virtual unsigned short name_color( const Character* seen_by ) const POL_OVERRIDE;

protected:
  virtual u16 get_damaged_sound() const POL_OVERRIDE;
  // MOVEMENT
public:
  bool use_adjustments() const;
  void use_adjustments( bool newvalue );
  bool could_move( Core::UFACING dir ) const;
  bool anchor_allows_move( Core::UFACING dir ) const;
  bool npc_path_blocked( Core::UFACING dir ) const;

  // EVENTS
public:
  // Speech: ASCII versions
  void on_pc_spoke( Character* src_chr, const char* speech, u8 texttype );
  void on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype );
  // Speech: Unicode (and ASCII) versions
  void on_pc_spoke( Character* src_chr, const char* speech, u8 texttype, const u16* wspeech,
                    const char lang[4], Bscript::ObjArray* speechtokens = NULL );
  void on_ghost_pc_spoke( Character* src_chr, const char* speech, u8 texttype, const u16* wspeech,
                          const char lang[4], Bscript::ObjArray* speechtokens = NULL );
  bool can_accept_event( Core::EVENTID eventid );
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
  DYN_PROPERTY( speech_color, u16, Core::PROP_SPEECH_COLOR, Core::DEFAULT_TEXT_COLOR );
  DYN_PROPERTY( speech_font, u16, Core::PROP_SPEECH_FONT, Core::DEFAULT_TEXT_FONT );
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
  const Core::NpcTemplate& template_;
};

inline Character* NPC::master() const
{
  return master_.get();
}

inline Core::NpcTemplate::ALIGNMENT NPC::alignment() const
{
  return template_.alignment;
}

inline unsigned short NPC::ar() const
{
  if ( ar_ == 0 )
  {
    if ( shield != NULL && shield->is_intrinsic() )
      return npc_ar_ + shield->ar();
    return npc_ar_;
  }
  else
    return ar_;
}
}
}
#endif
