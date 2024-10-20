#ifndef POLSERVER_SCOPETREE_H
#define POLSERVER_SCOPETREE_H

#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/ScopeName.h"
#include "clib/maputil.h"
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class ModuleFunctionDeclaration;
class UserFunction;
class Variable;
class ConstDeclaration;
class ClassDeclaration;
class ScopeInfo
{
public:
  ScopeInfo( const SourceLocation& );
  SourceLocation location;
  std::map<std::string, std::shared_ptr<Variable>> variables;
  std::vector<std::shared_ptr<ScopeInfo>> children;

  std::shared_ptr<Variable> walk( const std::string&, const Position& ) const;
  std::vector<std::shared_ptr<Variable>> walk_list( const std::string&, const Position& ) const;
  std::shared_ptr<Variable> resolve( const std::string& ) const;
  std::vector<std::shared_ptr<Variable>> resolve_list( const std::string& ) const;

private:
  static void describe_tree_to_indented( std::string&, const ScopeInfo&, unsigned indent );
};

struct ScopeTreeQuery
{
  // Calling scope, either empty-string (for Global) or a class name.
  std::string calling_scope;

  // The scope of the prefix for the query. The scope can be empty (eg. `print`)
  // or explicitly global (eg. `::print`).
  ScopeName prefix_scope;

  // The prefix to search for.
  std::string prefix;
};

class ScopeTree
{
public:
  ScopeTree( CompilerWorkspace& workspace );
  void push_scope( const SourceLocation& );
  void pop_scope( std::vector<std::shared_ptr<Variable>> variables );
  void set_globals( std::vector<std::shared_ptr<Variable>> variables );
  std::shared_ptr<Variable> find_variable( std::string name, const Position& ) const;
  // Data is owned by CompilerWorkspace
  UserFunction* find_user_function( std::string name ) const;
  // Data is owned by CompilerWorkspace
  ModuleFunctionDeclaration* find_module_function( std::string name ) const;
  // Data is owned by CompilerWorkspace
  ConstDeclaration* find_constant( std::string name ) const;

  std::vector<std::shared_ptr<Variable>> list_variables( const ScopeTreeQuery& query,
                                                         const Position& ) const;
  // Data is owned by CompilerWorkspace
  std::vector<UserFunction*> list_user_functions( const ScopeTreeQuery& query, const Position& ) const;
  // Data is owned by CompilerWorkspace
  std::vector<ModuleFunctionDeclaration*> list_module_functions(
      const ScopeTreeQuery& query ) const;
  // Data is owned by CompilerWorkspace
  std::vector<ConstDeclaration*> list_constants( const ScopeTreeQuery& query ) const;
  // Data is owned by CompilerWorkspace
  std::vector<ClassDeclaration*> list_classes( const ScopeTreeQuery& query ) const;

  std::vector<std::string> list_modules( const ScopeTreeQuery& query ) const;

private:
  CompilerWorkspace& workspace;
  std::map<std::string, std::shared_ptr<Variable>, Clib::ci_cmp_pred> globals;
  std::vector<std::shared_ptr<ScopeInfo>> scopes;
  unsigned ignored = 0;
};

}  // namespace Pol::Bscript::Compiler
#endif
