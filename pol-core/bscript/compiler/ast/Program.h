#ifndef POLSERVER_PROGRAM_H
#define POLSERVER_PROGRAM_H

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/model/LocalVariableScopeInfo.h"

namespace Pol::Bscript::Compiler
{
class FunctionBody;
class ProgramParameterList;
class Variable;

class Program : public Node
{
public:
  Program( const SourceLocation&, const std::string&, std::unique_ptr<ProgramParameterList>,
           std::unique_ptr<FunctionBody> );
  ~Program() override;

  void accept( NodeVisitor& ) override;
  void describe_to( std::string& ) const override;
  std::string type() const override;
  void describe_to( picojson::object& ) const override;

  const std::string program_name;
  ProgramParameterList& parameter_list();
  FunctionBody& body();

  LocalVariableScopeInfo local_variable_scope_info;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_PROGRAM_H
