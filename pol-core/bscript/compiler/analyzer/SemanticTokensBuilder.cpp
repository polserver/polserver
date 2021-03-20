#include "SemanticTokensBuilder.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/TopLevelStatements.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

#include <iostream>

namespace Pol::Bscript::Compiler
{
SemanticTokensBuilder::SemanticTokensBuilder( CompilerWorkspace& workspace, Report& report )
    : workspace( workspace ), report( report )
{
}

void SemanticTokensBuilder::build()
{
  if ( workspace.source )
  {
    workspace.source->accept( *this );
  }
}

antlrcpp::Any SemanticTokensBuilder::visitProgramDeclaration(
    EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx )
{
  annotate( ctx->PROGRAM(), SemanticTokenType::KEYWORD );
  return antlrcpp::Any();
}

void SemanticTokensBuilder::annotate( antlr4::tree::TerminalNode* node, SemanticTokenType type,
                                      std::initializer_list<SemanticTokenModifier> modifiers )
{
  auto* sym = node->getSymbol();

  workspace.tokens.push_back( SemanticToken( { sym->getLine(), sym->getCharPositionInLine() + 1,
                                               sym->getText().length(), type, modifiers } ) );
}

}  // namespace Pol::Bscript::Compiler
