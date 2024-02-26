#pragma once

#include <string>

namespace Pol::Bscript::Compiler
{
class Profile;
class Report;

class PrettifyBuilder
{
public:
  PrettifyBuilder( Profile&, Report& );

  std::string build( const std::string& pathname, bool is_module );

  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler
