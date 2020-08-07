#ifndef POLSERVER_PROGRAM_H
#define POLSERVER_PROGRAM_H

#include "Block.h"

namespace Pol::Bscript::Compiler
{
class ProgramParameterList;
class FunctionBody;
class Variable;

class Program : public Node
{
public:
  Program( const SourceLocation& source_location,
           std::unique_ptr<ProgramParameterList>, std::unique_ptr<FunctionBody> );
  ~Program();

  void accept( NodeVisitor& visitor ) override;
  void describe_to( fmt::Writer& ) const override;

  ProgramParameterList& parameter_list();
  FunctionBody& body();

  std::vector<std::shared_ptr<Variable>> debug_variables;
  unsigned locals_in_block;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_PROGRAM_H
