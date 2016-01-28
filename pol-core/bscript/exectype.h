/** @file
 *
 * @par History
 */


#ifndef __EXECTYPE_H
#define __EXECTYPE_H


#ifndef BSCRIPT_BOBJECT_H
#include "bobject.h"
#endif 

#include <vector>

namespace Pol {
  namespace Bscript {
	typedef std::vector<BObjectRef> BObjectRefVec;
  }
}
#endif
