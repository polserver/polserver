#include "SourceFileFormatter.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "clib/strutil.h"

#include <iostream>
#include <numeric>

namespace Pol::Bscript::Compiler
{

hardline_t hardline;
space_t space;
indent_t indent;
unindent_t unindent;

SourceFileFormatter::SourceFileFormatter(
    const Pol::Bscript::Compiler::SourceFileIdentifier& identifier,
    std::shared_ptr<Pol::Bscript::Compiler::SourceFile> sf, Report& report )
    : ident( identifier ), sf( sf ), report( report )
{
}

std::string SourceFileFormatter::format()
{
  antlr4::ParserRuleContext* ctx = nullptr;

  if ( auto compilation_unit = sf->get_compilation_unit( report, ident ) )
  {
    ctx = compilation_unit;
  }
  else if ( auto module_unit = sf->get_module_unit( report, ident ) )
  {
    ctx = module_unit;
  }
  else
  {
    throw std::runtime_error( "No compilation or module unit found." );
  }

  if ( report.error_count() )
  {
    return "";
  }

  ctx->accept( this );
  return formatted;
}

void SourceFileFormatter::out( const hardline_t&, bool /*include_right_comments*/ )
{
  extra_lines_allowed = true;

  // fmt::format_to( std::back_inserter( formatted ), "{}{}\n", formatted, current_line );
  auto indents = current_line.empty() ? 0 : current_indent_level();

  ++last_printed_line_number;

  formatted += std::string( indents, ' ' ) + current_line + "\n";
  current_line = "";
}

int SourceFileFormatter::current_indent_level() const
{
  return indent_levels.empty() ? 0 : indent_levels.top();
}

void SourceFileFormatter::out( const indent_t& )
{
  indent_levels.push( current_indent_level() + 2 );
}
void SourceFileFormatter::out( const unindent_t& )
{
  if ( !indent_levels.empty() )
  {
    indent_levels.pop();
  }
}

void SourceFileFormatter::out() {}

void SourceFileFormatter::out( const space_t& )
{
  if ( last_node != nullptr )
  {
    current_line += " ";
  }
}

void SourceFileFormatter::out( const Terminal& a )
{
  // We only add nodes that are not missing.
  last_node = a.node;
  if ( a.node )
  {
    auto symbol = a.node->getSymbol();
    auto line_number = symbol->getLine();

    if ( extra_lines_allowed )
    {
      int offset = static_cast<int>( line_number ) - last_printed_line_number;

      while ( offset > 0 )
      {
        out( hardline );
        offset--;
      }

      extra_lines_allowed = false;
    }

    last_printed_node = a.node;
    current_line += ( a.value.empty() ? a.node->getText() : a.value );
    last_printed_line_number = line_number;
  }
}

void SourceFileFormatter::out( const std::string& a )
{
  current_line += a;
}

void SourceFileFormatter::print() {}

template <typename T, typename... Types>
void SourceFileFormatter::print( T var1, Types... var2 )
{
  out( var1 );
  print( var2... );
}

antlrcpp::Any SourceFileFormatter::visitUseDeclaration(
    EscriptGrammar::EscriptParser::UseDeclarationContext* ctx )
{
  print( Terminal( ctx->USE(), "use" ), space, Terminal::from( ctx->stringIdentifier() ),
         Terminal( ctx->SEMI(), ";" ), hardline );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitProgramDeclaration(
    EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx )
{
  print( Terminal( ctx->PROGRAM(), "program" ), space, Terminal( ctx->IDENTIFIER() ) );
  visitProgramParameters( ctx->programParameters() );
  print( hardline, indent );
  visitBlock( ctx->block() );
  print( unindent, Terminal( ctx->ENDPROGRAM(), "endprogram" ), hardline );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitStatement(
    EscriptGrammar::EscriptParser::StatementContext* ctx )
{
  if ( auto expr = ctx->statementExpression )
  {
    visitExpression( expr );
    print( Terminal( ctx->SEMI(), ";" ), hardline );
  }
  else
  {
    visitChildren( ctx );
  }
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitIfStatement(
    EscriptGrammar::EscriptParser::IfStatementContext* ctx )
{
  auto elseif_index = 0;
  auto par_expression_index = 0;
  auto block_index = 0;

  print( Terminal( ctx->IF(), "if" ), space );
  if ( auto ifExpr = ctx->parExpression( par_expression_index++ ) )
  {
    visitParExpression( ifExpr );
  }
  if ( auto then = ctx->THEN() )
  {
    print( space, Terminal( then, "then" ) );
  }
  print( hardline, indent );
  if ( auto ifBlock = ctx->block( block_index++ ) )
  {
    visitBlock( ifBlock );
  }
  print( unindent );

  while ( auto elseif = ctx->ELSEIF( elseif_index++ ) )
  {
    print( Terminal( ctx->IF(), "elseif" ), space );
    if ( auto elseifExpr = ctx->parExpression( par_expression_index++ ) )
    {
      visitParExpression( elseifExpr );
    }
    print( hardline, indent );
    if ( auto elseifBlock = ctx->block( block_index++ ) )
    {
      visitBlock( elseifBlock );
    }
    print( unindent );
  }

  if ( auto else_node = ctx->ELSE() )
  {
    print( Terminal( else_node, "else" ), hardline, indent );
    if ( auto elseBlock = ctx->block( block_index++ ) )
    {
      visitBlock( elseBlock );
    }
    print( unindent );
  }
  print( Terminal( ctx->ENDIF(), "endif" ), hardline );

  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitProgramParameters(
    EscriptGrammar::EscriptParser::ProgramParametersContext* ctx )
{
  print( Terminal( ctx->LPAREN(), "(" ) );
  if ( auto programParameterList = ctx->programParameterList() )
  {
    visitProgramParameterList( programParameterList );
  }
  print( Terminal( ctx->RPAREN(), ")" ) );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitProgramParameterList(
    EscriptGrammar::EscriptParser::ProgramParameterListContext* ctx )
{
  // This loops through children, since the COMMA is optional inside the repeated programParameters,
  // ie.: `a b, c d` -> location of comma is independent of location of programParameter
  for ( size_t i = 0; i < ctx->children.size(); ++i )
  {
    auto* child = ctx->children.at( i );

    auto text = child->getText();
    if ( antlr4::tree::TerminalNode::is( child ) )
    {
      antlr4::tree::TerminalNode* typedChild = static_cast<antlr4::tree::TerminalNode*>( child );
      antlr4::Token* symbol = typedChild->getSymbol();
      if ( symbol->getType() == EscriptGrammar::EscriptParser::COMMA )
      {
        print( Terminal( typedChild, "," ) );
        if ( i < ctx->children.size() - 1 )
        {
          print( space );
        }
      }
    }
    else if ( antlr4::RuleContext::is( child ) )
    {
      if ( auto* typedChild =
               dynamic_cast<EscriptGrammar::EscriptParser::ProgramParameterContext*>( child );
           typedChild != nullptr )
      {
        if ( last_printed_node != nullptr )
        {
          auto last_printed_node_type = last_printed_node->getSymbol()->getType();
          if ( last_printed_node_type != EscriptGrammar::EscriptParser::COMMA &&
               last_printed_node_type != EscriptGrammar::EscriptParser::LPAREN )
            print( space );
        }
        visitProgramParameter( typedChild );
      }
    }
  }

  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitProgramParameter(
    EscriptGrammar::EscriptParser::ProgramParameterContext* ctx )
{
  print( Terminal( ctx->UNUSED(), "unused" ), space, Terminal( ctx->IDENTIFIER() ) );
  if ( auto expr = ctx->expression() )
  {
    print( space, Terminal( ctx->ASSIGN(), ":=" ), space );
    visitExpression( expr );
  }

  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitPrimary(
    EscriptGrammar::EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
  {
    print( Terminal::from( literal ) );
  }
  else if ( auto par_expression = ctx->parExpression() )
  {
    visitParExpression( par_expression );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    print( identifier );
  }
  else if ( auto f_call = ctx->functionCall() )
  {
    visitFunctionCall( f_call );
  }
  else if ( auto scoped_f_call = ctx->scopedFunctionCall() )
  {
    visitScopedFunctionCall( scoped_f_call );
  }
  else if ( auto dict_init = ctx->explicitDictInitializer() )
  {
    visitExplicitDictInitializer( dict_init );
  }
  else if ( auto struct_init = ctx->explicitStructInitializer() )
  {
    // return struct_initializer( struct_init );
  }
  else if ( auto fr = ctx->functionReference() )
  {
    // return function_reference( fr );
  }
  else if ( auto error_init = ctx->explicitErrorInitializer() )
  {
    // return error( error_init );
  }
  else if ( auto array_init = ctx->explicitArrayInitializer() )
  {
    // return array_initializer( array_init );
  }
  else if ( auto bare_array = ctx->bareArrayInitializer() )
  {
    // return array_initializer( bare_array );
  }
  else if ( auto inter_string = ctx->interpolatedString() )
  {
    // return interpolate_string( inter_string );
  }
  else if ( auto uninit = ctx->UNINIT() )
  {
    print( Terminal( uninit, "uninit" ) );
  }
  else if ( auto b_true = ctx->BOOL_TRUE() )
  {
    print( Terminal( b_true, "true" ) );
  }
  else if ( auto b_false = ctx->BOOL_FALSE() )
  {
    print( Terminal( b_false, "false" ) );
  }

  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitExplicitDictInitializer(
    EscriptGrammar::EscriptParser::ExplicitDictInitializerContext* ctx )
{
  print( Terminal( ctx->DICTIONARY(), "dictionary" ) );

  if ( auto dictInitializer = ctx->dictInitializer() )
    visitDictInitializer( dictInitializer );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitParExpression(
    EscriptGrammar::EscriptParser::ParExpressionContext* ctx )
{
  print( Terminal( ctx->LPAREN(), "(" ) );

  if ( auto expr = ctx->expression() )
    visitExpression( expr );

  print( Terminal( ctx->RPAREN(), ")" ) );

  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitExpressionList(
    EscriptGrammar::EscriptParser::ExpressionListContext* ctx )
{
  size_t expression_index = 0;
  size_t COMMA_index = 0;

  while ( auto expression = ctx->expression( expression_index++ ) )
  {
    visitExpression( expression );
    if ( auto COMMA = ctx->COMMA( COMMA_index++ ) )
    {
      print( Terminal( COMMA, "," ) );
      if ( expression_index < ctx->expression().size() )
      {
        print( space );
      }
    }
  }

  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitScopedFunctionCall(
    EscriptGrammar::EscriptParser::ScopedFunctionCallContext* ctx )
{
  print( ctx->IDENTIFIER(), Terminal( ctx->COLONCOLON(), "::" ) );
  if ( auto f_call = ctx->functionCall() )
    visitFunctionCall( f_call );
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitExpression(
    EscriptGrammar::EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
  {
    visitPrimary( prim );
  }
  else if ( ctx->prefix )
  {
    if ( auto prefix = ctx->children.size() < 1 ? nullptr : ctx->children.at( 0 ) )
      print( Terminal::from( prefix ) );
    if ( !ctx->expression().empty() )
      visitExpression( ctx->expression( 0 ) );
  }
  else if ( ctx->postfix )
  {
    if ( !ctx->expression().empty() )
      visitExpression( ctx->expression( 0 ) );

    if ( auto postfix = ctx->children.size() < 2 ? nullptr : ctx->children.at( 1 ) )
      print( Terminal::from( postfix ) );
  }
  else if ( ctx->bop )
  {
    if ( auto left = ctx->expression( 0 ) )
      visitExpression( left );

    print( space );

    if ( auto bop = ctx->children.empty() ? nullptr : ctx->children.at( 1 ) )
      print( Terminal::from( bop ) );

    print( space );

    if ( auto right = ctx->expression( 1 ) )
      visitExpression( right );
  }
  else if ( auto suffix = ctx->expressionSuffix() )
  {
    // expression_suffix( expression( ctx->expression()[0] ), suffix );
  }
  else if ( ctx->QUESTION() )
  {
    // conditional_operator( ctx );
  }

  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitFunctionCall(
    EscriptGrammar::EscriptParser::FunctionCallContext* ctx )
{
  print( ctx->IDENTIFIER() );
  print( Terminal( ctx->LPAREN(), "(" ) );

  if ( auto exprList = ctx->expressionList() )
    visitExpressionList( exprList );

  print( Terminal( ctx->RPAREN(), ")" ) );

  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitDictInitializerExpression(
    EscriptGrammar::EscriptParser::DictInitializerExpressionContext* ctx )
{
  if ( auto key = ctx->expression( 0 ) )
  {
    visitExpression( key );
  }
  if ( auto arrow = ctx->ARROW() )
  {
    print( space, Terminal( arrow, "->" ), space );
    if ( auto value = ctx->expression( 1 ) )
    {
      visitExpression( value );
    }
  }
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitDictInitializerExpressionList(
    EscriptGrammar::EscriptParser::DictInitializerExpressionListContext* ctx )
{
  size_t dictInitializerExpression_index = 0;
  size_t COMMA_index = 0;
  while ( auto dictInitializerExpression =
              ctx->dictInitializerExpression( dictInitializerExpression_index++ ) )
  {
    visitDictInitializerExpression( dictInitializerExpression );
    if ( auto COMMA = ctx->COMMA( COMMA_index++ ) )
    {
      print( Terminal( COMMA, "," ), space );
    }
  }
  return antlrcpp::Any();
}

antlrcpp::Any SourceFileFormatter::visitDictInitializer(
    EscriptGrammar::EscriptParser::DictInitializerContext* ctx )
{
  print( Terminal( ctx->LBRACE(), "{" ) );

  if ( auto dictInitializerExpressionList = ctx->dictInitializerExpressionList() )
    visitDictInitializerExpressionList( dictInitializerExpressionList );

  print( Terminal( ctx->RBRACE(), "}" ) );

  return antlrcpp::Any();
}

Terminal Terminal::from( antlr4::tree::ParseTree* ctx )
{
  antlr4::tree::ParseTree* tree = ctx;

  while ( tree )
  {
    if ( antlr4::tree::TerminalNode::is( tree ) )
    {
      return Terminal( static_cast<antlr4::tree::TerminalNode*>( tree ), tree->getText() );
    }
    tree = tree->children.empty() ? nullptr : tree->children.at( 0 );
  }

  return Terminal( nullptr );
}
}  // namespace Pol::Bscript::Compiler
