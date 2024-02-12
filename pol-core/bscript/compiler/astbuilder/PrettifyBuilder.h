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
class SourceFileLoader;

class PrettifyBuilder
{
public:
  PrettifyBuilder( SourceFileLoader& source_loader, Profile&, Report& );

  std::string build( const std::string& pathname, bool is_module );

  SourceFileLoader& source_loader;
  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler
