#ifndef POLSERVER_PROGRAM_H
#define POLSERVER_PROGRAM_H

#include "compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class ProgramParameterList;
class FunctionBody;

class Program : public Node
{
public:
  Program( const SourceLocation&, std::unique_ptr<ProgramParameterList>,
           std::unique_ptr<FunctionBody> );
  ~Program();

  void accept( NodeVisitor& ) override;
  void describe_to( fmt::Writer& ) const override;

  ProgramParameterList& parameter_list();
  FunctionBody& body();
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_PROGRAM_H
