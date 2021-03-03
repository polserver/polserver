#ifndef __STR_H
#define __STR_H

#include <sstream>
#include <string>
#include "bobject.h"

namespace Pol::Bscript
{
std::string try_to_format( BObjectImp* what, std::string& frmt );
}
#endif
