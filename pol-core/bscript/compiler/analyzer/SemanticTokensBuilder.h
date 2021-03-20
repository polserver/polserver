#ifndef POLSERVER_SEMANTICTOKENSBUILDER_H
#define POLSERVER_SEMANTICTOKENSBUILDER_H

#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/model/SemanticTokens.h"
#include <EscriptGrammar/EscriptParser.h>
#include <EscriptGrammar/EscriptParserBaseVisitor.h>
#include <initializer_list>
#include <vector>


namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Report;

class SemanticTokensBuilder : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  SemanticTokensBuilder( CompilerWorkspace& workspace, Report& );

  void build();

  antlrcpp::Any visitProgramDeclaration(
      EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx ) override;

private:
  CompilerWorkspace& workspace;
  Report& report;

  void annotate( antlr4::tree::TerminalNode* node, SemanticTokenType type,
                 std::initializer_list<SemanticTokenModifier> modifiers = {} );
};
}  // namespace Pol::Bscript::Compiler

#endif
