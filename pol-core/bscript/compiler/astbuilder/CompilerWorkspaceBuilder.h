#ifndef POLSERVER_COMPILERWORKSPACEBUILDER_H
#define POLSERVER_COMPILERWORKSPACEBUILDER_H

#include <memory>
#include <string>
#include <vector>

#include "bscript/compiler/model/UserFunctionInclusion.h"

namespace Pol::Bscript::Compiler
{
class BuilderWorkspace;
class CompilerWorkspace;
class ModuleFunctionDeclaration;
class Profile;
class Report;
class SourceFileCache;

class CompilerWorkspaceBuilder
{
public:
  CompilerWorkspaceBuilder( SourceFileCache& em_cache, SourceFileCache& inc_cache, Profile&,
                            Report& );

  std::unique_ptr<CompilerWorkspace> build( const std::string& pathname, UserFunctionInclusion );

private:
  void build_referenced_user_functions( BuilderWorkspace& );

  SourceFileCache& em_cache;
  SourceFileCache& inc_cache;
  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILERWORKSPACEBUILDER_H
