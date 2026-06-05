#pragma once

#include <sstream>
#include <string>

namespace Pol::Bscript
{
class BObjectImp;
bool try_to_format( std::stringstream& to_stream, BObjectImp* what, std::string& frmt );
std::string get_formatted( BObjectImp* what, std::string& frmt );
}  // namespace Pol::Bscript
