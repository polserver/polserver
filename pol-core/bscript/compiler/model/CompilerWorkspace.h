#ifndef POLSERVER_COMPILERWORKSPACE_H
#define POLSERVER_COMPILERWORKSPACE_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bscript/compiler/analyzer/Constants.h"
#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/model/ScopeTree.h"
#include "bscript/compiler/model/SemanticTokens.h"
#include "clib/maputil.h"

namespace Pol::Bscript::Compiler
{
class Block;
class ClassDeclaration;
class FlowControlLabel;
class ConstDeclaration;
class ModuleFunctionDeclaration;
class Program;
class Report;
class SourceFile;
class SourceFileIdentifier;
class TopLevelStatements;
class UserFunction;

class CompilerWorkspace
{
public:
  explicit CompilerWorkspace( Report& );
  ~CompilerWorkspace();

  void accept( NodeVisitor& );

  std::shared_ptr<SourceFile> source;

  std::vector<std::unique_ptr<ConstDeclaration>> const_declarations;
  Constants constants;

  std::unique_ptr<TopLevelStatements> top_level_statements;
  std::vector<std::unique_ptr<ModuleFunctionDeclaration>> module_function_declarations;
  std::vector<std::unique_ptr<UserFunction>> user_functions;
  std::vector<std::unique_ptr<ClassDeclaration>> class_declarations;
  std::map<std::string, size_t> class_declaration_indexes;
  std::unique_ptr<Program> program;

  // These reference ModuleFunctionDeclaration objects that are in module_functions
  std::vector<ModuleFunctionDeclaration*> referenced_module_function_declarations;

  std::vector<std::unique_ptr<SourceFileIdentifier>> referenced_source_file_identifiers;

  std::map<std::string, SourceLocation, Clib::ci_cmp_pred> all_function_locations;
  std::map<std::string, SourceLocation, Clib::ci_cmp_pred> all_class_locations;

  std::vector<std::string> global_variable_names;
  std::map<std::string, FlowControlLabel> user_function_labels;
  ScopeTree scope_tree;
  SemanticTokens tokens;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_COMPILERWORKSPACE_H
