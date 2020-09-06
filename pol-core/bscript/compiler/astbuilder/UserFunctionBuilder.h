#ifndef POLSERVER_USERFUNCTIONBUILDER_H
#define POLSERVER_USERFUNCTIONBUILDER_H

#include "compiler/astbuilder/CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class UserFunction;

class UserFunctionBuilder : public CompoundStatementBuilder
{
public:
  UserFunctionBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<UserFunction> function_declaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_USERFUNCTIONBUILDER_H
