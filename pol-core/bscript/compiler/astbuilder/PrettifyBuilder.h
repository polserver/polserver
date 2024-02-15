#pragma once

#include <memory>
#include <string>
#include <vector>

#include "bscript/compiler/astbuilder/ProgramBuilder.h"
#include "bscript/compiler/model/UserFunctionInclusion.h"

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
