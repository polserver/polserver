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
class SourceFileLoader;

class CompilerWorkspaceBuilder
{
public:
  CompilerWorkspaceBuilder( SourceFileLoader& source_loader, SourceFileCache& em_cache,
                            SourceFileCache& inc_cache, bool continue_on_error, Profile&, Report& );

  std::unique_ptr<CompilerWorkspace> build( const std::string& pathname, UserFunctionInclusion );
  std::unique_ptr<CompilerWorkspace> build_module( const std::string& pathname );

private:
  void build_referenced_user_functions( BuilderWorkspace& );

  SourceFileLoader& source_loader;
  SourceFileCache& em_cache;
  SourceFileCache& inc_cache;
  const bool continue_on_error;
  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILERWORKSPACEBUILDER_H
