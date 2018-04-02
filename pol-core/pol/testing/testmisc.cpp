/** @file
 *
 * @par History
 */

#include <string>

#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "../../plib/maptile.h"
#include "../dynproperties.h"
#include "../globals/uvars.h"
#include "../realms/realm.h"

namespace Pol
{
namespace Testing
{
void dummy() {}

void map_test()
{
  Plib::MAPTILE_CELL cell = Core::gamestate.main_realm->getmaptile( 1453, 1794 );
  INFO_PRINT << cell.landtile << " " << cell.z << "\n";
}

void dynprops_test()
{
  class Test : public Core::DynamicPropsHolder
  {
  public:
    DYN_PROPERTY( armod, s16, Core::PROP_AR_MOD, 0 );
    DYN_PROPERTY( max_items, u32, Core::PROP_MAX_ITEMS_MOD, 0 );
    DYN_PROPERTY( itemname, std::string, Core::PROP_NAME_SUFFIX, "" );
  };
  Test h;
  INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
  INFO_PRINT << "ar " << h.armod() << " " << h.has_armod() << "\n";
  h.armod( 10 );
  INFO_PRINT << "ar " << h.armod() << " " << h.has_armod() << "\n";
  h.armod( 0 );
  INFO_PRINT << "ar " << h.armod() << " " << h.has_armod() << "\n";
  INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
  INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
  h.itemname( "hello world" );
  INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
  h.itemname( "" );
  INFO_PRINT << "name " << h.itemname() << " " << h.has_itemname() << "\n";
  INFO_PRINT << "size " << h.estimateSizeDynProps() << "\n";
}

}  // namespace Testing
}  // namespace Pol
