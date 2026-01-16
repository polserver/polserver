/** @file
 *
 * @par History
 */


#ifndef POLDBG_H
#define POLDBG_H

namespace Pol
{
namespace Bscript
{
class BObjectImp;
}
namespace Core
{
void debug_listen_thread();
Bscript::BObjectImp* create_debug_context();
}  // namespace Core
}  // namespace Pol
#endif
