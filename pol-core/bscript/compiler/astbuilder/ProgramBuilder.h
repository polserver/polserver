#ifndef POLSERVER_PROGRAMBUILDER_H
#define POLSERVER_PROGRAMBUILDER_H

#include "bscript/compiler/astbuilder/CompoundStatementBuilder.h"

namespace Pol::Bscript::Compiler
{
class Program;

class ProgramBuilder : public CompoundStatementBuilder
{
public:
  ProgramBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<Program> program( EscriptGrammar::EscriptParser::ProgramDeclarationContext* );
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_PROGRAMBUILDER_H
