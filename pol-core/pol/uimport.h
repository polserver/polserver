/** @file
 *
 * @par History
 * - 2011/11/28 MuadDib:   Removed last of uox referencing code.
 */

#ifndef __UIMPORT_H
#define __UIMPORT_H
namespace Pol
{
namespace Core
{
int read_data();

void read_starting_locations();
void read_gameservers();
}  // namespace Core
namespace Accounts
{
void read_account_data();
}
}  // namespace Pol
#endif
