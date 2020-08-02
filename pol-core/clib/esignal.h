/** @file
 *
 * @par History
 */

#ifndef CLIB_ESIGNAL_H
#define CLIB_ESIGNAL_H

#include <atomic>

namespace Pol
{
namespace Clib
{
void signal_exit( int exit_code = 0 );
bool exit_signalled();
int exit_code();
void enable_exit_signaller();
}  // namespace Clib
}  // namespace Pol

#endif  // CLIB_ESIGNAL_H