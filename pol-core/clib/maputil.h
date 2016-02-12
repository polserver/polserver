/** @file
 *
 * @par History
 */


#ifndef CLIB_MAPUTIL_H
#define CLIB_MAPUTIL_H

#include "clib.h"
#include <cstring>
#include <string>

#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable:4996) // disables POSIX name deprecation warning for stricmp 
#endif

namespace Pol {
  namespace Clib {
	class ci_cmp_pred
	{
	public:
	  bool operator()( const std::string& x1, const std::string& x2 ) const
	  {
		return stricmp( x1.c_str(), x2.c_str() ) < 0;
	  }
	};
  }
}
#ifdef _MSC_VER
    #pragma warning(pop)
#endif

#endif
