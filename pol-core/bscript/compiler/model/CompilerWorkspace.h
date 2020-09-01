#ifndef POLSERVER_COMPILERWORKSPACE_H
#define POLSERVER_COMPILERWORKSPACE_H

#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class Block;
class ModuleFunctionDeclaration;
class Program;
class SourceFile;
class SourceFileIdentifier;
class TopLevelStatements;
class UserFunction;

class CompilerWorkspace
{
public:
  CompilerWorkspace();
  ~CompilerWorkspace();

  std::unique_ptr<TopLevelStatements> top_level_statements;
  std::vector<std::unique_ptr<ModuleFunctionDeclaration>> module_function_declarations;
  std::vector<std::unique_ptr<UserFunction>> user_functions;
  std::unique_ptr<Program> program;

  // These reference ModuleFunctionDeclaration objects that are in module_functions
  std::vector<ModuleFunctionDeclaration*> referenced_module_function_declarations;
  std::vector<const ModuleFunctionDeclaration*> module_functions_in_legacy_order;

  std::vector<std::unique_ptr<SourceFileIdentifier>> referenced_source_file_identifiers;

  std::vector<std::string> global_variable_names;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILERWORKSPACE_H
