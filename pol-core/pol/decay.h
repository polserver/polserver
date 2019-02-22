/** @file
 *
 * @par History
 */

#ifndef __DECAY_H
#define __DECAY_H

namespace Pol
{
namespace Core
{
void decay_thread( void* );
void decay_thread_shadow( void* );
void decay_single_thread( void* );
}  // namespace Core
}  // namespace Pol
#endif
