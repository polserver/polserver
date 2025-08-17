#include "PrettifyFileProcessor.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/file/PrettifyLineBuilder.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compilercfg.h"
#include "clib/filecont.h"
#include "clib/logfacility.h"

#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

/*
  //LineWidth
  FormatterLineWidth 100
  // keep original keyword spelling
  FormatterKeepKeywords 0
  // number of spaces for indent
  FormatterIndentLevel 2
  // multiple newlines get merged to a single
  FormatterMergeEmptyLines 1
  // space between emtpy parenthesis eg foo() vs foo( )
  FormatterEmptyParenthesisSpacing 0
  // space between emtpy brackets eg struct{} vs struct{ }
  FormatterEmptyBracketSpacing 0
  // space after/before parenthesis in conditionals
  // eg if ( true ) vs if (true)
  FormatterConditionalParenthesisSpacing 1
  // space after/before parenthesis
  // eg foo( true ) vs foo(true)
  FormatterParenthesisSpacing 1
  // space after/before brackets
  // eg array{ true } vs array{true}
  FormatterBracketSpacing 1
  // no space between type and opening brackect
  // eg struct{} vs struct {}
  FormatterBracketAttachToType 1
  // add space after delimiter comma or semi in for loops
  // eg {1, 2, 3} vs {1,2,3}
  FormatterDelimiterSpacing 1
  // add space around assignment
  // eg a := 1; vs a:=1;
  FormatterAssignmentSpacing 1
  // add space around comparison
  // eg a == 1 vs a==1
  FormatterComparisonSpacing 1
  // add space around operations
  // eg a + 1 vs a+1
  FormatterEllipsisSpacing 0
  // add space before ellipsis (...)
  // eg a... vs a ...
  FormatterOperatorSpacing 1
  // use \r\n as newline instead of \n
  FormatterWindowsLineEndings 0
  // use tabs instead of spaces
  FormatterUseTabs 0
  // tab width
  FormatterTabWidth 4
  // Insert a newline at end of file if missing
  FormatterInsertNewlineAtEOF 1
  // align trailing comments in a statement
  FormatterAlignTrailingComments 1
  // short case labels will be contracted to a single line
  FormatterAllowShortCaseLabelsOnASingleLine 1
  // short function references will be contracted to a single line
  FormatterAllowShortFuncRefsOnASingleLine 1
  // align short case statements needs FormatterAllowShortCaseLabelsOnASingleLine
  FormatterAlignConsecutiveShortCaseStatements 1
*/

namespace Pol::Bscript::Compiler
{
using namespace EscriptGrammar;

PrettifyFileProcessor::PrettifyFileProcessor( const SourceFileIdentifier& source_file_identifier,
                                              Profile& /*profile*/, Report& report )
    : source_file_identifier( source_file_identifier ) /*, profile( profile )*/, report( report )
{
}

antlrcpp::Any PrettifyFileProcessor::process_compilation_unit( SourceFile& sf )
{
  if ( auto compilation_unit = sf.get_compilation_unit( report, source_file_identifier ) )
  {
    if ( report.error_count() )
      return {};
    preprocess( sf );
    return compilation_unit->accept( this );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}

antlrcpp::Any PrettifyFileProcessor::process_module_unit( SourceFile& sf )
{
  if ( auto module_unit = sf.get_module_unit( report, source_file_identifier ) )
  {
    if ( report.error_count() )
      return {};
    preprocess( sf );
    return module_unit->accept( this );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}

antlrcpp::Any PrettifyFileProcessor::visitCompilationUnit(
    EscriptParser::CompilationUnitContext* ctx )
{
  visitChildren( ctx );
  if ( !linebuilder.finalize() )
    report.error( source_file_identifier, "left over formatting lines: {}",
                  linebuilder.currentTokens() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitModuleUnit( EscriptParser::ModuleUnitContext* ctx )
{
  visitChildren( ctx );
  if ( !linebuilder.finalize() )
    report.error( source_file_identifier, "left over formatting lines: {}",
                  linebuilder.currentTokens() );
  return {};
}


std::string PrettifyFileProcessor::prettify() const
{
  auto result =
      fmt::format( "{}", fmt::join( linebuilder.formattedLines(),
                                    compilercfg.FormatterWindowsLineEndings ? "\r\n" : "\n" ) );
  if ( result.empty() )
  {
    report.error( source_file_identifier, "formatting result is empty" );
    return {};
  }
  if ( compilercfg.FormatterInsertNewlineAtEOF && !result.empty() && result.back() != '\n' )
    result += compilercfg.FormatterWindowsLineEndings ? "\r\n" : "\n";
  return result;
}


antlrcpp::Any PrettifyFileProcessor::visitVarStatement( EscriptParser::VarStatementContext* ctx )
{
  _currentscope |= FmtToken::Scope::VAR;
  addToken( "var", ctx->VAR(), FmtToken::SPACE, FmtContext::VAR_STATEMENT );
  visitVariableDeclarationList( ctx->variableDeclarationList() );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  _currentscope &= ~FmtToken::Scope::VAR;
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitParExpression( EscriptParser::ParExpressionContext* ctx )
{
  addToken( "(", ctx->LPAREN(), linebuilder.openingParenthesisStyle() & ~FmtToken::ATTACHED );
  auto args = std::any_cast<bool>( visitExpression( ctx->expression() ) );
  addToken( ")", ctx->RPAREN(), linebuilder.closingParenthesisStyle( args ) );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBlock( EscriptParser::BlockContext* ctx )
{
  ++_currindent;
  visitChildren( ctx );
  --_currindent;
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitWhileStatement(
    EscriptParser::WhileStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "while", ctx->WHILE(), FmtToken::SPACE );
  visitParExpression( ctx->parExpression() );
  linebuilder.buildLine( _currindent );

  visitBlock( ctx->block() );
  addToken( "endwhile", ctx->ENDWHILE(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitVariableDeclarationList(
    EscriptParser::VariableDeclarationListContext* ctx )
{
  auto args = ctx->variableDeclaration();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    visitVariableDeclaration( args[i] );
    if ( i < args.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle(), FmtContext::VAR_COMMA );
  }
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitVariableDeclaration(
    EscriptParser::VariableDeclarationContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );

    if ( auto variable_declaration_initializer = ctx->variableDeclarationInitializer() )
    {
      _currentscope |= FmtToken::Scope::VAR;

      if ( auto assign = variable_declaration_initializer->ASSIGN() )
        addToken( ":=", assign, linebuilder.assignmentStyle() );

      if ( auto expression = variable_declaration_initializer->expression() )
        visitExpression( expression );
      else if ( auto arr = variable_declaration_initializer->ARRAY() )
        addToken( "array", arr, FmtToken::SPACE );
      _currentscope &= ~FmtToken::Scope::VAR;
    }
  }
  else if ( ctx->bindingDeclaration() )
  {
    _currentscope |= FmtToken::Scope::VAR;
    visitChildren( ctx );
    _currentscope &= ~FmtToken::Scope::VAR;
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBindingDeclaration(
    EscriptGrammar::EscriptParser::BindingDeclarationContext* ctx )
{
  // If the preceeding token is VAR or COMMA, we need to force unattached, ie:
  //   `var [foo]` (FormatterBracketSpacing=0), and
  //   `var [ foo ]` (FormatterBracketSpacing=1),
  // the LBRACK must not be attached.
  auto const& last_token = linebuilder.currentTokens().back();
  bool force_unattached =
      last_token.token_type == EscriptLexer::VAR || last_token.token_type == EscriptLexer::COMMA ||
      ( last_token.token_type == EscriptLexer::COLON && compilercfg.FormatterAssignmentSpacing );

  if ( auto lbrack = ctx->LBRACK() )
  {
    addToken( "[", lbrack, linebuilder.openingBracketStyle( false, force_unattached ) );
    ++_currentgroup;
    _currentscope |= FmtToken::Scope::ARRAY;
    size_t curcount = linebuilder.currentTokens().size();
    visitSequenceBindingList( ctx->sequenceBindingList() );
    _currentscope &= ~FmtToken::Scope::ARRAY;
    --_currentgroup;
    addToken( "]", ctx->RBRACK(),
              linebuilder.closingBracketStyle( curcount ) & ~FmtToken::BREAKPOINT );
  }
  else if ( auto lbrace = ctx->LBRACE() )
  {
    addToken( "{", lbrace, linebuilder.openingBracketStyle( false, true ) );
    ++_currentgroup;
    _currentscope |= FmtToken::Scope::DICT;
    size_t curcount = linebuilder.currentTokens().size();
    visitIndexBindingList( ctx->indexBindingList() );
    _currentscope &= ~FmtToken::Scope::DICT;
    --_currentgroup;
    addToken( "}", ctx->RBRACE(),
              linebuilder.closingBracketStyle( curcount ) & ~FmtToken::BREAKPOINT );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBindingDeclarationInitializer(
    EscriptGrammar::EscriptParser::BindingDeclarationInitializerContext* ctx )
{
  addToken( ":=", ctx->ASSIGN(), linebuilder.assignmentStyle() );
  visitExpression( ctx->expression() );

  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitIndexBindingList(
    EscriptGrammar::EscriptParser::IndexBindingListContext* ctx )
{
  auto args = ctx->indexBinding();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    visitIndexBinding( args[i] );

    if ( i < args.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitSequenceBindingList(
    EscriptGrammar::EscriptParser::SequenceBindingListContext* ctx )
{
  auto args = ctx->sequenceBinding();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    visitSequenceBinding( args[i] );
    if ( i < args.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitIndexBinding(
    EscriptGrammar::EscriptParser::IndexBindingContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );
  }

  if ( auto ellipsis = ctx->ELLIPSIS() )
  {
    addToken( "...", ellipsis, linebuilder.ellipsisStyle() );
  }
  else if ( auto expression = ctx->expression() )
  {
    // This corresponds to an expression index, eg the [foo] in `var { [foo]: bar } := baz;`, so
    // force it to be unattached if we have bracket spacing.
    addToken( "[", ctx->LBRACK(), FmtToken::NONE );
    visitExpression( expression );
    addToken( "]", ctx->RBRACK(), FmtToken::ATTACHED );
  }

  if ( auto binding = ctx->binding() )
  {
    visitBinding( binding );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBinding(
    EscriptGrammar::EscriptParser::BindingContext* ctx )
{
  // We always want it attached, eg. `foo:` (and not `foo :`) but we may not want a space after the
  // colon, eg.:
  //   `foo: bar` (FormatterAssignmentSpacing=1), vs
  //   `foo:bar` (FormatterAssignmentSpacing=0)
  addToken( ":", ctx->COLON(),
            FmtToken::ATTACHED |
                ( compilercfg.FormatterAssignmentSpacing ? FmtToken::SPACE : FmtToken::NONE ) );

  if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );
  }
  else if ( auto binding_decl = ctx->bindingDeclaration() )
  {
    visitBindingDeclaration( binding_decl );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitSequenceBinding(
    EscriptGrammar::EscriptParser::SequenceBindingContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );
    if ( auto ellipsis = ctx->ELLIPSIS() )
    {
      addToken( "...", ellipsis, linebuilder.ellipsisStyle() );
    }
  }
  else if ( auto binding_decl = ctx->bindingDeclaration() )
  {
    visitBindingDeclaration( binding_decl );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitConstStatement(
    EscriptParser::ConstStatementContext* ctx )
{
  addToken( "const", ctx->TOK_CONST(), FmtToken::SPACE );
  auto const_declaration_ctx = ctx->constantDeclaration();
  make_identifier( const_declaration_ctx->IDENTIFIER() );

  if ( auto variable_declaration_initializer =
           const_declaration_ctx->variableDeclarationInitializer() )
  {
    if ( auto assign = variable_declaration_initializer->ASSIGN() )
      addToken( ":=", assign, linebuilder.assignmentStyle() );
    if ( auto expression = variable_declaration_initializer->expression() )
      visitExpression( expression );
    else if ( auto array = variable_declaration_initializer->ARRAY() )
      addToken( "array", array, FmtToken::SPACE );
  }
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );

  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDictInitializerExpression(
    EscriptParser::DictInitializerExpressionContext* ctx )
{
  visitExpression( ctx->expression( 0 ) );  // key

  if ( ctx->ELLIPSIS() )
  {
    addToken( "...", ctx->ELLIPSIS(), linebuilder.ellipsisStyle() );
  }
  else
  {
    if ( ctx->expression().size() > 1 )
    {
      addToken( "->", ctx->ARROW(),
                linebuilder.delimiterStyle() & ~FmtToken::BREAKPOINT & ~FmtToken::ATTACHED );
      visitExpression( ctx->expression( 1 ) );
    }
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDoStatement( EscriptParser::DoStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "do", ctx->DO(), FmtToken::SPACE | FmtToken::BREAKPOINT );
  linebuilder.buildLine( _currindent );

  visitBlock( ctx->block() );

  addToken( "dowhile", ctx->DOWHILE(), FmtToken::SPACE );
  visitParExpression( ctx->parExpression() );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitEnumListEntry( EscriptParser::EnumListEntryContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), linebuilder.assignmentStyle() );
    visitExpression( expression );
  }
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitEnumList( EscriptParser::EnumListContext* ctx )

{
  auto enums = ctx->enumListEntry();
  for ( size_t i = 0; i < enums.size(); ++i )
  {
    visitEnumListEntry( enums[i] );
    if ( i < enums.size() - 1 )
      addToken( ",", ctx->COMMA( i ), FmtToken::SPACE | FmtToken::ATTACHED );

    linebuilder.buildLine( _currindent );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitEnumStatement( EscriptParser::EnumStatementContext* ctx )
{
  addToken( "enum", ctx->ENUM(), FmtToken::SPACE );
  if ( auto class_token = ctx->CLASS() )
  {
    addToken( "class", class_token, FmtToken::SPACE );
  }
  make_identifier( ctx->IDENTIFIER() );
  linebuilder.buildLine( _currindent );
  ++_currindent;
  auto enums = visitEnumList( ctx->enumList() );
  --_currindent;

  addToken( "endenum", ctx->ENDENUM(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExitStatement( EscriptParser::ExitStatementContext* ctx )
{
  addToken( "exit", ctx->EXIT(), FmtToken::SPACE );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitArrayInitializer(
    EscriptParser::ArrayInitializerContext* ctx )
{
  if ( auto lbrace = ctx->LBRACE() )
    addToken( "{", lbrace, linebuilder.openingBracketStyle( true ) );
  else if ( auto lparen = ctx->LPAREN() )
    addToken( "(", lparen, linebuilder.openingParenthesisStyle() );
  ++_currentgroup;
  size_t curcount = linebuilder.currentTokens().size();
  bool args = false;
  if ( auto expr = ctx->expressionList() )
    args = std::any_cast<bool>( visitExpressionList( expr ) );
  --_currentgroup;

  if ( auto rbrace = ctx->RBRACE() )
    addToken( "}", rbrace, linebuilder.closingBracketStyle( curcount ) );
  else if ( auto rparen = ctx->RPAREN() )
    addToken( ")", rparen, linebuilder.closingParenthesisStyle( args ) );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitArrayInitializer(
    EscriptParser::ExplicitArrayInitializerContext* ctx )
{
  ++_currentgroup;
  _currentscope |= FmtToken::Scope::ARRAY;
  addToken( "array", ctx->ARRAY(), FmtToken::SPACE );
  if ( auto init = ctx->arrayInitializer() )
    visitArrayInitializer( init );
  _currentscope &= ~FmtToken::Scope::ARRAY;
  --_currentgroup;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDictInitializerExpressionList(
    EscriptParser::DictInitializerExpressionListContext* ctx )
{
  auto inits = ctx->dictInitializerExpression();
  ++_currentgroup;
  for ( size_t i = 0; i < inits.size(); ++i )
  {
    visitDictInitializerExpression( inits[i] );
    if ( i < inits.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() );
  }
  --_currentgroup;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDictInitializer(
    EscriptParser::DictInitializerContext* ctx )
{
  addToken( "{", ctx->LBRACE(), linebuilder.openingBracketStyle( true ) );
  size_t curcount = linebuilder.currentTokens().size();
  if ( auto expr = ctx->dictInitializerExpressionList() )
    visitDictInitializerExpressionList( expr );

  addToken( "}", ctx->RBRACE(), linebuilder.closingBracketStyle( curcount ) );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitDictInitializer(
    EscriptParser::ExplicitDictInitializerContext* ctx )
{
  ++_currentgroup;
  _currentscope |= FmtToken::Scope::DICT;
  addToken( "dictionary", ctx->DICTIONARY(), FmtToken::SPACE );
  if ( auto init = ctx->dictInitializer() )
    visitDictInitializer( init );
  _currentscope &= ~FmtToken::Scope::DICT;
  --_currentgroup;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStructInitializerExpressionList(
    EscriptParser::StructInitializerExpressionListContext* ctx )
{
  auto inits = ctx->structInitializerExpression();
  for ( size_t i = 0; i < inits.size(); ++i )
  {
    visitStructInitializerExpression( inits[i] );
    if ( i < inits.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStructInitializer(
    EscriptParser::StructInitializerContext* ctx )
{
  addToken( "{", ctx->LBRACE(), linebuilder.openingBracketStyle( true ) );
  ++_currentgroup;
  size_t curcount = linebuilder.currentTokens().size();
  if ( auto expr = ctx->structInitializerExpressionList() )
    visitStructInitializerExpressionList( expr );
  --_currentgroup;

  addToken( "}", ctx->RBRACE(), linebuilder.closingBracketStyle( curcount ) );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitErrorInitializer(
    EscriptParser::ExplicitErrorInitializerContext* ctx )
{
  addToken( "error", ctx->TOK_ERROR(), FmtToken::SPACE );
  if ( auto init = ctx->structInitializer() )
    visitStructInitializer( init );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBareArrayInitializer(
    EscriptParser::BareArrayInitializerContext* ctx )
{
  _currentscope |= FmtToken::Scope::ARRAY;
  addToken( "{", ctx->LBRACE(), linebuilder.openingBracketStyle() & ~FmtToken::ATTACHED );
  ++_currentgroup;
  size_t curcount = linebuilder.currentTokens().size();
  if ( auto expr = ctx->expressionList() )
    visitExpressionList( expr );
  --_currentgroup;
  addToken( "}", ctx->RBRACE(), linebuilder.closingBracketStyle( curcount ) );
  _currentscope &= ~FmtToken::Scope::ARRAY;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitStructInitializer(
    EscriptParser::ExplicitStructInitializerContext* ctx )
{
  ++_currentgroup;
  _currentscope |= FmtToken::Scope::STRUCT;
  addToken( "struct", ctx->STRUCT(), FmtToken::SPACE );
  if ( auto init = ctx->structInitializer() )
    visitStructInitializer( init );
  _currentscope &= ~FmtToken::Scope::STRUCT;
  --_currentgroup;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExpressionList(
    EscriptParser::ExpressionListContext* ctx )
{
  auto args = ctx->expressionListEntry();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    visitExpression( args[i]->expression() );

    if ( args[i]->ELLIPSIS() )
      addToken( "...", args[i]->ELLIPSIS(), linebuilder.ellipsisStyle() );

    if ( i < args.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() | FmtToken::PREFERRED_BREAK );
  }
  return !args.empty();
}

antlrcpp::Any PrettifyFileProcessor::visitIndexList( EscriptParser::IndexListContext* ctx )
{
  auto args = ctx->expression();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    visitExpression( args[i] );
    if ( i < args.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() | FmtToken::PREFERRED_BREAK );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExpression( EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
  {
    visitPrimary( prim );
    return true;
  }
  else if ( ctx->prefix )
  {
    addToken( ctx->prefix->getText(), ctx->prefix,
              ctx->prefix->getType() == EscriptLexer::BANG_B ? FmtToken::SPACE : FmtToken::NONE );
    visitExpression( ctx->expression( 0 ) );
    return true;
  }
  else if ( ctx->postfix )
  {
    visitExpression( ctx->expression( 0 ) );
    addToken( ctx->postfix->getText(), ctx->postfix, FmtToken::SPACE | FmtToken::ATTACHED );
    return true;
  }
  else if ( ctx->bop && ctx->expression().size() == 2 )
  {
    visitExpression( ctx->expression( 0 ) );  // left
    int style = FmtToken::NONE;
    switch ( ctx->bop->getType() )
    {
    case EscriptLexer::OR_A:
    case EscriptLexer::OR_B:
    case EscriptLexer::AND_A:
    case EscriptLexer::AND_B:
      style = FmtToken::SPACE | FmtToken::BREAKPOINT | FmtToken::PREFERRED_BREAK_LOGICAL;
      break;
    case EscriptLexer::ADDMEMBER:
    case EscriptLexer::DELMEMBER:
    case EscriptLexer::CHKMEMBER:
      style = FmtToken::ATTACHED;
      break;
    case EscriptLexer::ASSIGN:
    case EscriptLexer::ADD_ASSIGN:
    case EscriptLexer::SUB_ASSIGN:
    case EscriptLexer::MUL_ASSIGN:
    case EscriptLexer::DIV_ASSIGN:
    case EscriptLexer::MOD_ASSIGN:
      _currentscope |= FmtToken::Scope::VAR;  // matches with visitStatement
      style = linebuilder.assignmentStyle();
      break;
    case EscriptLexer::TOK_IN:
      style = FmtToken::SPACE;
      break;
    case EscriptLexer::LE:
    case EscriptLexer::GE:
    case EscriptLexer::GT:
    case EscriptLexer::LT:
    case EscriptLexer::EQUAL:
    case EscriptLexer::NOTEQUAL_A:
    case EscriptLexer::NOTEQUAL_B:
      style = linebuilder.comparisonStyle();
      break;
    case EscriptLexer::ADD:
    case EscriptLexer::SUB:
    case EscriptLexer::MUL:
    case EscriptLexer::DIV:
    case EscriptLexer::MOD:
    case EscriptLexer::LSHIFT:
    case EscriptLexer::RSHIFT:
    case EscriptLexer::BITAND:
    case EscriptLexer::BITOR:
      style = linebuilder.operatorStyle();
      break;
    case EscriptLexer::ELVIS:
    {
      // TODO before it sets breakpoint before
      // if really needed should be a flag like attached
      style = FmtToken::SPACE | FmtToken::BREAKPOINT;
      break;
    }
    default:
      style = FmtToken::SPACE;
    }
    addToken( ctx->bop->getText(), ctx->bop, style );

    visitExpression( ctx->expression( 1 ) );  // right
    return true;
  }
  else if ( auto suffix = ctx->expressionSuffix() )
  {
    expression_suffix( ctx->expression( 0 ), suffix );
    return true;
  }
  else if ( ctx->QUESTION() )
  {
    visitExpression( ctx->expression( 0 ) );  // conditional
    addToken( "?", ctx->QUESTION(), FmtToken::SPACE | FmtToken::BREAKPOINT );
    visitExpression( ctx->expression( 1 ) );  // consequent
    addToken( ":", ctx->COLON(), FmtToken::SPACE | FmtToken::BREAKPOINT );

    visitExpression( ctx->expression( 2 ) );  // alternate
    return true;
  }

  return false;
}

antlrcpp::Any PrettifyFileProcessor::expression_suffix(
    EscriptParser::ExpressionContext* expr_ctx,
    EscriptParser::ExpressionSuffixContext* expr_suffix_ctx )
{
  if ( auto indexing = expr_suffix_ctx->indexingSuffix() )
  {
    visitExpression( expr_ctx );
    addToken( "[", indexing->LBRACK(), FmtToken::ATTACHED );
    visitIndexList( indexing->indexList() );
    addToken( "]", indexing->RBRACK(), FmtToken::SPACE | FmtToken::ATTACHED );
  }
  else if ( auto member = expr_suffix_ctx->navigationSuffix() )
  {
    visitExpression( expr_ctx );
    addToken( ".", member->DOT(), FmtToken::ATTACHED );
    if ( auto string_literal = member->STRING_LITERAL() )
      make_string_literal( string_literal );
    else if ( auto identifier = member->IDENTIFIER() )
      make_identifier( identifier );
    else if ( auto function_keyword = member->FUNCTION() )
      make_identifier( function_keyword );
  }
  else if ( auto method = expr_suffix_ctx->methodCallSuffix() )
  {
    visitExpression( expr_ctx );
    addToken( ".", method->DOT(), FmtToken::ATTACHED );
    make_identifier( method->IDENTIFIER() );
    addToken( "(", method->LPAREN(), linebuilder.openingParenthesisStyle() );
    bool args = false;
    if ( auto expr = method->expressionList() )
      args = std::any_cast<bool>( visitExpressionList( expr ) );
    addToken( ")", method->RPAREN(), linebuilder.closingParenthesisStyle( args ) );
  }
  else if ( auto function_call = expr_suffix_ctx->functionCallSuffix() )
  {
    visitExpression( expr_ctx );
    addToken( "(", function_call->LPAREN(), linebuilder.openingParenthesisStyle() );
    bool args = false;
    if ( auto expr = function_call->expressionList() )
      args = std::any_cast<bool>( visitExpressionList( expr ) );
    addToken( ")", function_call->RPAREN(), linebuilder.closingParenthesisStyle( args ) );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFloatLiteral( EscriptParser::FloatLiteralContext* ctx )
{
  if ( auto float_literal = ctx->FLOAT_LITERAL() )
    return make_float_literal( float_literal );
  else if ( auto hex_float_literal = ctx->HEX_FLOAT_LITERAL() )
    return make_float_literal( hex_float_literal );

  return visitChildren( ctx );
}

antlrcpp::Any PrettifyFileProcessor::visitForeachIterableExpression(
    EscriptParser::ForeachIterableExpressionContext* ctx )
{
  if ( auto parExpression = ctx->parExpression() )
  {
    visitParExpression( parExpression );
  }
  else if ( auto functionCall = ctx->functionCall() )
  {
    visitFunctionCall( functionCall );
  }
  else if ( auto scopedIdentifier = ctx->scopedIdentifier() )
  {
    visitScopedIdentifier( scopedIdentifier );
  }
  else if ( auto scopedFunctionCall = ctx->scopedFunctionCall() )
  {
    visitScopedFunctionCall( scopedFunctionCall );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );
  }
  else if ( auto explicitArrayInitializer = ctx->explicitArrayInitializer() )
  {
    visitExplicitArrayInitializer( explicitArrayInitializer );
  }
  else if ( auto bareArrayInitializer = ctx->bareArrayInitializer() )
  {
    visitBareArrayInitializer( bareArrayInitializer );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitForeachStatement(
    EscriptParser::ForeachStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "foreach", ctx->FOREACH(), FmtToken::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  addToken( "in", ctx->TOK_IN(), FmtToken::SPACE );
  visitForeachIterableExpression( ctx->foreachIterableExpression() );
  linebuilder.buildLine( _currindent );

  visitBlock( ctx->block() );

  addToken( "endforeach", ctx->ENDFOREACH(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitForStatement( EscriptParser::ForStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );

  auto forGroup = ctx->forGroup();

  addToken( "for", ctx->FOR(), FmtToken::SPACE );
  if ( auto basicForStatement = forGroup->basicForStatement() )
    visitBasicForStatement( basicForStatement );
  else if ( auto cstyleForStatement = forGroup->cstyleForStatement() )
    visitCstyleForStatement( cstyleForStatement );
  addToken( "endfor", ctx->ENDFOR(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionCall( EscriptParser::FunctionCallContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );
  _currentscope |= FmtToken::Scope::FUNCTION;

  addToken( "(", ctx->LPAREN(), linebuilder.openingParenthesisStyle(),
            FmtContext::PREFERRED_BREAK_START );

  bool argcount = false;
  if ( auto args = ctx->expressionList() )
    argcount = std::any_cast<bool>( visitExpressionList( args ) );

  addToken( ")", ctx->RPAREN(), linebuilder.closingParenthesisStyle( argcount ),
            FmtContext::PREFERRED_BREAK_END );
  _currentscope &= ~FmtToken::Scope::FUNCTION;
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitFunctionDeclaration(
    EscriptParser::FunctionDeclarationContext* ctx )
{
  if ( auto exported = ctx->EXPORTED() )
    addToken( "exported", exported, FmtToken::SPACE );
  addToken( "function", ctx->FUNCTION(), FmtToken::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  visitFunctionParameters( ctx->functionParameters() );

  visitBlock( ctx->block() );

  addToken( "endfunction", ctx->ENDFUNCTION(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionParameters(
    EscriptParser::FunctionParametersContext* ctx )
{
  addToken( "(", ctx->LPAREN(), linebuilder.openingParenthesisStyle(),
            FmtContext::PREFERRED_BREAK_START );

  bool argcount = false;
  if ( auto args = ctx->functionParameterList() )
    argcount = std::any_cast<bool>( visitFunctionParameterList( args ) );

  auto closingstyle = linebuilder.closingParenthesisStyle( argcount );
  if ( _suppressnewline )  // add space if newline is suppressed
    closingstyle |= FmtToken::SPACE;
  addToken( ")", ctx->RPAREN(), closingstyle, FmtContext::PREFERRED_BREAK_END );
  if ( !_suppressnewline )
    linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionParameterList(
    EscriptParser::FunctionParameterListContext* ctx )
{
  auto params = ctx->functionParameter();
  for ( size_t i = 0; i < params.size(); ++i )
  {
    visitFunctionParameter( params[i] );
    if ( i < params.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() | FmtToken::PREFERRED_BREAK );
  }
  return !params.empty();
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionParameter(
    EscriptParser::FunctionParameterContext* ctx )
{
  if ( auto byref = ctx->BYREF() )
    addToken( "byref", byref, FmtToken::SPACE );
  if ( auto unused = ctx->UNUSED() )
    addToken( "unused", unused, FmtToken::SPACE );
  make_identifier( ctx->IDENTIFIER() );

  if ( ctx->ELLIPSIS() )
    addToken( "...", ctx->ELLIPSIS(), linebuilder.ellipsisStyle() );

  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), linebuilder.assignmentStyle() );
    visitExpression( expression );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBreakStatement(
    EscriptParser::BreakStatementContext* ctx )
{
  addToken( "break", ctx->BREAK(), FmtToken::SPACE );
  if ( auto identifier = ctx->IDENTIFIER() )
    make_identifier( identifier );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitSwitchBlockStatementGroup(
    EscriptParser::SwitchBlockStatementGroupContext* ctx )
{
  if ( ctx->switchLabel().size() == 1 )
    if ( compilercfg.FormatterAllowShortCaseLabelsOnASingleLine )
      linebuilder.markPackableLineStart();
  for ( const auto& switchLabel : ctx->switchLabel() )
  {
    visitSwitchLabel( switchLabel );
    linebuilder.buildLine( _currindent );
  }
  visitBlock( ctx->block() );
  if ( compilercfg.FormatterAllowShortCaseLabelsOnASingleLine )
    linebuilder.markPackableLineEnd();
  if ( ctx->switchLabel().size() == 1 )
    linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitSwitchLabel( EscriptParser::SwitchLabelContext* ctx )
{
  if ( auto integerLiteral = ctx->integerLiteral() )
    visitIntegerLiteral( integerLiteral );
  else if ( auto boolLiteral = ctx->boolLiteral() )
    visitBoolLiteral( boolLiteral );
  else if ( auto uninit = ctx->UNINIT() )
    addToken( "uninit", uninit, FmtToken::SPACE );
  else if ( auto identifier = ctx->IDENTIFIER() )
    make_identifier( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    make_string_literal( string_literal );
  else if ( auto defaultctx = ctx->DEFAULT() )
    addToken( "default", defaultctx, FmtToken::SPACE );

  addToken( ":", ctx->COLON(), FmtToken::SPACE | FmtToken::ATTACHED | FmtToken::BREAKPOINT );
  linebuilder.markLastTokensAsSwitchLabel();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitCaseStatement( EscriptParser::CaseStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "case", ctx->CASE(), FmtToken::SPACE | FmtToken::BREAKPOINT );
  addToken( "(", ctx->LPAREN(), linebuilder.openingParenthesisStyle() & ~FmtToken::ATTACHED );
  auto args = std::any_cast<bool>( visitExpression( ctx->expression() ) );
  addToken( ")", ctx->RPAREN(), linebuilder.closingParenthesisStyle( args ) );
  linebuilder.buildLine( _currindent );
  ++_currindent;
  size_t start = linebuilder.formattedLines().size();
  for ( const auto& switchBlockStatementGroup : ctx->switchBlockStatementGroup() )
  {
    visitSwitchBlockStatementGroup( switchBlockStatementGroup );
  }
  --_currindent;
  linebuilder.alignSingleLineSwitchStatements( start );
  addToken( "endcase", ctx->ENDCASE(), FmtToken::SPACE | FmtToken::BREAKPOINT );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitClassBody(
    EscriptGrammar::EscriptParser::ClassBodyContext* ctx )
{
  ++_currindent;
  visitChildren( ctx );
  --_currindent;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitClassDeclaration(
    EscriptGrammar::EscriptParser::ClassDeclarationContext* ctx )
{
  addToken( "class", ctx->CLASS(), FmtToken::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  visitClassParameters( ctx->classParameters() );

  visitClassBody( ctx->classBody() );

  addToken( "endclass", ctx->ENDCLASS(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitClassParameters(
    EscriptGrammar::EscriptParser::ClassParametersContext* ctx )
{
  addToken( "(", ctx->LPAREN(), linebuilder.openingParenthesisStyle(),
            FmtContext::PREFERRED_BREAK_START );

  bool argcount = false;
  if ( auto args = ctx->classParameterList() )
    argcount = std::any_cast<bool>( visitClassParameterList( args ) );

  auto closingstyle = linebuilder.closingParenthesisStyle( argcount );
  if ( _suppressnewline )  // add space if newline is suppressed
    closingstyle |= FmtToken::SPACE;
  addToken( ")", ctx->RPAREN(), closingstyle, FmtContext::PREFERRED_BREAK_END );
  if ( !_suppressnewline )
    linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitClassParameterList(
    EscriptGrammar::EscriptParser::ClassParameterListContext* ctx )
{
  auto params = ctx->IDENTIFIER();
  for ( size_t i = 0; i < params.size(); ++i )
  {
    make_identifier( params[i] );
    if ( i < params.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() | FmtToken::PREFERRED_BREAK );
  }

  return !params.empty();
}

antlrcpp::Any PrettifyFileProcessor::visitContinueStatement(
    EscriptParser::ContinueStatementContext* ctx )
{
  addToken( "continue", ctx->CONTINUE(), FmtToken::SPACE );

  if ( auto identifier = ctx->IDENTIFIER() )
    make_identifier( identifier );

  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBasicForStatement(
    EscriptParser::BasicForStatementContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );
  addToken( ":=", ctx->ASSIGN(), linebuilder.assignmentStyle() );
  visitExpression( ctx->expression( 0 ) );
  addToken( "to", ctx->TO(), FmtToken::SPACE );
  visitExpression( ctx->expression( 1 ) );
  linebuilder.buildLine( _currindent );

  visitBlock( ctx->block() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitCstyleForStatement(
    EscriptParser::CstyleForStatementContext* ctx )
{
  addToken( "(", ctx->LPAREN(), linebuilder.openingParenthesisStyle() & ~FmtToken::ATTACHED );
  visitExpression( ctx->expression( 0 ) );
  addToken( ";", ctx->SEMI( 0 ), linebuilder.delimiterStyle() );
  visitExpression( ctx->expression( 1 ) );
  addToken( ";", ctx->SEMI( 1 ), linebuilder.delimiterStyle() );
  visitExpression( ctx->expression( 2 ) );
  addToken( ")", ctx->RPAREN(), linebuilder.closingParenthesisStyle( true ) );
  linebuilder.buildLine( _currindent );

  visitBlock( ctx->block() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitRepeatStatement(
    EscriptParser::RepeatStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "repeat", ctx->REPEAT(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );

  visitBlock( ctx->block() );

  addToken( "until", ctx->UNTIL(), FmtToken::SPACE );
  visitExpression( ctx->expression() );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitReturnStatement(
    EscriptParser::ReturnStatementContext* ctx )
{
  addToken( "return", ctx->RETURN(), FmtToken::SPACE );
  if ( auto expression = ctx->expression() )
    visitExpression( expression );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitScopedFunctionCall(
    EscriptParser::ScopedFunctionCallContext* ctx )
{
  if ( ctx->IDENTIFIER() )
    make_identifier( ctx->IDENTIFIER() );  // scope
  addToken( "::", ctx->COLONCOLON(), FmtToken::ATTACHED );
  visitFunctionCall( ctx->functionCall() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitScopedIdentifier(
    EscriptGrammar::EscriptParser::ScopedIdentifierContext* ctx )
{
  if ( ctx->scope )
    addToken( ctx->scope->getText(), ctx->scope, FmtToken::NONE );
  addToken( "::", ctx->COLONCOLON(), FmtToken::ATTACHED );
  addToken( ctx->identifier->getText(), ctx->identifier, FmtToken::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionReference(
    EscriptParser::FunctionReferenceContext* ctx )
{
  addToken( "@", ctx->AT(), FmtToken::NONE );

  if ( ctx->scope )
  {
    addToken( ctx->scope->getText(), ctx->scope, FmtToken::NONE );
    addToken( "::", ctx->COLONCOLON(), FmtToken::ATTACHED );
  }

  addToken( ctx->function->getText(), ctx->function, FmtToken::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionExpression(
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  addToken( "@", ctx->AT(), FmtToken::NONE );


  _suppressnewline = true;  // functionparams force newline
  if ( auto params = ctx->functionParameters() )
    visitFunctionParameters( params );
  _suppressnewline = false;

  // we want the starting { at the same line and not attached
  addToken( "{", ctx->LBRACE(), linebuilder.openingBracketStyle() & ~FmtToken::ATTACHED );
  if ( compilercfg.FormatterAllowShortFuncRefsOnASingleLine )
    linebuilder.markPackableLineStart();
  linebuilder.buildLine( _currindent );  // now start a newline

  ++_currentgroup;
  size_t curcount = linebuilder.currentTokens().size();
  visitBlock( ctx->block() );
  --_currentgroup;

  addToken( "}", ctx->RBRACE(), linebuilder.closingBracketStyle( curcount ) );
  if ( compilercfg.FormatterAllowShortFuncRefsOnASingleLine )
    linebuilder.markPackableLineEnd();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitGotoStatement( EscriptParser::GotoStatementContext* ctx )
{
  addToken( "goto", ctx->GOTO(), FmtToken::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitIfStatement( EscriptParser::IfStatementContext* ctx )
{
  auto blocks = ctx->block();
  auto par_expression = ctx->parExpression();
  size_t num_expressions = par_expression.size();
  for ( size_t clause_index = 0; clause_index < num_expressions; ++clause_index )
  {
    auto expression_ctx = par_expression.at( clause_index );
    if ( !clause_index )
      addToken( "if", ctx->IF(), FmtToken::SPACE );
    else
      addToken( "elseif", ctx->ELSEIF( clause_index - 1 ), FmtToken::SPACE );
    auto expression_ast = visitParExpression( expression_ctx );
    if ( !clause_index && ctx->THEN() )
      addToken( "then", ctx->THEN(), FmtToken::SPACE );

    linebuilder.buildLine( _currindent );

    if ( blocks.size() > clause_index )
      visitBlock( blocks.at( clause_index ) );
  }
  if ( ctx->ELSE() )
  {
    addToken( "else", ctx->ELSE(), FmtToken::SPACE );
    linebuilder.buildLine( _currindent );
    visitBlock( blocks.at( blocks.size() - 1 ) );
  }
  addToken( "endif", ctx->ENDIF(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitIncludeDeclaration(
    EscriptParser::IncludeDeclarationContext* ctx )
{
  addToken( "include", ctx->INCLUDE(), FmtToken::SPACE );
  visitStringIdentifier( ctx->stringIdentifier() );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitInterpolatedString(
    EscriptParser::InterpolatedStringContext* ctx )
{
  addToken( "$\"", ctx->INTERPOLATED_STRING_START(), FmtToken::SPACE );
  visitChildren( ctx );
  addToken( "\"", ctx->DOUBLE_QUOTE_INSIDE(), FmtToken::ATTACHED | FmtToken::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitInterpolatedStringPart(
    EscriptParser::InterpolatedStringPartContext* ctx )
{
  if ( auto expression_ctx = ctx->expression() )
  {
    addToken( "{", ctx->LBRACE_INSIDE(), FmtToken::ATTACHED );
    visitExpression( expression_ctx );

    if ( auto format_string = ctx->FORMAT_STRING() )
    {
      addToken( ":", ctx->COLON(), FmtToken::ATTACHED );
      addToken( format_string->getText(), format_string, FmtToken::ATTACHED );
    }
    // TODO: this seems to be the only non token
    Range r( *expression_ctx );
    addToken( "}", r.end, FmtToken::ATTACHED, 0 );
  }

  else if ( auto string_literal = ctx->STRING_LITERAL_INSIDE() )
  {
    addToken( string_literal->getText(), string_literal, FmtToken::ATTACHED );
  }
  else if ( auto lbrace = ctx->DOUBLE_LBRACE_INSIDE() )
  {
    addToken( "{{", lbrace, FmtToken::ATTACHED );
  }
  else if ( auto rbrace = ctx->DOUBLE_RBRACE() )
  {
    addToken( "}}", rbrace, FmtToken::ATTACHED );
  }
  else if ( auto escaped = ctx->REGULAR_CHAR_INSIDE() )
  {
    addToken( escaped->getText(), escaped, FmtToken::ATTACHED );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBoolLiteral( EscriptParser::BoolLiteralContext* ctx )
{
  if ( auto bool_false = ctx->BOOL_FALSE() )
    make_bool_literal( bool_false );
  else if ( auto bool_true = ctx->BOOL_TRUE() )
    make_bool_literal( bool_true );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitIntegerLiteral(
    EscriptParser::IntegerLiteralContext* ctx )
{
  if ( auto decimal_literal = ctx->DECIMAL_LITERAL() )
    return make_integer_literal( decimal_literal );

  else if ( auto hex_literal = ctx->HEX_LITERAL() )
    return make_integer_literal( hex_literal );

  else if ( auto oct_literal = ctx->OCT_LITERAL() )
    return make_integer_literal( oct_literal );

  else if ( auto binary_literal = ctx->BINARY_LITERAL() )
    return make_integer_literal( binary_literal );

  return visitChildren( ctx );
}

antlrcpp::Any PrettifyFileProcessor::visitLiteral( EscriptParser::LiteralContext* ctx )
{
  if ( auto string_literal = ctx->STRING_LITERAL() )
    return make_string_literal( string_literal );
  else if ( auto integer_literal = ctx->integerLiteral() )
    return visitIntegerLiteral( integer_literal );
  else if ( auto float_literal = ctx->floatLiteral() )
    return visitFloatLiteral( float_literal );
  else if ( auto uninit = ctx->UNINIT() )
    addToken( "uninit", uninit, FmtToken::SPACE );
  else if ( auto bool_literal = ctx->boolLiteral() )
    return visitBoolLiteral( bool_literal );
  return visitChildren( ctx );
}

antlrcpp::Any PrettifyFileProcessor::visitModuleFunctionDeclaration(
    EscriptParser::ModuleFunctionDeclarationContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );
  addToken( "(", ctx->LPAREN(), linebuilder.openingParenthesisStyle() );
  bool args = false;
  if ( auto moduleFunctionParameterList = ctx->moduleFunctionParameterList() )
    args = std::any_cast<bool>( visitModuleFunctionParameterList( moduleFunctionParameterList ) );

  addToken( ")", ctx->RPAREN(), linebuilder.closingParenthesisStyle( args ) );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitModuleFunctionParameterList(
    EscriptParser::ModuleFunctionParameterListContext* ctx )
{
  auto params = ctx->moduleFunctionParameter();
  for ( size_t i = 0; i < params.size(); ++i )
  {
    visitModuleFunctionParameter( params[i] );
    if ( i < params.size() - 1 )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() | FmtToken::PREFERRED_BREAK );
  }
  return !params.empty();
}

antlrcpp::Any PrettifyFileProcessor::visitModuleFunctionParameter(
    EscriptParser::ModuleFunctionParameterContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), linebuilder.assignmentStyle() );
    visitExpression( expression );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitPrimary( EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
    return visitLiteral( literal );
  else if ( auto parExpression = ctx->parExpression() )
    return visitParExpression( parExpression );
  else if ( auto functionCall = ctx->functionCall() )
    return visitFunctionCall( functionCall );
  else if ( auto scopedIdentifier = ctx->scopedIdentifier() )
    return visitScopedIdentifier( scopedIdentifier );
  else if ( auto scopedFunctionCall = ctx->scopedFunctionCall() )
    return visitScopedFunctionCall( scopedFunctionCall );
  else if ( auto identifier = ctx->IDENTIFIER() )
    return make_identifier( identifier );
  else if ( auto functionReference = ctx->functionReference() )
    return visitFunctionReference( functionReference );
  else if ( auto functionExpression = ctx->functionExpression() )
    return visitFunctionExpression( functionExpression );
  else if ( auto explicitArrayInitializer = ctx->explicitArrayInitializer() )
    return visitExplicitArrayInitializer( explicitArrayInitializer );
  else if ( auto explicitStructInitializer = ctx->explicitStructInitializer() )
    return visitExplicitStructInitializer( explicitStructInitializer );
  else if ( auto explicitDictInitializer = ctx->explicitDictInitializer() )
    return visitExplicitDictInitializer( explicitDictInitializer );
  else if ( auto explicitErrorInitializer = ctx->explicitErrorInitializer() )
    return visitExplicitErrorInitializer( explicitErrorInitializer );
  else if ( auto bareArrayInitializer = ctx->bareArrayInitializer() )
    return visitBareArrayInitializer( bareArrayInitializer );
  else if ( auto interpolatedString = ctx->interpolatedString() )
    return visitInterpolatedString( interpolatedString );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitProgramDeclaration(
    EscriptParser::ProgramDeclarationContext* ctx )
{
  addToken( "program", ctx->PROGRAM(), FmtToken::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  visitProgramParameters( ctx->programParameters() );
  linebuilder.buildLine( _currindent );
  visitBlock( ctx->block() );
  addToken( "endprogram", ctx->ENDPROGRAM(), FmtToken::SPACE );
  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitProgramParameterList(
    EscriptParser::ProgramParameterListContext* ctx )
{
  auto params = ctx->programParameter();
  for ( size_t i = 0; i < params.size(); ++i )
  {
    visitProgramParameter( params[i] );
    // comma is optional here
    if ( i < params.size() - 1 && ctx->COMMA( i ) )
      addToken( ",", ctx->COMMA( i ), linebuilder.delimiterStyle() | FmtToken::PREFERRED_BREAK );
  }
  return !params.empty();
}

antlrcpp::Any PrettifyFileProcessor::visitProgramParameters(
    EscriptParser::ProgramParametersContext* ctx )
{
  addToken( "(", ctx->LPAREN(), linebuilder.openingParenthesisStyle() );

  bool args = false;
  if ( auto programParameterList = ctx->programParameterList() )
    args = std::any_cast<bool>( visitProgramParameterList( programParameterList ) );

  addToken( ")", ctx->RPAREN(), linebuilder.closingParenthesisStyle( args ) );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitProgramParameter(
    EscriptParser::ProgramParameterContext* ctx )
{
  if ( auto unused = ctx->UNUSED() )
    addToken( "unused", unused, FmtToken::SPACE );

  make_identifier( ctx->IDENTIFIER() );
  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), linebuilder.assignmentStyle() );
    visitExpression( expression );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStatement( EscriptParser::StatementContext* ctx )
{
  if ( auto ifStatement = ctx->ifStatement() )
    return visitIfStatement( ifStatement );
  else if ( auto gotoStatement = ctx->gotoStatement() )
    return visitGotoStatement( gotoStatement );
  else if ( auto returnStatement = ctx->returnStatement() )
    return visitReturnStatement( returnStatement );
  else if ( auto constStatement = ctx->constStatement() )
    return visitConstStatement( constStatement );
  else if ( auto varStatement = ctx->varStatement() )
    return visitVarStatement( varStatement );
  else if ( auto doStatement = ctx->doStatement() )
    return visitDoStatement( doStatement );
  else if ( auto whileStatement = ctx->whileStatement() )
    return visitWhileStatement( whileStatement );
  else if ( auto exitStatement = ctx->exitStatement() )
    return visitExitStatement( exitStatement );
  else if ( auto breakStatement = ctx->breakStatement() )
    return visitBreakStatement( breakStatement );
  else if ( auto continueStatement = ctx->continueStatement() )
    return visitContinueStatement( continueStatement );
  else if ( auto forStatement = ctx->forStatement() )
    return visitForStatement( forStatement );
  else if ( auto foreachStatement = ctx->foreachStatement() )
    return visitForeachStatement( foreachStatement );
  else if ( auto repeatStatement = ctx->repeatStatement() )
    return visitRepeatStatement( repeatStatement );
  else if ( auto caseStatement = ctx->caseStatement() )
    return visitCaseStatement( caseStatement );
  else if ( auto enumStatement = ctx->enumStatement() )
    return visitEnumStatement( enumStatement );
  else if ( auto expression = ctx->statementExpression )
  {
    visitExpression( expression );

    addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );
    _currentscope &= ~FmtToken::Scope::VAR;  // matches with assignment inside of visitExpression
    linebuilder.buildLine( _currindent );
    return {};
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStringIdentifier(
    EscriptParser::StringIdentifierContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
    return make_identifier( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    return make_string_literal( string_literal );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStructInitializerExpression(
    EscriptParser::StructInitializerExpressionContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
    make_identifier( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    make_string_literal( string_literal );

  if ( auto expression = ctx->expression() )
  {
    if ( auto assign = ctx->ASSIGN() )
      addToken( ":=", assign, linebuilder.assignmentStyle() );

    visitExpression( expression );

    if ( ctx->ELLIPSIS() )
    {
      addToken( "...", ctx->ELLIPSIS(), linebuilder.ellipsisStyle() );
    }
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitUseDeclaration(
    EscriptParser::UseDeclarationContext* ctx )
{
  addToken( "use", ctx->USE(), FmtToken::SPACE );
  visitStringIdentifier( ctx->stringIdentifier() );
  addToken( ";", ctx->SEMI(), linebuilder.terminatorStyle() );

  linebuilder.buildLine( _currindent );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_statement_label(
    EscriptParser::StatementLabelContext* ctx )
{
  if ( ctx )
  {
    make_identifier( ctx->IDENTIFIER() );
    addToken( ":", ctx->COLON(), FmtToken::SPACE | FmtToken::ATTACHED );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_identifier( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, FmtToken::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_string_literal( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, FmtToken::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_integer_literal( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, FmtToken::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_float_literal( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, FmtToken::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_bool_literal( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, FmtToken::SPACE );
  return {};
}

void PrettifyFileProcessor::addToken( std::string&& text, const Position& pos, int style,
                                      size_t token_type, FmtContext context )
{
  linebuilder.addPart( { std::forward<std::string>( text ), pos, style, _currentgroup, token_type,
                         context, _currentscope } );
}
void PrettifyFileProcessor::addToken( std::string&& text, antlr4::tree::TerminalNode* terminal,
                                      int style, FmtContext context )
{
  if ( compilercfg.FormatterKeepKeywords )
    addToken( terminal->getSymbol()->getText(), Range( *terminal ).start, style,
              terminal->getSymbol()->getType(), context );
  else
    addToken( std::forward<std::string>( text ), Range( *terminal ).start, style,
              terminal->getSymbol()->getType(), context );
}
void PrettifyFileProcessor::addToken( std::string&& text, antlr4::Token* token, int style,
                                      FmtContext context )
{
  if ( compilercfg.FormatterKeepKeywords )
    addToken( token->getText(), Range( token ).start, style, token->getType(), context );
  else
    addToken( std::forward<std::string>( text ), Range( token ).start, style, token->getType(),
              context );
}

void PrettifyFileProcessor::preprocess( SourceFile& sf )
{
  auto rawlines = load_raw_file();
  auto comments = collectComments( sf );
  std::vector<Range> skiplines;
  for ( const auto& c : comments )
  {
    if ( c.text.find( "format-off" ) != std::string::npos )
      skiplines.emplace_back( c.pos, c.pos );
    else if ( c.text.find( "format-on" ) != std::string::npos )
      if ( !skiplines.empty() )
        skiplines.back().end = c.pos_end;
  }
  if ( !skiplines.empty() )
  {
    // set unmatched end to file eof
    if ( skiplines.back().start.line_number == skiplines.back().end.line_number )
      skiplines.back().end.line_number = rawlines.size() + 1;
  }

  // remove all comments between noformat ranges
  comments.erase( std::remove_if( comments.begin(), comments.end(),
                                  [&]( auto& c )
                                  {
                                    for ( const auto& s : skiplines )
                                    {
                                      if ( s.start.line_number <= c.pos.line_number &&
                                           s.end.line_number >= c.pos.line_number )
                                        return true;
                                    }
                                    return false;
                                  } ),
                  comments.end() );
  linebuilder.setComments( std::move( comments ) );
  linebuilder.setSkipLines( std::move( skiplines ) );
  linebuilder.setRawLines( std::move( rawlines ) );
}

std::vector<std::string> PrettifyFileProcessor::load_raw_file()
{
  Clib::FileContents fc( source_file_identifier.pathname.c_str(), true );
  const auto& contents = fc.str_contents();
  std::vector<std::string> rawlines;
  std::string currline;
  for ( size_t i = 0; i < contents.size(); ++i )
  {
    if ( contents[i] == '\r' && i + 1 < contents.size() && contents[i + 1] == '\n' )
    {
      ++i;
      rawlines.emplace_back( std::move( currline ) );
      currline.clear();
      continue;
    }
    if ( contents[i] == '\r' || contents[i] == '\n' )
    {
      rawlines.emplace_back( std::move( currline ) );
      currline.clear();
    }
    else
      currline += contents[i];
  }
  if ( !currline.empty() )
    rawlines.emplace_back( std::move( currline ) );
  return rawlines;
}

std::vector<FmtToken> PrettifyFileProcessor::collectComments( SourceFile& sf )
{
  auto alltokens = sf.get_all_tokens();
  std::vector<FmtToken> comments;
  for ( size_t i = 0; i < alltokens.size(); ++i )
  {
    const auto& comment = alltokens[i];
    if ( comment->getType() == EscriptLexer::COMMENT ||
         comment->getType() == EscriptLexer::LINE_COMMENT )
    {
      Range startpos( comment );
      Range endpos( comment );

      // use whitespace token as start if its on the same line
      // so that the tokenid of a comment is always the next tokenid after a "actual" token
      if ( i > 0 )
      {
        if ( alltokens[i - 1]->getType() == EscriptLexer::WS &&
             alltokens[i - 1]->getLine() == startpos.start.line_number )
          startpos = Range( alltokens[i - 1] );
      }
      FmtToken info;
      info.text = comment->getText();
      info.token_type = comment->getType();
      info.style = FmtToken::SPACE;
      // replace tabs with spaces if not active
      if ( compilercfg.FormatterFormatInsideComments && !compilercfg.FormatterUseTabs )
      {
        auto it = info.text.find( '\t' );
        while ( it != std::string::npos )
        {
          // replacing tabs inbetween is a bit more complicated...
          // the size is based on the next tabstop which is eg 4
          // if its multiline we need to calc it based on the last newline
          auto lastnewline = info.text.find_last_of( '\n', it );
          if ( lastnewline == std::string::npos )
            lastnewline = 0;
          auto delta = ( it - lastnewline ) % compilercfg.FormatterTabWidth;
          info.text.replace( it, 1, std::string( compilercfg.FormatterTabWidth - delta, ' ' ) );
          it = info.text.find( '\t' );
        }
      }
      if ( info.token_type == EscriptLexer::LINE_COMMENT )
      {
        info.context = FmtContext::LINE_COMMENT;
        info.style |= FmtToken::FORCED_BREAK;
        if ( compilercfg.FormatterFormatInsideComments )
        {
          info.text.erase( 0, 2 );  // remove //
          auto firstchar = info.text.find_first_not_of( " \t" );
          if ( info.text.empty() || firstchar == std::string::npos )
          {
            // empty or only whitespace
            info.text = "//";
          }
          else if ( info.text.front() == '/' )
          {
            // assuming its ////// -> keep it
            info.text = std::string( "//" ) + info.text;
          }
          else
          {
            // add space if non is there
            if ( info.text.front() != ' ' )
              info.text = std::string( "// " ) + info.text;
            else
              info.text = std::string( "//" ) + info.text;
          }
          auto lastchar = info.text.find_last_not_of( ' ' );
          info.text.erase( info.text.begin() + lastchar + 1, info.text.end() );
        }
      }
      else
      {
        info.context = FmtContext::COMMENT;
        if ( compilercfg.FormatterFormatInsideComments )
        {
          info.text.erase( 0, 2 );                                  // remove /*
          info.text.erase( info.text.end() - 2, info.text.end() );  // remove */
          auto firstchar = info.text.find_first_not_of( " \t" );
          // remove remaining whitespace
          info.text.erase( 0, firstchar );
          auto lastchar = info.text.find_last_not_of( " \t" );
          info.text.erase( info.text.begin() + lastchar + 1, info.text.end() );
          if ( info.text.empty() )
            info.text = std::string( "/*" ) + info.text + "*/";
          else
          {
            std::string text = "/*";
            // if its in the style of "/** blubb" or "/*\n"  dont add whitespace
            if ( info.text.front() != '\n' && info.text.front() != '\r' &&
                 info.text.front() != '*' )
              text += ' ';
            text += info.text;
            // if its in the style of "blubb **/" or "\n*/"  dont add whitespace
            if ( info.text.back() != '\n' && info.text.back() != '\r' && info.text.back() != '*' )
              text += ' ';

            // detect aligned "*/"
            // /*
            //  *
            //  */
            if ( info.text.back() == '\n' || info.text.back() == '\r' )
            {
              auto lastnewline = text.find_last_of( "\n\r", text.size() - 2 );
              if ( lastnewline != std::string::npos && lastnewline + 2 < text.size() )
              {
                if ( text[lastnewline + 1] == ' ' && text[lastnewline + 2] == '*' )
                  text += ' ';
              }
            }
            text += "*/";
            info.text = text;
          }
        }
        // split lines to use correct linenendings
        std::string rawtext = std::move( info.text );
        info.text.clear();
        for ( size_t ri = 0; ri < rawtext.size(); ++ri )
        {
          if ( rawtext[ri] == '\r' && ri + 1 < rawtext.size() && rawtext[ri + 1] == '\n' )
          {
            ++ri;
            auto lastchar = info.text.find_last_not_of( ' ' );
            info.text.erase( info.text.begin() + lastchar + 1, info.text.end() );
            info.text += compilercfg.FormatterWindowsLineEndings ? "\r\n" : "\n";
          }
          else if ( rawtext[ri] == '\r' || rawtext[ri] == '\n' )
          {
            auto lastchar = info.text.find_last_not_of( ' ' );
            info.text.erase( info.text.begin() + lastchar + 1, info.text.end() );
            info.text += compilercfg.FormatterWindowsLineEndings ? "\r\n" : "\n";
          }
          else
            info.text += rawtext[ri];
        }
      }
      info.pos = startpos.start;
      info.pos_end = endpos.end;
      comments.emplace_back( std::move( info ) );
    }
  }
  return comments;
}
}  // namespace Pol::Bscript::Compiler
