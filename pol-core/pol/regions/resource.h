/** @file
 *
 * @par History
 */


#ifndef RESOURCE_H
#define RESOURCE_H

// TODO: RESSOURCES DONT HANDLE REALMS!

#include <iosfwd>
#include <map>
#include <set>
#include <string>
#include <time.h>

#include "../clib/rawtypes.h"
#include "../plib/poltype.h"
#include "base/position.h"
#include "regions/region.h"
#include "zone.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}  // namespace Bscript
namespace Clib
{
class ConfigElem;
class StreamWriter;
}  // namespace Clib
namespace Realms
{
class Realm;
}  // namespace Realms
}  // namespace Pol

namespace Pol
{
namespace Items
{
void remove_resources( u32 objtype, u16 amount );
void return_resources( u32 objtype, u16 amount );
}  // namespace Items
namespace Core
{
class ResourceDef;

Bscript::BObjectImp* get_harvest_difficulty( const char* resource, const Pos4d& pos,
                                             unsigned short marker );
Bscript::BObjectImp* harvest_resource( const char* resource, const Pos4d& pos, int b, int n );

Bscript::BObjectImp* get_region_string( const char* resource, const Pos4d& pos,
                                        const std::string& propname );

ResourceDef* find_resource_def( const std::string& rname );
void write_resources_dat( Clib::StreamWriter& sw_resource );
void read_resources_dat();
void clean_resources();


class ResourceRegion : public Region
{
public:
  explicit ResourceRegion( Clib::ConfigElem& elem, RegionId id );
  size_t estimateSize() const override;

  void read_data( Clib::ConfigElem& elem );

  Bscript::BObjectImp* get_harvest_difficulty( const Pos4d& pos );
  Bscript::BObjectImp* harvest_resource( const Pos4d& pos, int b, int n );

  void regenerate( time_t now );
  void write( Clib::StreamWriter& sw, const std::string& resource_name ) const;

  unsigned int tilecount_;
  friend class ResourceDef;

private:
  //
  // These are only used for depletion levels.
  //
  const unsigned int units_per_area_;
  const unsigned int seconds_per_regrow_;
  typedef std::map<unsigned int, unsigned short> Depletions;
  Depletions depletions_;
  time_t last_regen_;


  //
  // the following are used if resource banks are implemented.
  //
  unsigned int capacity_;  // config
  unsigned int units_;     // data
};


class ResourceDef final : public RegionGroup<ResourceRegion>
{
public:
  explicit ResourceDef( const char* name );
  void read_config( Clib::ConfigElem& elem );
  void read_data( Clib::ConfigElem& elem );
  size_t estimateSize() const override;
  // void read_region( ConfigElem& elem );

  bool findmarker( const Pos4d& pos, unsigned int objtype );
  void regenerate( time_t now );
  void consume( unsigned amount );
  void produce( unsigned amount );
  void counttiles();
  void write( Clib::StreamWriter& sw ) const;

protected:
private:
  unsigned int initial_units_;
  int current_units_;
  std::set<unsigned short> landtiles_;
  std::set<unsigned short> tiles_;
  // std::vector<ResourceRegion*> regions_;
};
}  // namespace Core
}  // namespace Pol
#endif  // RESOURCE_H
