#ifndef POLSERVER_JSONASTBUILDER_H
#define POLSERVER_JSONASTBUILDER_H

#include <memory>
#include <string>
#include <vector>

#include "bscript/compiler/astbuilder/ProgramBuilder.h"
#include "bscript/compiler/model/UserFunctionInclusion.h"

namespace Pol::Bscript::Compiler
{
class JsonAst;
class Profile;
class Report;
class SourceFileLoader;

class JsonAstBuilder
{
public:
  JsonAstBuilder( SourceFileLoader& source_loader, Profile&, Report& );

  std::string build( const std::string& pathname, bool is_module );

  SourceFileLoader& source_loader;
  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_JSONASTBUILDER_H
