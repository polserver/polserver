/** @file
 *
 * @par History
 *
 * @par Design decisions (64bit)
 * Since boost::any has a size of 8 a padding of 8 will be introduced which means that even if the
 * prop type is a u8 full 16 bytes will be used
 * boost::variant is as big as the biggest possible value plus a type information member
 * in our case the biggest type to store in a variant is u32 thus only a padding of 4 exists:
 * min/max size is 12 instead of 16
 * an empty vector still uses 24bytes moving the vectors into a pointer saves if unused 16bytes.
 *
 * @par Layout
 * - Uobject
 *   - ptr DynProps
 *     -> 8 bytes per object
 * - DynProps
 *   - bitset (currently 4 bytes (more then 32 types = 8 bytes)
 *   - vector<PropHolder> variant version
 *   - ptr vector<PropHolder> any version
 *     -> N "small" properties:
 *     4+24+12*N+8 = 36+12*N
 *     -> +M "big" properties:
 *     +24+(16+unk)*M = (36+12*N) + (24+(16+unk)*M)
 *     unk is the type size which is stored in boost::any
 *
 * @todo Is it worse it to combine e.g resistances struct?
 * 5 * s16
 * 24+12*5 -> 84
 * Combining means storage as boost::any:
 * 24+(16+5*2) -> 50
 * But only valid if all 5 props are really set, if only 2 props are set variant is smaller->
 * 24+12*2=48
 * Different idea combine per resistance type mod and real value:
 * for variant its still 24+12*5 -> 84
 * any would use 24+(16+5*2)*2 -> 76 (but again only if all props are set)
 * Combining would use the u32 size for a prop in a variant without loss (2*s16)
 * -> 8 props can be stored with less space
 */


#ifndef __POL_DYNPROPS_H
#define __POL_DYNPROPS_H

#include <algorithm>
#include <any>
#include <bitset>
#include <memory>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "gameclck.h"

namespace Pol
{
namespace Core
{
// define to generate 3 methods for get/set/has
#define DYN_PROPERTY( name, type, id, defaultvalue )                                         \
  type name() const                                                                          \
  {                                                                                          \
    type val;                                                                                \
    if ( getmember<type>( id, &val ) )                                                       \
      return val;                                                                            \
    return defaultvalue;                                                                     \
  };                                                                                         \
  void name( const type& val ) { setmember( id, val, static_cast<type>( defaultvalue ) ); }; \
  bool has_##name() const { return hasmember( id ); }
// define to generate 3 methods for get/set/has
#define DYN_PROPERTY_POINTER( name, type, id )            \
  type name() const                                       \
  {                                                       \
    type val;                                             \
    if ( getmember<type>( id, &val ) )                    \
      return val;                                         \
    return nullptr;                                       \
  };                                                      \
  void name( type val ) { setmemberPointer( id, val ); }; \
  bool has_##name() const { return hasmember( id ); }
// enum for the propertys
enum DynPropTypes : u8
{
  PROP_AR_MOD = 0,                       // UArmor, Character
  PROP_MAX_ITEMS_MOD = 1,                // UContainer
  PROP_MAX_SLOTS_MOD = 2,                // UContainer
  PROP_MAX_WEIGHT_MOD = 3,               // UContainer
  PROP_SELLPRICE = 4,                    // Item
  PROP_BUYPRICE = 5,                     // Item
  PROP_MAXHP_MOD = 6,                    // Item
  PROP_NAME_SUFFIX = 7,                  // Item
  PROP_RESIST_FIRE = 8,                  // UObject
  PROP_RESIST_COLD = 9,                  // UObject
  PROP_RESIST_ENERGY = 10,               // UObject
  PROP_RESIST_POISON = 11,               // UObject
  PROP_RESIST_PHYSICAL = 12,             // UObject
  PROP_DAMAGE_FIRE = 13,                 // UObject
  PROP_DAMAGE_COLD = 14,                 // UObject
  PROP_DAMAGE_ENERGY = 15,               // UObject
  PROP_DAMAGE_POISON = 16,               // UObject
  PROP_DAMAGE_PHYSICAL = 17,             // UObject
  PROP_DMG_MOD = 18,                     // UWeapon
  PROP_SPEED_MOD = 19,                   // UWeapon
  PROP_ORIG_RESIST_FIRE = 20,            // Npc
  PROP_ORIG_RESIST_COLD = 21,            // Npc
  PROP_ORIG_RESIST_ENERGY = 22,          // Npc
  PROP_ORIG_RESIST_POISON = 23,          // Npc
  PROP_ORIG_RESIST_PHYSICAL = 24,        // Npc
  PROP_ORIG_DAMAGE_FIRE = 25,            // Npc
  PROP_ORIG_DAMAGE_COLD = 26,            // Npc
  PROP_ORIG_DAMAGE_ENERGY = 27,          // Npc
  PROP_ORIG_DAMAGE_POISON = 28,          // Npc
  PROP_ORIG_DAMAGE_PHYSICAL = 29,        // Npc
  PROP_STATCAP_SKILLCAP = 30,            // Character
  PROP_EXT_STATBAR_LUCK = 31,            // Character
  PROP_EXT_STATBAR_FOLLOWERS = 32,       // Character
  PROP_EXT_STATBAR_TITHING = 33,         // Character
  PROP_MOVEMENTCOST_MOD = 34,            // Character
  PROP_QUALITY = 35,                     // Item (Equipment has fixed member)
  PROP_DOUBLECLICK_WAIT = 36,            // Character
  PROP_DISABLE_SKILLS_UNTIL = 37,        // Character
  PROP_SQUELCHED_UNTIL = 38,             // Character
  PROP_DEAFENED_UNTIL = 39,              // Character
  PROP_LIGHTOVERRIDE_UNTIL = 40,         // Character
  PROP_LIGHTOVERRIDE = 41,               // Character
  PROP_TITLE_PREFIX = 42,                // Character
  PROP_TITLE_SUFFIX = 43,                // Character
  PROP_TITLE_GUILD = 44,                 // Character
  PROP_TITLE_RACE = 45,                  // Character
  PROP_SPEECH_COLOR = 46,                // Npc
  PROP_SPEECH_FONT = 47,                 // Npc
  PROP_CARRY_CAPACITY_MOD = 48,          // Character
  PROP_DELAY_MOD = 49,                   // Character
  PROP_HIT_CHANCE_MOD = 50,              // Character
  PROP_EVASIONCHANCE_MOD = 51,           // Character
  PROP_PARTY = 52,                       // Character
  PROP_PARTY_CANDIDATE = 53,             // Character
  PROP_PARTY_OFFLINE = 54,               // Character
  PROP_GUILD = 55,                       // Character
  PROP_GOTTEN_BY = 56,                   // Item
  PROP_GOTTEN_ITEM = 57,                 // Character
  PROP_PROCESS = 58,                     // Item
  PROP_HOUSE = 59,                       // House
  PROP_LOWER_REAG_COST = 60,             // UObject
  PROP_SPELL_DAMAGE_INCREASE = 61,       // UObject
  PROP_FASTER_CASTING = 62,              // UObject
  PROP_FASTER_CAST_RECOVERY = 63,        // UObject
  PROP_DEFENCE_INCREASE = 64,            // UObject
  PROP_DEFENCE_INCREASE_CAP = 65,        // UObject
  PROP_LOWER_MANA_COST = 66,             // UObject
  PROP_HIT_CHANCE = 67,                  // UObject
  PROP_RESIST_FIRE_CAP = 68,             // UObject
  PROP_RESIST_COLD_CAP = 69,             // UObject
  PROP_RESIST_ENERGY_CAP = 70,           // UObject
  PROP_RESIST_POISON_CAP = 71,           // UObject
  PROP_RESIST_PHYSICAL_CAP = 72,         // UObject
  PROP_DEFENCE_INCREASE_MOD = 73,        // UObject
  PROP_LUCK_MOD = 74,                    // UObject
  PROP_ORIG_LOWER_REAG_COST = 75,        // Npc
  PROP_ORIG_SPELL_DAMAGE_INCREASE = 76,  // Npc
  PROP_ORIG_FASTER_CASTING = 77,         // Npc
  PROP_ORIG_FASTER_CAST_RECOVERY = 78,   // Npc
  PROP_ORIG_DEFENCE_INCREASE = 79,       // Npc
  PROP_ORIG_DEFENCE_INCREASE_CAP = 80,   // Npc
  PROP_ORIG_LOWER_MANA_COST = 81,        // Npc
  PROP_ORIG_HIT_CHANCE = 82,             // Npc
  PROP_ORIG_RESIST_FIRE_CAP = 83,        // Npc
  PROP_ORIG_RESIST_COLD_CAP = 84,        // Npc
  PROP_ORIG_RESIST_ENERGY_CAP = 85,      // Npc
  PROP_ORIG_RESIST_POISON_CAP = 86,      // Npc
  PROP_ORIG_RESIST_PHYSICAL_CAP = 87,    // Npc
  PROP_ORIG_LUCK = 88,                   // Npc
  PROP_SWING_SPEED_INCREASE = 89,        // UObject
  PROP_SWING_SPEED_INCREASE_MOD = 90,    // UObject
  PROP_ORIG_SWING_SPEED_INCREASE = 91,   // Npc
  PROP_PARRYCHANCE_MOD = 92,             // Character
  PROP_WEIGHT_MULTIPLIER_MOD = 93,       // Item
  PROP_HELD_WEIGHT_MULTIPLIER_MOD = 94,  // Container

  PROP_FLAG_SIZE  // used for bitset size
};

// value & mod struct definition for e.g. the resist/damage properties
struct ValueModPack
{
  s16 value;
  s16 mod;
  ValueModPack( s16 value_ );
  ValueModPack();
  bool operator==( const ValueModPack& other ) const;
  bool operator!=( const ValueModPack& other ) const;
  ValueModPack& addToValue( const ValueModPack& other );
  ValueModPack& addToValue( s16 other );
  ValueModPack& removeFromValue( const ValueModPack& other );
  ValueModPack& removeFromValue( s16 other );
  ValueModPack& addToMod( s16 other );
  ValueModPack& setAsMod( s16 other );
  ValueModPack& setAsValue( s16 other );
  ValueModPack& resetModAsValue();
  s16 sum() const;

  static const ValueModPack DEFAULT;
};
static_assert( sizeof( ValueModPack ) == sizeof( u32 ), "size missmatch" );

// combination of skill and stat cap
struct SkillStatCap
{
  s16 statcap;
  u16 skillcap;
  SkillStatCap();
  SkillStatCap( s16 statcap_, u16 skillcap_ );
  bool operator==( const SkillStatCap& other ) const;

  static const SkillStatCap DEFAULT;
};
static_assert( sizeof( SkillStatCap ) == sizeof( u32 ), "size missmatch" );

// combination of followers/followers_max
struct ExtStatBarFollowers
{
  s8 followers;
  s8 followers_max;
  ExtStatBarFollowers();
  ExtStatBarFollowers( s8 followers_, s8 followers_max_ );
  bool operator==( const ExtStatBarFollowers& other ) const;

  static const ExtStatBarFollowers DEFAULT;
};
static_assert( sizeof( ExtStatBarFollowers ) == sizeof( u16 ), "size missmatch" );

// movement cost mod (not in variant storage)
struct MovementCostMod
{
  double walk;
  double run;
  double walk_mounted;
  double run_mounted;
  MovementCostMod();
  MovementCostMod( double walk_, double run_, double walk_mounted_, double run_mounted_ );
  bool operator==( const MovementCostMod& other ) const;

  static const MovementCostMod DEFAULT;
};

template <typename Storage>
class PropHolderContainer;

// small property type no types above size 4, for bigger types boost::any will be used
typedef std::variant<u8, u16, u32, s8, s16, s32, ValueModPack, SkillStatCap, ExtStatBarFollowers>
    variant_storage;
template <typename T>
struct can_be_used_in_variant
{
  static const bool value =
      std::is_same<T, u8>::value || std::is_same<T, u16>::value || std::is_same<T, u32>::value ||
      std::is_same<T, s8>::value || std::is_same<T, s16>::value || std::is_same<T, s32>::value ||
      std::is_same<T, ValueModPack>::value || std::is_same<T, SkillStatCap>::value ||
      std::is_same<T, ExtStatBarFollowers>::value || std::is_same<T, gameclock_t>::value;
};

// holder class
// stores the property kind and via boost::variant/boost::any the value
template <typename Storage>
class PropHolder
{
public:
  template <typename S>
  friend class PropHolderContainer;
  explicit PropHolder( DynPropTypes type );
  PropHolder( DynPropTypes type, const Storage& value );
  template <typename V>
  V getValue() const;

protected:
  DynPropTypes _type;
  Storage _value;
};

// Container class
// simple vector of PropHolder instances, used to have common code between boost::any/variant
template <typename Storage>
class PropHolderContainer
{
public:
  PropHolderContainer();
  template <typename V>
  bool getValue( DynPropTypes type, V* value ) const;
  template <typename V>
  bool updateValue( DynPropTypes type, const V& value );
  template <typename V>
  bool updateValuePointer( DynPropTypes type, V value );
  template <typename V>
  void addValue( DynPropTypes type, const V& value );
  template <typename V>
  void addValuePointer( DynPropTypes type, V value );
  void removeValue( DynPropTypes type );
  size_t estimateSize() const;

private:
  std::vector<PropHolder<Storage>> _props;
};

// management class
// has a bitset for fast checking if a property exists
// simple vector for all the "small" PropHolders
// and a lazy filled unique_ptr for "big" PropHolders
class DynProps
{
public:
  DynProps();
  // fast bitflag check
  bool hasProperty( DynPropTypes type ) const;
  // get property returns false if non existent (checks via hasProperty before)
  template <typename V>
  bool getProperty( DynPropTypes type, V* value ) const;
  // set property (sets also the flag)
  template <typename V>
  void setProperty( DynPropTypes type, const V& value );
  template <typename V>
  void setPropertyPointer( DynPropTypes type, V value );
  // remove a prop
  template <typename V>
  void removeProperty( DynPropTypes type );
  size_t estimateSize() const;

private:
  std::bitset<PROP_FLAG_SIZE> _prop_bits;
  PropHolderContainer<variant_storage> _props;
  std::unique_ptr<PropHolderContainer<std::any>> _any_props;
};

// Base class for dynamic properties, should be derived from
// holds a pointer to the DynProps, which is lazy filled
// due to memory usage (empty DynProps is larger then an unique_ptr)
// TODO: should the ptr be release if the last member was removed?
class DynamicPropsHolder
{
public:
  DynamicPropsHolder();
  bool hasmember( DynPropTypes member ) const;
  template <typename V>
  bool getmember( DynPropTypes member, V* value ) const;
  template <typename V>
  void setmember( DynPropTypes member, const V& value, const V& defaultvalue );
  template <typename V>
  void setmemberPointer( DynPropTypes member, V value );
  size_t estimateSizeDynProps() const;

protected:
  ~DynamicPropsHolder() = default;

private:
  void initProps();
  std::unique_ptr<DynProps> _dynprops;
};


////////////////////////////
// Imp start
////////////////////////////

////////////////
// ValueModPack

inline ValueModPack::ValueModPack( s16 value_ ) : value( value_ ), mod( 0 ) {}
inline ValueModPack::ValueModPack() : value( 0 ), mod( 0 ) {}
inline bool ValueModPack::operator==( const ValueModPack& other ) const
{
  return value == other.value && mod == other.mod;
}
inline bool ValueModPack::operator!=( const ValueModPack& other ) const
{
  return !operator==( other );
}
inline ValueModPack& ValueModPack::addToValue( const ValueModPack& other )
{
  value += other.value + other.mod;
  return *this;
}
inline ValueModPack& ValueModPack::addToValue( s16 other )
{
  value += other;
  return *this;
}
inline ValueModPack& ValueModPack::removeFromValue( const ValueModPack& other )
{
  value -= other.value + other.mod;
  return *this;
}
inline ValueModPack& ValueModPack::removeFromValue( s16 other )
{
  value -= other;
  return *this;
}
inline ValueModPack& ValueModPack::addToMod( s16 other )
{
  mod += other;
  return *this;
}
inline ValueModPack& ValueModPack::setAsMod( s16 other )
{
  mod = other;
  return *this;
}
inline ValueModPack& ValueModPack::setAsValue( s16 other )
{
  value = other;
  return *this;
}
inline ValueModPack& ValueModPack::resetModAsValue()
{
  value = mod;
  return *this;
}
inline s16 ValueModPack::sum() const
{
  return value + mod;
}

////////////////
// SkillStatCap
inline SkillStatCap::SkillStatCap() : statcap( 0 ), skillcap( 0 ) {}
inline SkillStatCap::SkillStatCap( s16 statcap_, u16 skillcap_ )
    : statcap( statcap_ ), skillcap( skillcap_ )
{
}
inline bool SkillStatCap::operator==( const SkillStatCap& other ) const
{
  return statcap == other.statcap && skillcap == other.skillcap;
}

////////////////
// ExtStatBarFollowers
inline ExtStatBarFollowers::ExtStatBarFollowers() : followers( 0 ), followers_max( 0 ) {}
inline ExtStatBarFollowers::ExtStatBarFollowers( s8 followers_, s8 followers_max_ )
    : followers( followers_ ), followers_max( followers_max_ )
{
}
inline bool ExtStatBarFollowers::operator==( const ExtStatBarFollowers& other ) const
{
  return followers == other.followers && followers_max == other.followers_max;
}

////////////////
// MovementCostMod
inline MovementCostMod::MovementCostMod()
    : walk( 1.0 ), run( 1.0 ), walk_mounted( 1.0 ), run_mounted( 1.0 )
{
}
inline MovementCostMod::MovementCostMod( double walk_, double run_, double walk_mounted_,
                                         double run_mounted_ )
    : walk( walk_ ), run( run_ ), walk_mounted( walk_mounted_ ), run_mounted( run_mounted_ )
{
}
inline bool MovementCostMod::operator==( const MovementCostMod& other ) const
{
  return walk == other.walk && run == other.run && walk_mounted == other.walk_mounted &&
         run_mounted == other.run_mounted;
}

////////////////
// PropHolder
template <class Storage>
inline PropHolder<Storage>::PropHolder( DynPropTypes type ) : _type( type ), _value()
{
}
template <class Storage>
inline PropHolder<Storage>::PropHolder( DynPropTypes type, const Storage& value )
    : _type( type ), _value( value )
{
}

template <>
template <typename V>
inline V PropHolder<std::any>::getValue() const
{
  return std::any_cast<V>( _value );
}
template <>
template <typename V>
inline V PropHolder<variant_storage>::getValue() const
{
  return std::get<V>( _value );
}

////////////////
// PropHolderContainer
template <class Storage>
inline PropHolderContainer<Storage>::PropHolderContainer() : _props()
{
}

template <class Storage>
template <typename V>
inline bool PropHolderContainer<Storage>::getValue( DynPropTypes type, V* value ) const
{
  for ( const PropHolder<Storage>& prop : _props )
  {
    if ( prop._type == type )
    {
      *value = prop.template getValue<V>();
      return true;
    }
  }
  return false;
}

template <class Storage>
template <typename V>
inline bool PropHolderContainer<Storage>::updateValue( DynPropTypes type, const V& value )
{
  for ( PropHolder<Storage>& prop : _props )
  {
    if ( prop._type == type )
    {
      prop._value = value;
      return true;
    }
  }
  return false;
}

template <class Storage>
template <typename V>
inline bool PropHolderContainer<Storage>::updateValuePointer( DynPropTypes type, V value )
{
  for ( PropHolder<Storage>& prop : _props )
  {
    if ( prop._type == type )
    {
      prop._value = value;
      return true;
    }
  }
  return false;
}

template <class Storage>
template <typename V>
inline void PropHolderContainer<Storage>::addValue( DynPropTypes type, const V& value )
{
  _props.emplace_back( type, value );
}

template <class Storage>
template <typename V>
inline void PropHolderContainer<Storage>::addValuePointer( DynPropTypes type, V value )
{
  _props.emplace_back( type, value );
}

template <class Storage>
inline void PropHolderContainer<Storage>::removeValue( DynPropTypes type )
{
  _props.erase(
      std::remove_if( _props.begin(), _props.end(),
                      [&type]( const PropHolder<Storage>& x ) { return type == x._type; } ),
      _props.end() );
}

template <class Storage>
inline size_t PropHolderContainer<Storage>::estimateSize() const
{
  return 3 * sizeof( void* ) + _props.capacity() * sizeof( PropHolder<Storage> );
}


////////////////
// helper template functions to switch between both containers
namespace
{
template <typename V>
static typename std::enable_if<can_be_used_in_variant<V>::value, bool>::type getPropertyHelper(
    const PropHolderContainer<variant_storage>& variant_props,
    const std::unique_ptr<PropHolderContainer<std::any>>& any_props, DynPropTypes type, V* value )
{
  (void)any_props;
  return variant_props.getValue( type, value );
}
template <typename V>
static typename std::enable_if<!can_be_used_in_variant<V>::value, bool>::type getPropertyHelper(
    const PropHolderContainer<variant_storage>& variant_props,
    const std::unique_ptr<PropHolderContainer<std::any>>& any_props, DynPropTypes type, V* value )
{
  (void)variant_props;
  passert_always( any_props.get() );
  return any_props->getValue( type, value );
}

template <typename V>
static typename std::enable_if<can_be_used_in_variant<V>::value, bool>::type updatePropertyHelper(
    PropHolderContainer<variant_storage>& variant_props,
    std::unique_ptr<PropHolderContainer<std::any>>& any_props, DynPropTypes type, const V& value )
{
  (void)any_props;
  return variant_props.updateValue( type, value );
}
template <typename V>
static typename std::enable_if<!can_be_used_in_variant<V>::value, bool>::type updatePropertyHelper(
    PropHolderContainer<variant_storage>& variant_props,
    std::unique_ptr<PropHolderContainer<std::any>>& any_props, DynPropTypes type, const V& value )
{
  (void)variant_props;
  passert_always( any_props.get() );
  return any_props->updateValue( type, value );
}
template <typename V>
static typename std::enable_if<can_be_used_in_variant<V>::value, void>::type addPropertyHelper(
    PropHolderContainer<variant_storage>& variant_props,
    std::unique_ptr<PropHolderContainer<std::any>>& any_props, DynPropTypes type, const V& value )
{
  (void)any_props;
  variant_props.addValue( type, value );
}
template <typename V>
static typename std::enable_if<!can_be_used_in_variant<V>::value, void>::type addPropertyHelper(
    PropHolderContainer<variant_storage>& variant_props,
    std::unique_ptr<PropHolderContainer<std::any>>& any_props, DynPropTypes type, const V& value )
{
  (void)variant_props;
  if ( !any_props )
    any_props.reset( new PropHolderContainer<std::any>() );
  any_props->addValue( type, value );
}

template <typename V>
static typename std::enable_if<can_be_used_in_variant<V>::value, void>::type removePropertyHelper(
    PropHolderContainer<variant_storage>& variant_props,
    std::unique_ptr<PropHolderContainer<std::any>>& any_props, DynPropTypes type )
{
  (void)any_props;
  variant_props.removeValue( type );
}
template <typename V>
static typename std::enable_if<!can_be_used_in_variant<V>::value, void>::type removePropertyHelper(
    PropHolderContainer<variant_storage>& variant_props,
    std::unique_ptr<PropHolderContainer<std::any>>& any_props, DynPropTypes type )
{
  (void)variant_props;
  passert_always( any_props.get() );
  any_props->removeValue( type );
}
}  // namespace

////////////////
// DynProps
inline DynProps::DynProps() : _prop_bits(), _props(), _any_props( nullptr ) {}

inline bool DynProps::hasProperty( DynPropTypes type ) const
{
  return _prop_bits.test( type );
}
template <typename V>
inline bool DynProps::getProperty( DynPropTypes type, V* value ) const
{
  if ( !hasProperty( type ) )
    return false;
  return getPropertyHelper<V>( _props, _any_props, type, value );
}

template <typename V>
inline void DynProps::setProperty( DynPropTypes type, const V& value )
{
  if ( hasProperty( type ) )
  {
    bool res = updatePropertyHelper<V>( _props, _any_props, type, value );
    passert_always( res );
    return;
  }
  _prop_bits.set( type, true );
  addPropertyHelper<V>( _props, _any_props, type, value );
}

template <typename V>
inline void DynProps::setPropertyPointer( DynPropTypes type, V value )
{
  if ( hasProperty( type ) )
  {
    passert_always( _any_props.get() );
    bool res = _any_props->updateValue( type, value );
    passert_always( res );
    return;
  }
  _prop_bits.set( type, true );
  if ( !_any_props )
    _any_props.reset( new PropHolderContainer<std::any>() );
  _any_props->addValue( type, value );
}

template <typename V>
inline void DynProps::removeProperty( DynPropTypes type )
{
  if ( !hasProperty( type ) )
    return;
  removePropertyHelper<V>( _props, _any_props, type );
  _prop_bits.set( type, false );
}

inline size_t DynProps::estimateSize() const
{
  size_t size = sizeof( std::bitset<PROP_FLAG_SIZE> ) + sizeof( std::unique_ptr<void> ) +
                _props.estimateSize();
  if ( _any_props )
    size += _any_props->estimateSize();
  return size;
}

////////////////
// DynamicPropsHolder

inline DynamicPropsHolder::DynamicPropsHolder() : _dynprops( nullptr ) {}


inline void DynamicPropsHolder::initProps()
{
  if ( !_dynprops )
    _dynprops.reset( new DynProps() );
}

template <typename V>
inline bool DynamicPropsHolder::getmember( DynPropTypes member, V* value ) const
{
  if ( !_dynprops )
    return false;
  return _dynprops->getProperty( member, value );
}

inline bool DynamicPropsHolder::hasmember( DynPropTypes member ) const
{
  if ( !_dynprops || !_dynprops->hasProperty( member ) )
    return false;
  return true;
}

template <typename V>
inline void DynamicPropsHolder::setmember( DynPropTypes member, const V& value,
                                           const V& defaultvalue )
{
  if ( value == defaultvalue )
  {
    if ( _dynprops )
      _dynprops->removeProperty<V>( member );
    return;
  }
  initProps();
  _dynprops->setProperty( member, value );
}

template <typename V>
inline void DynamicPropsHolder::setmemberPointer( DynPropTypes member, V value )
{
  if ( value == nullptr )
  {
    if ( _dynprops )
      _dynprops->removeProperty<V>( member );
    return;
  }
  initProps();
  _dynprops->setProperty( member, value );
}

inline size_t DynamicPropsHolder::estimateSizeDynProps() const
{
  size_t size = sizeof( DynamicPropsHolder );
  if ( _dynprops )
    size += _dynprops->estimateSize();
  return size;
}


}  // namespace Core
}  // namespace Pol

#endif
