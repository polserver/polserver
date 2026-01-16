#ifndef CLIB_WALLCLOCK_H
#define CLIB_WALLCLOCK_H

namespace Pol
{
namespace Clib
{
using wallclock_t = unsigned int;
using wallclock_diff_t = unsigned int;

wallclock_t wallclock();
wallclock_diff_t wallclock_diff_ms( wallclock_t start, wallclock_t finish );
}  // namespace Clib
}  // namespace Pol
#endif  // CLIB_WALLCLOCK_H
