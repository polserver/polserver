#ifndef POLSERVER_SCOPETREE_H
#define POLSERVER_SCOPETREE_H

#include "bscript/compiler/file/SourceLocation.h"
#include "clib/maputil.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class ModuleFunctionDeclaration;
class UserFunction;
class Variable;
class ConstDeclaration;
class ScopeInfo
{
public:
  ScopeInfo( const SourceLocation& );
  SourceLocation location;
  std::map<std::string, std::shared_ptr<Variable>> variables;
  std::vector<std::shared_ptr<ScopeInfo>> children;

  std::shared_ptr<Variable> walk( const std::string&, const Position& ) const;
  std::shared_ptr<Variable> resolve( const std::string& ) const;

private:
  static void describe_tree_to_indented( fmt::Writer&, const ScopeInfo&, unsigned indent );
  friend fmt::Writer& operator<<( fmt::Writer&, const ScopeInfo& );
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

private:
  CompilerWorkspace& workspace;
  std::map<std::string, std::shared_ptr<Variable>, Clib::ci_cmp_pred> globals;
  std::vector<std::shared_ptr<ScopeInfo>> scopes;
  unsigned ignored = 0;

  friend fmt::Writer& operator<<( fmt::Writer& w, const ScopeTree& );
};

fmt::Writer& operator<<( fmt::Writer&, const ScopeTree& );
fmt::Writer& operator<<( fmt::Writer& w, const ScopeInfo& );
}  // namespace Pol::Bscript::Compiler
#endif
