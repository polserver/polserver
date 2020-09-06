/** @file
 *
 * @par History
 */


#ifndef REGION_H
#define REGION_H

#include <map>
#include <string>

#include "../plib/poltype.h"
#include "base/position.h"
#include "proplist.h"
#include "zone.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}
namespace Realms
{
class Realm;
}
namespace Core
{
class RegionGroupBase;

class Region
{
public:
  Bscript::BObjectImp* get_region_string( const std::string& propname );
  const std::string& name() const;
  RegionId regionid() const;
  virtual size_t estimateSize() const;

protected:
  explicit Region( Clib::ConfigElem& elem, RegionId id );
  virtual ~Region();

  // virtual void read_config( ConfigElem& elem );
  void read_custom_config( Clib::ConfigElem& elem );

  friend class RegionGroupBase;

protected:
  std::string name_;
  RegionId regionid_;
  //
  // custom per-region properties
  //
  // Note, these aren't "script-packed", they're just strings.
  //
  PropertyList proplist_;
};

inline const std::string& Region::name() const
{
  return name_;
}

inline RegionId Region::regionid() const
{
  return regionid_;
}

class RegionGroupBase
{
public:
  explicit RegionGroupBase( const char* name );
  virtual ~RegionGroupBase();
  virtual size_t estimateSize() const;

  void read_region( Clib::ConfigElem& elem );
  void create_bgnd_region( Clib::ConfigElem& elem );

  const std::string& name() const;

protected:
  Region* getregion_byname( const std::string& regionname );
  Region* getregion_byloc( const Pos2d& pos, Realms::Realm* realm );

  std::vector<Region*> regions_;

  typedef std::map<Realms::Realm*, RegionId**> RegionRealms;
  RegionRealms regionrealms;

private:
  virtual Region* create_region( Clib::ConfigElem& elem, RegionId id ) const = 0;

  RegionId getregionid( const Pos2d& pos, Realms::Realm* realm );
  void paint_zones( Clib::ConfigElem& elem, RegionId ridx );
  std::string name_;
  typedef std::map<std::string, Region*> RegionsByName;
  RegionsByName regions_byname_;
};

inline const std::string& RegionGroupBase::name() const
{
  return name_;
}

template <class T>
class RegionGroup : public RegionGroupBase
{
public:
  explicit RegionGroup( const char* name );

  virtual T* getregion( const Pos4d& pos );
  virtual T* getregion( const Pos2d& pos, Realms::Realm* realm );
  virtual T* getregion( const std::string& regionname );

protected:
  virtual Region* create_region( Clib::ConfigElem& elem, RegionId id ) const override;
};

template <class T>
RegionGroup<T>::RegionGroup( const char* name ) : RegionGroupBase( name )
{
}

template <class T>
inline T* RegionGroup<T>::getregion( const Pos4d& pos )
{
  return static_cast<T*>( getregion_byloc( pos.xy(), pos.realm() ) );
}

template <class T>
inline T* RegionGroup<T>::getregion( const Pos2d& pos, Realms::Realm* realm )
{
  return static_cast<T*>( getregion_byloc( pos, realm ) );
}

template <class T>
inline T* RegionGroup<T>::getregion( const std::string& regionname )
{
  return static_cast<T*>( getregion_byname( regionname ) );
}

template <class T>
inline Region* RegionGroup<T>::create_region( Clib::ConfigElem& elem, RegionId id ) const
{
  return new T( elem, id );
}

void read_region_data( RegionGroupBase& grp, const char* preferred_filename,
                       const char* other_filename, const char* tags_expected );
}  // namespace Core
}  // namespace Pol
#endif
