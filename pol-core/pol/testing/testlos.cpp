/** @file
 *
 * @par History
 */


#include <pol_global_config.h>

#ifdef ENABLE_BENCHMARK
#include <benchmark/benchmark.h>
#include <memory>

#include "pol/item/item.h"
#include "pol/module/uomod.h"
#include "pol/scrsched.h"
#include "pol/uoexec.h"
#include "pol/uoscrobj.h"

namespace Pol::Testing
{
static void BM_method( benchmark::State& state )
{
  std::unique_ptr<Core::UOExecutor> ex( Core::create_script_executor() );
  auto uoemod = new Module::UOExecutorModule( *ex );
  ex->addModule( uoemod );
  Items::Item* item = Items::Item::create( 0xffa1 );
  auto ref = new Module::EItemRefObjImp( item );

  while ( state.KeepRunning() )
  {
    ref->call_method( "test", *( ex.get() ) );
  }
}
BENCHMARK( BM_method );

static void BM_member_id( benchmark::State& state )
{
  Items::Item* item = Items::Item::create( 0xffa1 );
  auto ref = new Module::EItemRefObjImp( item );

  while ( state.KeepRunning() )
  {
    ref->get_member_id( 1 );
  }
}
BENCHMARK( BM_member_id );
}  // namespace Pol::Testing
#endif
