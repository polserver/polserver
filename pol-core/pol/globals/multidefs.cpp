/** @file
 *
 * @par History
 * - 2009/09/01 Turley:    VS2005/2008 Support moved inline MultiDef::getkey to .h
 * - 2009/09/03 MuadDib:   Relocation of multi related cpp/h
 */


#include "multidefs.h"

#include <cstddef>

#include "../../clib/stlutil.h"
#include "../multi/multidef.h"


namespace Pol::Multi
{
MultiDefBuffer multidef_buffer;

MultiDefBuffer::MultiDefBuffer() : multidefs_by_multiid() {}
MultiDefBuffer::~MultiDefBuffer() = default;

void MultiDefBuffer::deinitialize()
{
  Multi::MultiDefs::iterator iter = multidefs_by_multiid.begin();
  for ( ; iter != multidefs_by_multiid.end(); ++iter )
  {
    if ( iter->second != nullptr )
      delete iter->second;
    iter->second = nullptr;
  }
  multidefs_by_multiid.clear();
}

size_t MultiDefBuffer::estimateSize() const
{
  size_t size = Clib::memsize( multidefs_by_multiid );

  for ( const auto& m_pair : multidefs_by_multiid )
  {
    if ( m_pair.second != nullptr )
      size += m_pair.second->estimateSize();
  }
  return size;
}
}  // namespace Pol::Multi
