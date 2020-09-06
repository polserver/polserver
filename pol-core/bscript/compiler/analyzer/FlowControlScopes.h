#ifndef POLSERVER_FLOWCONTROLSCOPES_H
#define POLSERVER_FLOWCONTROLSCOPES_H

#include <map>
#include <string>

namespace Pol::Bscript::Compiler
{
class FlowControlScope;
class Report;
class Variables;

class FlowControlScopes
{
public:
  FlowControlScopes( const Variables& local_variables, Report& report );

  const FlowControlScope* find( const std::string& name );

  bool any() const;

private:
  friend class FlowControlScope;
  void push( const FlowControlScope* );
  void pop( const FlowControlScope* );

  const FlowControlScope* current_unnamed_scope;
  std::map<std::string, const FlowControlScope*> named_scopes;
  const Variables& local_variables;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler


#endif  // POLSERVER_FLOWCONTROLSCOPES_H
