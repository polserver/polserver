#ifndef POLSERVER_COMPILERWORKSPACEBUILDER_H
#define POLSERVER_COMPILERWORKSPACEBUILDER_H

#include <memory>
#include <string>

namespace Pol::Bscript::Compiler
{
class BuilderWorkspace;
struct LegacyFunctionOrder;
class Profile;
class Report;
class CompilerWorkspace;
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
  void build_referenced_user_functions( BuilderWorkspace& );

  SourceFileCache& em_cache;
  SourceFileCache& inc_cache;
  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILERWORKSPACEBUILDER_H
