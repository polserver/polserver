/** @file
 *
 * @par History
 */


#ifndef GLOBALS_MULTIDEFS_H
#define GLOBALS_MULTIDEFS_H

#include "../../clib/rawtypes.h"

#include <map>
#include <cstddef>
#include <boost/noncopyable.hpp>

namespace Pol
{
namespace Multi
{
class MultiDef;


typedef std::map< u16, MultiDef* > MultiDefs;

// used for POL and uotool
class MultiDefBuffer : boost::noncopyable
{
public:
  MultiDefBuffer();
  ~MultiDefBuffer();

  void deinitialize();
  size_t estimateSize() const;

  MultiDefs multidefs_by_multiid;

private:
};

extern MultiDefBuffer multidef_buffer;
}
}
#endif
