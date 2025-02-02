/** @file
 *
 * @par History
 */

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <string>

#include "../../clib/clib.h"
#include "../../clib/logfacility.h"
#include "../../clib/rawtypes.h"
#include "testenv.h"

#include <type_traits>

namespace Pol
{
namespace Testing
{
void clamp_test()
{
  // "from" start -> "to" res
#define T_CLAMP( from, start, to, res )                             \
  UnitTest( []() { return Clib::clamp_convert<to>( start ); }, res, \
            fmt::format( " {:#x} " #from " -> {:#x} " #to, start, res ) )

#define LMIN( T ) std::numeric_limits<T>::min()
#define LMAX( T ) std::numeric_limits<T>::max()

  // s8 combinations
  T_CLAMP( s8, LMIN( s8 ), s8, LMIN( s8 ) );
  T_CLAMP( s16, LMIN( s16 ), s8, LMIN( s8 ) );
  T_CLAMP( s32, LMIN( s32 ), s8, LMIN( s8 ) );
  T_CLAMP( s64, LMIN( s64 ), s8, LMIN( s8 ) );
  T_CLAMP( s8, LMAX( s8 ), s8, LMAX( s8 ) );
  T_CLAMP( s16, LMAX( s16 ), s8, LMAX( s8 ) );
  T_CLAMP( s32, LMAX( s32 ), s8, LMAX( s8 ) );
  T_CLAMP( s64, LMAX( s64 ), s8, LMAX( s8 ) );

  T_CLAMP( u8, LMIN( u8 ), s8, 0 );
  T_CLAMP( u16, LMIN( u16 ), s8, 0 );
  T_CLAMP( u32, LMIN( u32 ), s8, 0 );
  T_CLAMP( u64, LMIN( u64 ), s8, 0 );
  T_CLAMP( u8, LMAX( u8 ), s8, LMAX( s8 ) );
  T_CLAMP( u16, LMAX( u16 ), s8, LMAX( s8 ) );
  T_CLAMP( u32, LMAX( u32 ), s8, LMAX( s8 ) );
  T_CLAMP( u64, LMAX( u64 ), s8, LMAX( s8 ) );

  // s16 combinations
  T_CLAMP( s8, LMIN( s8 ), s16, LMIN( s8 ) );
  T_CLAMP( s16, LMIN( s16 ), s16, LMIN( s16 ) );
  T_CLAMP( s32, LMIN( s32 ), s16, LMIN( s16 ) );
  T_CLAMP( s64, LMIN( s64 ), s16, LMIN( s16 ) );
  T_CLAMP( s8, LMAX( s8 ), s16, LMAX( s8 ) );
  T_CLAMP( s16, LMAX( s16 ), s16, LMAX( s16 ) );
  T_CLAMP( s32, LMAX( s32 ), s16, LMAX( s16 ) );
  T_CLAMP( s64, LMAX( s64 ), s16, LMAX( s16 ) );

  T_CLAMP( u8, LMIN( u8 ), s16, 0 );
  T_CLAMP( u16, LMIN( u16 ), s16, 0 );
  T_CLAMP( u32, LMIN( u32 ), s16, 0 );
  T_CLAMP( u64, LMIN( u64 ), s16, 0 );
  T_CLAMP( u8, LMAX( u8 ), s16, (s16)LMAX( u8 ) );
  T_CLAMP( u16, LMAX( u16 ), s16, LMAX( s16 ) );
  T_CLAMP( u32, LMAX( u32 ), s16, LMAX( s16 ) );
  T_CLAMP( u64, LMAX( u64 ), s16, LMAX( s16 ) );

  // s32 combinations
  T_CLAMP( s8, LMIN( s8 ), s32, LMIN( s8 ) );
  T_CLAMP( s16, LMIN( s16 ), s32, LMIN( s16 ) );
  T_CLAMP( s32, LMIN( s32 ), s32, LMIN( s32 ) );
  T_CLAMP( s64, LMIN( s64 ), s32, LMIN( s32 ) );
  T_CLAMP( s8, LMAX( s8 ), s32, LMAX( s8 ) );
  T_CLAMP( s16, LMAX( s16 ), s32, LMAX( s16 ) );
  T_CLAMP( s32, LMAX( s32 ), s32, LMAX( s32 ) );
  T_CLAMP( s64, LMAX( s64 ), s32, LMAX( s32 ) );

  T_CLAMP( u8, LMIN( u8 ), s32, 0 );
  T_CLAMP( u16, LMIN( u16 ), s32, 0 );
  T_CLAMP( u32, LMIN( u32 ), s32, 0 );
  T_CLAMP( u64, LMIN( u64 ), s32, 0 );
  T_CLAMP( u8, LMAX( u8 ), s32, (s32)LMAX( u8 ) );
  T_CLAMP( u16, LMAX( u16 ), s32, (s32)LMAX( u16 ) );
  T_CLAMP( u32, LMAX( u32 ), s32, LMAX( s32 ) );
  T_CLAMP( u64, LMAX( u64 ), s32, LMAX( s32 ) );

  // s64 combinations
  T_CLAMP( s8, LMIN( s8 ), s64, LMIN( s8 ) );
  T_CLAMP( s16, LMIN( s16 ), s64, LMIN( s16 ) );
  T_CLAMP( s32, LMIN( s32 ), s64, LMIN( s32 ) );
  T_CLAMP( s64, LMIN( s64 ), s64, LMIN( s64 ) );
  T_CLAMP( s8, LMAX( s8 ), s64, LMAX( s8 ) );
  T_CLAMP( s16, LMAX( s16 ), s64, LMAX( s16 ) );
  T_CLAMP( s32, LMAX( s32 ), s64, LMAX( s32 ) );
  T_CLAMP( s64, LMAX( s64 ), s64, LMAX( s64 ) );

  T_CLAMP( u8, LMIN( u8 ), s64, 0 );
  T_CLAMP( u16, LMIN( u16 ), s64, 0 );
  T_CLAMP( u32, LMIN( u32 ), s64, 0 );
  T_CLAMP( u64, LMIN( u64 ), s64, 0 );
  T_CLAMP( u8, LMAX( u8 ), s64, (s64)LMAX( u8 ) );
  T_CLAMP( u16, LMAX( u16 ), s64, (s64)LMAX( u16 ) );
  T_CLAMP( u32, LMAX( u32 ), s64, (s64)LMAX( u32 ) );
  T_CLAMP( u64, LMAX( u64 ), s64, LMAX( s64 ) );

  // u8 combinations
  T_CLAMP( s8, LMIN( s8 ), u8, 0u );
  T_CLAMP( s16, LMIN( s16 ), u8, 0u );
  T_CLAMP( s32, LMIN( s32 ), u8, 0u );
  T_CLAMP( s64, LMIN( s64 ), u8, 0u );
  T_CLAMP( s8, LMAX( s8 ), u8, (u8)LMAX( s8 ) );
  T_CLAMP( s16, LMAX( s16 ), u8, LMAX( u8 ) );
  T_CLAMP( s32, LMAX( s32 ), u8, LMAX( u8 ) );
  T_CLAMP( s64, LMAX( s64 ), u8, LMAX( u8 ) );

  T_CLAMP( u8, LMIN( u8 ), u8, 0u );
  T_CLAMP( u16, LMIN( u16 ), u8, 0u );
  T_CLAMP( u32, LMIN( u32 ), u8, 0u );
  T_CLAMP( u64, LMIN( u64 ), u8, 0u );
  T_CLAMP( u8, LMAX( u8 ), u8, LMAX( u8 ) );
  T_CLAMP( u16, LMAX( u16 ), u8, LMAX( u8 ) );
  T_CLAMP( u32, LMAX( u32 ), u8, LMAX( u8 ) );
  T_CLAMP( u64, LMAX( u64 ), u8, LMAX( u8 ) );

  // u16 combinations
  T_CLAMP( s8, LMIN( s8 ), u16, 0u );
  T_CLAMP( s16, LMIN( s16 ), u16, 0u );
  T_CLAMP( s32, LMIN( s32 ), u16, 0u );
  T_CLAMP( s64, LMIN( s64 ), u16, 0u );
  T_CLAMP( s8, LMAX( s8 ), u16, (u16)LMAX( s8 ) );
  T_CLAMP( s16, LMAX( s16 ), u16, (u16)LMAX( s16 ) );
  T_CLAMP( s32, LMAX( s32 ), u16, LMAX( u16 ) );
  T_CLAMP( s64, LMAX( s64 ), u16, LMAX( u16 ) );

  T_CLAMP( u8, LMIN( u8 ), u16, 0u );
  T_CLAMP( u16, LMIN( u16 ), u16, 0u );
  T_CLAMP( u32, LMIN( u32 ), u16, 0u );
  T_CLAMP( u64, LMIN( u64 ), u16, 0u );
  T_CLAMP( u8, LMAX( u8 ), u16, LMAX( u8 ) );
  T_CLAMP( u16, LMAX( u16 ), u16, LMAX( u16 ) );
  T_CLAMP( u32, LMAX( u32 ), u16, LMAX( u16 ) );
  T_CLAMP( u64, LMAX( u64 ), u16, LMAX( u16 ) );

  // u32 combinations
  T_CLAMP( s8, LMIN( s8 ), u32, 0u );
  T_CLAMP( s16, LMIN( s16 ), u32, 0u );
  T_CLAMP( s32, LMIN( s32 ), u32, 0u );
  T_CLAMP( s64, LMIN( s64 ), u32, 0u );
  T_CLAMP( s8, LMAX( s8 ), u32, (u32)LMAX( s8 ) );
  T_CLAMP( s16, LMAX( s16 ), u32, (u32)LMAX( s16 ) );
  T_CLAMP( s32, LMAX( s32 ), u32, (u32)LMAX( s32 ) );
  T_CLAMP( s64, LMAX( s64 ), u32, LMAX( u32 ) );

  T_CLAMP( u8, LMIN( u8 ), u32, 0u );
  T_CLAMP( u16, LMIN( u16 ), u32, 0u );
  T_CLAMP( u32, LMIN( u32 ), u32, 0u );
  T_CLAMP( u64, LMIN( u64 ), u32, 0u );
  T_CLAMP( u8, LMAX( u8 ), u32, LMAX( u8 ) );
  T_CLAMP( u16, LMAX( u16 ), u32, LMAX( u16 ) );
  T_CLAMP( u32, LMAX( u32 ), u32, LMAX( u32 ) );
  T_CLAMP( u64, LMAX( u64 ), u32, LMAX( u32 ) );

  // u64 combinations
  T_CLAMP( s8, LMIN( s8 ), u64, 0u );
  T_CLAMP( s16, LMIN( s16 ), u64, 0u );
  T_CLAMP( s32, LMIN( s32 ), u64, 0u );
  T_CLAMP( s64, LMIN( s64 ), u64, 0u );
  T_CLAMP( s8, LMAX( s8 ), u64, (u64)LMAX( s8 ) );
  T_CLAMP( s16, LMAX( s16 ), u64, (u64)LMAX( s16 ) );
  T_CLAMP( s32, LMAX( s32 ), u64, (u64)LMAX( s32 ) );
  T_CLAMP( s64, LMAX( s64 ), u64, (u64)LMAX( s64 ) );

  T_CLAMP( u8, LMIN( u8 ), u64, 0u );
  T_CLAMP( u16, LMIN( u16 ), u64, 0u );
  T_CLAMP( u32, LMIN( u32 ), u64, 0u );
  T_CLAMP( u64, LMIN( u64 ), u64, 0u );
  T_CLAMP( u8, LMAX( u8 ), u64, LMAX( u8 ) );
  T_CLAMP( u16, LMAX( u16 ), u64, LMAX( u16 ) );
  T_CLAMP( u32, LMAX( u32 ), u64, LMAX( u32 ) );
  T_CLAMP( u64, LMAX( u64 ), u64, LMAX( u64 ) );
#undef T_CLAMP
#undef LMAX
#undef LMIN
}

}  // namespace Testing
}  // namespace Pol
