#ifndef BASEOBJECT_H
#define BASEOBJECT_H

#include "clib/rawtypes.h"

#include "base/position.h"

#include <utility>

namespace Pol
{
namespace Realms
{
class Realm;
}
namespace Core
{
/**
 * This is meant to be coarse-grained. It's meant as an alternative to dynamic_cast.
 *
 * Mostly used to go from UItem to UContainer.
 *
 * @warning When adding a class, be sure to to also update class_to_type static method
 */
enum class UOBJ_CLASS : u8
{
  CLASS_ITEM,
  CLASS_CONTAINER,
  CLASS_CHARACTER,
  CLASS_NPC,
  CLASS_WEAPON,
  CLASS_ARMOR,
  CLASS_MULTI,

  INVALID = 0xff,
};

// ULWObject: Lightweight object.
// Should contain minimal data structures (and no virtuals)
// can also be created with arbritary data check los.h
class ULWObject
{
protected:
  ULWObject( UOBJ_CLASS uobj_class );
  ULWObject( const ULWObject& ) = delete;
  ULWObject& operator=( const ULWObject& ) = delete;
  ~ULWObject() = default;

  void pos( Pos4d newpos );

public:
  bool orphan() const;

  bool isa( UOBJ_CLASS uobj_class ) const;
  bool ismobile() const;
  bool isitem() const;
  bool ismulti() const;

  u8 look_height() const;  // where you're looking from, or to

  const Pos4d& pos() const;
  const Pos3d& pos3d() const;
  const Pos2d& pos2d() const;
  u16 x() const;
  u16 y() const;
  s8 z() const;
  Realms::Realm* realm() const;

  UFACING direction_toward( ULWObject* other ) const;
  UFACING direction_toward( const Pos2d& other ) const;
  UFACING direction_away( ULWObject* other ) const;
  UFACING direction_away( const Pos2d& other ) const;

private:
  Pos4d position;

public:
  u32 serial;
  u16 graphic;
  u8 height;

protected:
  const UOBJ_CLASS uobj_class_;
};

inline ULWObject::ULWObject( UOBJ_CLASS uobj_class )
    : position(), serial( 0 ), graphic( 0 ), height( 0 ), uobj_class_( uobj_class )
{
}

inline u8 ULWObject::look_height() const
{
  switch ( uobj_class_ )
  {
  case UOBJ_CLASS::CLASS_ITEM:
  case UOBJ_CLASS::CLASS_CONTAINER:
  case UOBJ_CLASS::CLASS_WEAPON:
  case UOBJ_CLASS::CLASS_ARMOR:
  case UOBJ_CLASS::CLASS_MULTI:
    return height / 2;

  case UOBJ_CLASS::CLASS_CHARACTER:
  case UOBJ_CLASS::CLASS_NPC:
  case UOBJ_CLASS::INVALID:
    return height;
  }
  return 0;
}

inline bool ULWObject::isa( UOBJ_CLASS uobj_class ) const
{
  return uobj_class_ == uobj_class;
}

inline bool ULWObject::ismobile() const
{
  return ( uobj_class_ == UOBJ_CLASS::CLASS_CHARACTER || uobj_class_ == UOBJ_CLASS::CLASS_NPC );
}

inline bool ULWObject::isitem() const
{
  return !ismobile();
}

inline bool ULWObject::ismulti() const
{
  return ( uobj_class_ == UOBJ_CLASS::CLASS_MULTI );
}

inline bool ULWObject::orphan() const
{
  return ( serial == 0 );
}

inline const Pos4d& ULWObject::pos() const
{
  return position;
}
inline void ULWObject::pos( Pos4d newpos )
{
  position = std::move( newpos );
}
inline const Pos3d& ULWObject::pos3d() const
{
  return pos().xyz();
}
inline const Pos2d& ULWObject::pos2d() const
{
  return pos().xy();
}

// TODO POS remove as final step
inline u16 ULWObject::x() const
{
  return position.x();
}
inline u16 ULWObject::y() const
{
  return position.y();
}
inline s8 ULWObject::z() const
{
  return position.z();
}
inline Realms::Realm* ULWObject::realm() const
{
  return position.realm();
}

inline UFACING ULWObject::direction_toward( ULWObject* other ) const
{
  return pos().xy().direction_toward( other->pos().xy() );
}
inline UFACING ULWObject::direction_toward( const Pos2d& other ) const
{
  return pos().xy().direction_toward( other );
}
inline UFACING ULWObject::direction_away( ULWObject* other ) const
{
  return pos().xy().direction_away( other->pos().xy() );
}
inline UFACING ULWObject::direction_away( const Pos2d& other ) const
{
  return pos().xy().direction_away( other );
}

}  // namespace Core
}  // namespace Pol

#endif
