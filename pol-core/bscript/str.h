#ifndef __STR_H
#define __STR_H

#include "bobject.h"
#include <sstream>
#include <string>

namespace Pol::Bscript
{
std::string get_formatted( BObjectImp* what, std::string& frmt );
}
#endif
