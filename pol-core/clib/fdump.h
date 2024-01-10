/** @file
 *
 * @par History
 */


#pragma once

#include <format/format.h>
#include <iterator>

namespace Pol::Clib
{
void fdump( fmt::Writer& writer, const void* data, int len );
void fdump( std::back_insert_iterator<std::string> buffer, const void* data, int len );
}  // namespace Pol::Clib

