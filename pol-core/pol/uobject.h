/** @file
 *
 * @par History
 * - 2009/02/01 MuadDib:   Resistance storage added.
 *
 * @note ATTENTION
 * This header is part of the PCH
 * Remove the include in all StdAfx.h files or live with the consequences :)
 */


#ifndef __UOBJECT_H
#define __UOBJECT_H

#ifndef __CLIB_RAWTYPES_H
#include "../clib/rawtypes.h"
#endif

#include <atomic>
#include <boost/any.hpp>
#include <boost/flyweight.hpp>
#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <type_traits>

#include "../clib/boostutils.h"
#include "../clib/refptr.h"
#include "baseobject.h"
#include "dynproperties.h"
#include "proplist.h"

#define pf_endl '\n'


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
}
namespace Items
{
class Item;
class UArmor;
class UWeapon;
}  // namespace Items
namespace Mobile
{
class Character;
class NPC;
}  // namespace Mobile
namespace Multi
{
class UMulti;
class UBoat;
}  // namespace Multi
namespace Core
{
class UContainer;
class WornItemsContainer;
class ExportScript;
class UOExecutor;

#pragma pack( push, 1 )
struct Resistances
{
  s16 fire;
  s16 cold;
  s16 poison;
  s16 energy;
  s16 physical;
};

enum ElementalType
{
  ELEMENTAL_TYPE_MAX = 0x4,
  ELEMENTAL_FIRE = 0x0,
  ELEMENTAL_COLD = 0x1,
  ELEMENTAL_ENERGY = 0x2,
  ELEMENTAL_POISON = 0x3,
  ELEMENTAL_PHYSICAL = 0x4
};

struct ElementDamages
{
  s16 fire;
  s16 cold;
  s16 poison;
  s16 energy;
  s16 physical;
};

#pragma pack( pop )

template <typename ENUM,
          typename std::enable_if<
              std::is_enum<ENUM>::value && !std::is_convertible<ENUM, int>::value, int>::type = 0>
struct AttributeFlags
{
  typedef typename std::underlying_type<ENUM>::type enum_t;
  AttributeFlags() : flags_( 0 ){};

  bool get( ENUM flag ) const
  {
    // no implicit conversion to bool, to be able to check against all bits set
    return ( flags_ & static_cast<enum_t>( flag ) ) == static_cast<enum_t>( flag );
  };
  void set( ENUM flag ) { flags_ |= static_cast<enum_t>( flag ); };
  void remove( ENUM flag ) { flags_ &= ~static_cast<enum_t>( flag ); };
  void change( ENUM flag, bool value )
  {
    if ( value )
      set( flag );
    else
      remove( flag );
  }
  void reset() { flags_ = 0; };

private:
  enum_t flags_;
};

enum class OBJ_FLAGS : u16
{
  DIRTY = 1 << 0,  // UObject flags
  SAVE_ON_EXIT = 1 << 1,
  NEWBIE = 1 << 2,  // Item flags
  INSURED = 1 << 3,
  MOVABLE = 1 << 4,
  IN_USE = 1 << 5,
  INVISIBLE = 1 << 6,
  LOCKED = 1 << 7,              // ULockable flag
  CONTENT_TO_GRAVE = 1 << 8,    // UCorpse flag
  NO_DROP = 1 << 9,             // Item flag
  NO_DROP_EXCEPTION = 1 << 10,  // Container/Character flag
  CURSED = 1 << 11,             // Cursed
};

/**
 * @warning if you add fields, be sure to update Items::create().
 */
class UObject : protected ref_counted, public ULWObject, public DynamicPropsHolder
{
public:
  virtual std::string name() const;
  virtual std::string description() const;

  bool specific_name() const;
  void setname( const std::string& );

  bool getprop( const std::string& propname, std::string& propvalue ) const;
  void setprop( const std::string& propname, const std::string& propvalue );
  void eraseprop( const std::string& propname );
  void copyprops( const UObject& obj );
  void copyprops( const PropertyList& proplist );
  void getpropnames( std::vector<std::string>& propnames ) const;
  const PropertyList& getprops() const;

  virtual void destroy();

  virtual unsigned int weight() const = 0;

  virtual UObject* toplevel_owner();  // this isn't really right, it returns the WornItemsContainer
  virtual UObject* owner();
  virtual const UObject* owner() const;
  virtual UObject* self_as_owner();
  virtual const UObject* self_as_owner() const;
  virtual const UObject* toplevel_owner() const;
  const Pos4d& toplevel_pos() const;

  virtual u8 visible_size() const;
  bool in_range( const UObject* other, u16 range ) const;
  bool in_range( const Pos4d& other, u16 range ) const;
  bool in_range( const Pos2d& other, u16 range ) const;
  u16 distance_to( const Pos2d& other ) const;
  u16 distance_to( const Pos4d& other ) const;

  void setposition( Pos4d newpos );

  UFACING direction_toward( UObject* other ) const;
  UFACING direction_toward( const Pos2d& other ) const;
  UFACING direction_away( UObject* other ) const;
  UFACING direction_away( const Pos2d& other ) const;

  virtual void setfacing( u8 newfacing ) = 0;
  virtual void on_facing_changed();

  virtual bool setgraphic( u16 newobjtype );
  virtual bool setcolor( u16 newcolor );
  virtual void on_color_changed();

  bool saveonexit() const;
  void saveonexit( bool newvalue );

  virtual void printOn( Clib::StreamWriter& ) const;
  virtual void printSelfOn( Clib::StreamWriter& sw ) const;

  virtual void printOnDebug( Clib::StreamWriter& sw ) const;
  virtual void fixInvalidGraphic();
  virtual void readProperties( Clib::ConfigElem& elem );
  // virtual Bscript::BObjectImp* script_member( const char *membername );
  virtual Bscript::BObjectImp* make_ref() = 0;
  virtual Bscript::BObjectImp* get_script_member( const char* membername ) const;
  virtual Bscript::BObjectImp* get_script_member_id( const int id ) const;  /// id test

  virtual Bscript::BObjectImp* set_script_member( const char* membername,
                                                  const std::string& value );
  virtual Bscript::BObjectImp* set_script_member( const char* membername, int value );
  virtual Bscript::BObjectImp* set_script_member_double( const char* membername, double value );

  virtual Bscript::BObjectImp* set_script_member_id( const int id, const std::string& value );
  virtual Bscript::BObjectImp* set_script_member_id( const int id, int value );
  virtual Bscript::BObjectImp* set_script_member_id_double( const int id, double value );

  virtual Bscript::BObjectImp* script_method( const char* methodname, UOExecutor& ex );
  virtual Bscript::BObjectImp* script_method_id( const int id, UOExecutor& ex );

  virtual Bscript::BObjectImp* custom_script_method( const char* methodname, UOExecutor& ex );
  virtual bool script_isa( unsigned isatype ) const;
  virtual const char* target_tag() const;

  virtual const char* classname() const = 0;

  virtual size_t estimatedSize() const;
  virtual bool get_method_hook( const char* methodname, Bscript::Executor* ex, ExportScript** hook,
                                unsigned int* PC ) const;

  unsigned ref_counted_count() const;
  inline void increv() { _rev++; };
  inline u32 rev() const { return _rev; };
  bool dirty() const;
  void set_dirty();
  void clear_dirty() const;
  static std::atomic<unsigned int> dirty_writes;
  static std::atomic<unsigned int> clean_writes;

protected:
  virtual void printProperties( Clib::StreamWriter& sw ) const;
  virtual void printDebugProperties( Clib::StreamWriter& sw ) const;

  UObject( u32 objtype, UOBJ_CLASS uobj_class );
  ~UObject() override;

  friend class ref_ptr<UObject>;
  friend class ref_ptr<Mobile::Character>;
  friend class ref_ptr<Items::Item>;
  friend class ref_ptr<Multi::UBoat>;
  friend class ref_ptr<Multi::UMulti>;
  friend class ref_ptr<Mobile::NPC>;
  friend class ref_ptr<UContainer>;
  friend class ref_ptr<Items::UWeapon>;
  friend class ref_ptr<Items::UArmor>;
  friend class ref_ptr<WornItemsContainer>;

  // DATA:
public:
  u32 serial_ext;

  const u32 objtype_;
  u16 color;

  u8 facing;  // not always used for items.
  // always used for characters

  DYN_PROPERTY( maxhp_mod, s16, PROP_MAXHP_MOD, 0 );
  DYN_PROPERTY( fire_resist, ValueModPack, PROP_RESIST_FIRE, ValueModPack::DEFAULT );
  DYN_PROPERTY( cold_resist, ValueModPack, PROP_RESIST_COLD, ValueModPack::DEFAULT );
  DYN_PROPERTY( energy_resist, ValueModPack, PROP_RESIST_ENERGY, ValueModPack::DEFAULT );
  DYN_PROPERTY( poison_resist, ValueModPack, PROP_RESIST_POISON, ValueModPack::DEFAULT );
  DYN_PROPERTY( physical_resist, ValueModPack, PROP_RESIST_PHYSICAL, ValueModPack::DEFAULT );

  DYN_PROPERTY( fire_damage, ValueModPack, PROP_DAMAGE_FIRE, ValueModPack::DEFAULT );
  DYN_PROPERTY( cold_damage, ValueModPack, PROP_DAMAGE_COLD, ValueModPack::DEFAULT );
  DYN_PROPERTY( energy_damage, ValueModPack, PROP_DAMAGE_ENERGY, ValueModPack::DEFAULT );
  DYN_PROPERTY( poison_damage, ValueModPack, PROP_DAMAGE_POISON, ValueModPack::DEFAULT );
  DYN_PROPERTY( physical_damage, ValueModPack, PROP_DAMAGE_PHYSICAL, ValueModPack::DEFAULT );

  DYN_PROPERTY( fire_resist_cap, ValueModPack, PROP_RESIST_FIRE_CAP, ValueModPack::DEFAULT );
  DYN_PROPERTY( cold_resist_cap, ValueModPack, PROP_RESIST_COLD_CAP, ValueModPack::DEFAULT );
  DYN_PROPERTY( energy_resist_cap, ValueModPack, PROP_RESIST_ENERGY_CAP, ValueModPack::DEFAULT );
  DYN_PROPERTY( poison_resist_cap, ValueModPack, PROP_RESIST_POISON_CAP, ValueModPack::DEFAULT );
  DYN_PROPERTY( physical_resist_cap, ValueModPack, PROP_RESIST_PHYSICAL_CAP,
                ValueModPack::DEFAULT );

  DYN_PROPERTY( lower_reagent_cost, ValueModPack, PROP_LOWER_REAG_COST, ValueModPack::DEFAULT );
  DYN_PROPERTY( spell_damage_increase, ValueModPack, PROP_SPELL_DAMAGE_INCREASE,
                ValueModPack::DEFAULT );
  DYN_PROPERTY( faster_casting, ValueModPack, PROP_FASTER_CASTING, ValueModPack::DEFAULT );
  DYN_PROPERTY( faster_cast_recovery, ValueModPack, PROP_FASTER_CAST_RECOVERY,
                ValueModPack::DEFAULT );

  DYN_PROPERTY( defence_increase, ValueModPack, PROP_DEFENCE_INCREASE, ValueModPack::DEFAULT );
  DYN_PROPERTY( defence_increase_cap, ValueModPack, PROP_DEFENCE_INCREASE_CAP,
                ValueModPack::DEFAULT );
  DYN_PROPERTY( lower_mana_cost, ValueModPack, PROP_LOWER_MANA_COST, ValueModPack::DEFAULT );
  DYN_PROPERTY( hit_chance, ValueModPack, PROP_HIT_CHANCE, ValueModPack::DEFAULT );
  DYN_PROPERTY( luck, ValueModPack, PROP_EXT_STATBAR_LUCK, ValueModPack::DEFAULT );
  DYN_PROPERTY( swing_speed_increase, ValueModPack, PROP_SWING_SPEED_INCREASE,
                ValueModPack::DEFAULT );
  DYN_PROPERTY( min_attack_range_increase, ValueModPack, PROP_MIN_ATTACK_RANGE_INCREASE,
                ValueModPack::DEFAULT );
  DYN_PROPERTY( max_attack_range_increase, ValueModPack, PROP_MAX_ATTACK_RANGE_INCREASE,
                ValueModPack::DEFAULT );


private:
  u32 _rev;

protected:
  boost_utils::object_name_flystring name_;
  // mutable due to dirty flag
  mutable AttributeFlags<OBJ_FLAGS> flags_;

private:
  PropertyList proplist_;

private:  // not implemented:
  UObject( const UObject& );
  UObject& operator=( const UObject& );
};

inline bool UObject::specific_name() const
{
  return !name_.get().empty();
}

inline void UObject::set_dirty()
{
  flags_.set( OBJ_FLAGS::DIRTY );
}

inline unsigned UObject::ref_counted_count() const
{
  return ref_counted::count();
}

inline bool IsCharacter( u32 serial )
{
  return ( ~serial & 0x40000000Lu ) ? true : false;
}

inline bool IsItem( u32 serial )
{
  return ( serial & 0x40000000Lu ) ? true : false;
}


inline const Pos4d& UObject::toplevel_pos() const
{
  return toplevel_owner()->pos();
}
inline bool UObject::in_range( const UObject* other, u16 range ) const
{
  return toplevel_pos().in_range( other->toplevel_pos(), range );
}
inline bool UObject::in_range( const Pos2d& other, u16 range ) const
{
  return toplevel_pos().in_range( other, range );
}
inline bool UObject::in_range( const Pos4d& other, u16 range ) const
{
  return toplevel_pos().in_range( other, range );
}
inline u16 UObject::distance_to( const Pos4d& other ) const
{
  return toplevel_pos().pol_distance( other );
}
inline u16 UObject::distance_to( const Pos2d& other ) const
{
  return toplevel_pos().xy().pol_distance( other );
}
inline u8 UObject::visible_size() const
{
  // TODO Pos: should objects on a multi need to take
  // it into account that multis due to the footprint have a higher updaterange?
  return 0;
}
}  // namespace Core
}  // namespace Pol

#endif
