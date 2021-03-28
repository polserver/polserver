#ifndef POLSERVER_SCOPETREE_H
#define POLSERVER_SCOPETREE_H

#include "bscript/compiler/file/SourceLocation.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Pol::Bscript::Compiler
{
class Variable;

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
  void push_scope( const SourceLocation& );
  void pop_scope( std::vector<std::shared_ptr<Variable>> variables );
  void set_globals( std::vector<std::shared_ptr<Variable>> variables );
  std::shared_ptr<Variable> find_variable( const std::string&, const Position& ) const;

private:
  std::map<std::string, std::shared_ptr<Variable>> globals;
  std::vector<std::shared_ptr<ScopeInfo>> scopes;
  unsigned ignored = 0;

  friend fmt::Writer& operator<<( fmt::Writer& w, const ScopeTree& );
};

fmt::Writer& operator<<( fmt::Writer&, const ScopeTree& );
fmt::Writer& operator<<( fmt::Writer& w, const ScopeInfo& );
}  // namespace Pol::Bscript::Compiler
#endif
