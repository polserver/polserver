#ifndef POLSERVER_COMPILERWORKSPACEBUILDER_H
#define POLSERVER_COMPILERWORKSPACEBUILDER_H

#include <memory>
#include <string>

namespace Pol::Bscript::Compiler
{
struct LegacyFunctionOrder;
class Profile;
class Report;
class CompilerWorkspace;

class CompilerWorkspaceBuilder
{
public:
  CompilerWorkspaceBuilder( Profile&, Report& );

  std::unique_ptr<CompilerWorkspace> build(
      const std::string& pathname,
      const LegacyFunctionOrder* legacy_function_order );

private:

  Profile& profile;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILERWORKSPACEBUILDER_H
