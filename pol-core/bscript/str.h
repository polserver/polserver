#pragma once

#include <fmt/ostream.h>

#include <sstream>
#include <string>

namespace Pol::Bscript
{
class BObjectImp;
class BObject;

bool try_to_format( std::stringstream& to_stream, BObjectImp* what, std::string& frmt );
std::string get_formatted( BObjectImp* what, std::string& frmt );
}  // namespace Pol::Bscript

template <>
struct fmt::formatter<Pol::Bscript::BObjectImp> : fmt::ostream_formatter
{
};
template <>
struct fmt::formatter<Pol::Bscript::BObject> : fmt::ostream_formatter
{
};
