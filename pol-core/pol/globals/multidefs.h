/** @file
 *
 * @par History
 */


#ifndef GLOBALS_MULTIDEFS_H
#define GLOBALS_MULTIDEFS_H

#include <cstddef>
#include <map>

#include "../../clib/rawtypes.h"

namespace Pol
{
namespace Multi
{
class MultiDef;


using MultiDefs = std::map<u16, MultiDef*>;

// used for POL and uotool
class MultiDefBuffer
{
public:
  MultiDefBuffer();
  ~MultiDefBuffer();
  MultiDefBuffer( const MultiDefBuffer& ) = delete;
  MultiDefBuffer& operator=( const MultiDefBuffer& ) = delete;

  void deinitialize();
  size_t estimateSize() const;

  MultiDefs multidefs_by_multiid;

private:
};

extern MultiDefBuffer multidef_buffer;
}  // namespace Multi
}  // namespace Pol
#endif
