/** @file
 *
 * @par History
 */


#pragma once

#include <iterator>
#include <string>

namespace Pol::Clib
{
void fdump( std::back_insert_iterator<std::string> buffer, const void* data, int len );
}  // namespace Pol::Clib

