#ifndef POLSERVER_COMPILERWORKSPACEBUILDER_H
#define POLSERVER_COMPILERWORKSPACEBUILDER_H

#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class BuilderWorkspace;
struct LegacyFunctionOrder;
class Profile;
class Report;
class CompilerWorkspace;
class ModuleFunctionDeclaration;
class SourceFileCache;

class CompilerWorkspaceBuilder
{
public:
  CompilerWorkspaceBuilder( SourceFileCache& em_cache, SourceFileCache& inc_cache, Profile&,
                            Report& );

  std::unique_ptr<CompilerWorkspace> build(
      const std::string& pathname,
      const LegacyFunctionOrder* legacy_function_order );

private:
  std::vector<const ModuleFunctionDeclaration*> get_module_functions_in_order(
      BuilderWorkspace& workspace, const LegacyFunctionOrder& h );


  SourceFileCache& em_cache;
  SourceFileCache& inc_cache;
  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILERWORKSPACEBUILDER_H
