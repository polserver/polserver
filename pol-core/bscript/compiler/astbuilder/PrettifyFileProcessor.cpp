#include "PrettifyFileProcessor.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/JsonAst.h"
#include "clib/logfacility.h"

#include <iostream>

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
const int IDENT_LEVEL = 2;
const int LINEWIDTH = 80;

PrettifyFileProcessor::PrettifyFileProcessor( const SourceFileIdentifier& source_file_identifier,
                                              Profile& profile, Report& report )
    : source_file_identifier( source_file_identifier ), profile( profile ), report( report )
{
}

antlrcpp::Any PrettifyFileProcessor::process_compilation_unit( SourceFile& sf )
{
  if ( auto compilation_unit = sf.get_compilation_unit( report, source_file_identifier ) )
  {
    collectComments( sf );
    return compilation_unit->accept( this );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}

antlrcpp::Any PrettifyFileProcessor::process_module_unit( SourceFile& sf )
{
  if ( auto module_unit = sf.get_module_unit( report, source_file_identifier ) )
  {
    collectComments( sf );
    return module_unit->accept( this );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}
std::string PrettifyFileProcessor::prettify() const
{
  return fmt::format( "{}", fmt::join( _lines, "\n" ) );
}

void PrettifyFileProcessor::collectComments( SourceFile& sf )
{
  auto alltokens = sf.get_all_tokens();

  _comments.clear();
  for ( const auto& comment : alltokens )
  {
    if ( comment->getType() == EscriptGrammar::EscriptLexer::COMMENT ||
         comment->getType() == EscriptGrammar::EscriptLexer::LINE_COMMENT )
    {
      Range r( &*comment );
      CommentInfo info;
      info.text = comment->getText();
      if ( comment->getType() == EscriptGrammar::EscriptLexer::LINE_COMMENT )
      {
        info.text.erase( 0, 2 );  // remove //
        auto t = info.text.find_first_not_of( " \t" );
        info.text.erase( 0, t );  // remove remaining whitespace
        info.text = std::string( "// " ) + info.text;
        info.linecomment = true;
      }
      else
      {
        info.text.erase( 0, 2 );  // remove /*
        auto firstchar = info.text.find_first_not_of( " \t" );
        info.text.erase( 0, firstchar );                          // remove remaining whitespace
        info.text.erase( info.text.end() - 2, info.text.end() );  // remove */
        auto lastchar = info.text.find_last_not_of( " \t" );
        info.text.erase( info.text.begin() + lastchar + 1, info.text.end() );
        // if its in the style of /*** blubb **/ dont add whitespace
        if ( info.text.front() == '*' && info.text.back() == '*' )
          info.text = std::string( "/*" ) + info.text + "*/";
        else
          info.text = std::string( "/* " ) + info.text + " */";
        info.linecomment = false;
      }
      info.pos = r.start;
      info.pos_end = r.end;
      _comments.emplace_back( std::move( info ) );
    }
  }
}

void PrettifyFileProcessor::mergeComments()
{
  // add comments before current line
  while ( !_comments.empty() )
  {
    if ( _comments.front().pos.line_number < _line_parts.front().lineno )
    {
      if ( _comments.front().pos.line_number > _last_line + 1 )
      {
        _lines.emplace_back( "" );
      }
      _lines.push_back( std::string( _currident * IDENT_LEVEL, ' ' ) + _comments.front().text );
      _last_line = _comments.front().pos_end.line_number;
      _comments.erase( _comments.begin() );
    }
    else
      break;
  }
  // add comments inbetween
  for ( size_t i = 0; i < _line_parts.size(); )
  {
    if ( _comments.empty() )
      break;
    if ( _line_parts[i].tokenid > _comments.front().pos.token_index )
    {
      TokenPart p{ std::move( _comments.front().text ), _comments.front().pos, TokenPart::SPACE };
      if ( _comments.front().linecomment )
        p.style |= TokenPart::FORCED_BREAK;
      _line_parts.insert( _line_parts.begin() + i, p );
      _comments.erase( _comments.begin() );
    }
    else
      ++i;
  }
  // add comments at the end of the current line
  if ( !_comments.empty() )
  {
    // next token could be terminator, but further away could be a new "line"
    if ( _comments.front().pos.line_number == _line_parts.back().lineno )
    {
      if ( _comments.front().linecomment ||
           _comments.front().pos.token_index <= _line_parts.back().tokenid + 2 )
      {
        _line_parts.emplace_back( std::move( _comments.front().text ), _comments.front().pos,
                                  TokenPart::SPACE );
        _comments.erase( _comments.begin() );
      }
    }
  }
}

void PrettifyFileProcessor::buildLine()
{
  mergeComments();

  // fill lines with final strings splitted at breakpoints
  std::vector<std::pair<std::string, bool>> lines;
  std::string line;
  for ( size_t i = 0; i < _line_parts.size(); ++i )
  {
    line += _line_parts[i].text;
    // add space if set, but not if the following part is attached
    if ( _line_parts[i].style & TokenPart::SPACE )
    {
      if ( i + 1 < _line_parts.size() )
      {
        if ( !( _line_parts[i + 1].style & TokenPart::ATTACHED ) )
          line += ' ';
      }
      else
        line += ' ';
    }
    if ( _line_parts[i].style & TokenPart::FORCED_BREAK )
    {
      lines.emplace_back( std::make_pair( std::move( line ), true ) );
      line.clear();
    }
    // start a new line if breakpoint
    else if ( _line_parts[i].style & TokenPart::BREAKPOINT )
    {
      // if the next part is attached, dont break now and instead after the attached one
      if ( i + 1 < _line_parts.size() )
      {
        if ( _line_parts[i + 1].style & TokenPart::ATTACHED )
        {
          _line_parts[i + 1].style |= TokenPart::BREAKPOINT;
          continue;
        }
      }
      lines.emplace_back( std::make_pair( std::move( line ), false ) );
      line.clear();
    }
  }
  // add remainings
  if ( !line.empty() )
  {
    lines.emplace_back( std::make_pair( std::move( line ), false ) );
    line.clear();
  }
  INFO_PRINTLN( "BREAK " );
  for ( auto& [l, forced] : lines )
    INFO_PRINTLN( "\"{}\"", l );
  // add newline from original sourcecode
  // TODO: it just adds one
  if ( _line_parts.front().lineno > _last_line + 1 )
  {
    _lines.emplace_back( "" );
  }
  // build final lines
  size_t alignmentspace = 0;
  for ( auto& [l, forced] : lines )
  {
    // following lines need to be aligned
    if ( line.empty() && alignmentspace )
      line = std::string( alignmentspace, ' ' );
    // first breakpoint defines the alignment and add initial ident level
    if ( !alignmentspace )
    {
      alignmentspace = l.size() + _currident * IDENT_LEVEL;
      line += std::string( _currident * IDENT_LEVEL, ' ' );
    }
    line += l;
    // linewidth reached add current line, start a new one
    if ( line.size() > LINEWIDTH || forced )
    {
      _lines.emplace_back( std::move( line ) );
      line.clear();
    }
  }
  if ( !line.empty() )
    _lines.emplace_back( std::move( line ) );
  _last_line = _line_parts.back().lineno;
  _line_parts.clear();
}

antlrcpp::Any PrettifyFileProcessor::visitCompilationUnit(
    EscriptGrammar::EscriptParser::CompilationUnitContext* ctx )
{
  visitChildren( ctx );
  while ( !_comments.empty() )
  {
    _lines.push_back( std::string( _currident * 2, ' ' ) + _comments.front().text );
    _comments.erase( _comments.begin() );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitVarStatement(
    EscriptGrammar::EscriptParser::VarStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "var", r.start, TokenPart::SPACE );
  visitVariableDeclarationList( ctx->variableDeclarationList() );
  _line_parts.emplace_back( ";", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitWhileStatement(
    EscriptGrammar::EscriptParser::WhileStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  Range r( *ctx );
  _line_parts.emplace_back( "while", r.start, TokenPart::SPACE );
  _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  auto testc = ctx->parExpression()->expression();
  visitExpression( testc );
  _line_parts.emplace_back( ")", Range( *testc ).end, TokenPart::SPACE );
  buildLine();

  ++_currident;
  visitBlock( ctx->block() );
  --_currident;
  _line_parts.emplace_back( "endwhile", r.end, TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitVariableDeclarationList(
    EscriptGrammar::EscriptParser::VariableDeclarationListContext* ctx )
{
  auto args = ctx->variableDeclaration();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    if ( i > 0 )
    {
      Range r( *args[i] );
      _line_parts.emplace_back( ",", r.start, TokenPart::SPACE | TokenPart::ATTACHED );
    }
    visitVariableDeclaration( args[i] );
  }
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitVariableDeclaration(
    EscriptGrammar::EscriptParser::VariableDeclarationContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  if ( auto variable_declaration_initializer = ctx->variableDeclarationInitializer() )
  {
    if ( auto expression = variable_declaration_initializer->expression() )
    {
      Range r( *expression );
      _line_parts.emplace_back( ":=", r.start, TokenPart::SPACE );
      visitExpression( expression );
    }

    else if ( auto arr = variable_declaration_initializer->ARRAY() )
    {
      Range r( *arr );
      _line_parts.emplace_back( "array", r.end, TokenPart::SPACE );
    }
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitConstStatement(
    EscriptGrammar::EscriptParser::ConstStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "const", r.start, TokenPart::SPACE );
  auto const_declaration_ctx = ctx->constantDeclaration();
  make_identifier( const_declaration_ctx->IDENTIFIER() );

  if ( auto variable_declaration_initializer =
           const_declaration_ctx->variableDeclarationInitializer() )
  {
    if ( auto expression = variable_declaration_initializer->expression() )
    {
      Range re( *expression );
      _line_parts.emplace_back( ":=", re.start, TokenPart::SPACE );
      visitExpression( expression );
      _line_parts.emplace_back( ";", re.end,
                                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    }

    else if ( auto array = variable_declaration_initializer->ARRAY() )
    {
      Range ra( *array );
      _line_parts.emplace_back( ":=", ra.start, TokenPart::SPACE );
      _line_parts.emplace_back( "array", ra.start, TokenPart::SPACE );
      _line_parts.emplace_back( ";", ra.end,
                                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    }
  }

  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDictInitializerExpression(
    EscriptGrammar::EscriptParser::DictInitializerExpressionContext* ctx )
{
  visitExpression( ctx->expression( 0 ) );  // key

  if ( ctx->expression().size() > 1 )
  {
    Range r( *ctx->expression( 1 ) );
    _line_parts.emplace_back( "->", r.start, TokenPart::SPACE );
    visitExpression( ctx->expression( 1 ) );
  }
  Range r( *ctx );
  _line_parts.emplace_back( ",", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDoStatement(
    EscriptGrammar::EscriptParser::DoStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  Range r( *ctx );
  _line_parts.emplace_back( "do", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();

  ++_currident;
  visitBlock( ctx->block() );
  --_currident;

  auto exp = ctx->parExpression()->expression();
  _line_parts.emplace_back( "dowhile", Range( *exp ).start, TokenPart::SPACE );
  _line_parts.emplace_back( "(", Range( *exp ).start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( exp );
  _line_parts.emplace_back( ")", Range( *exp ).end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitEnumListEntry(
    EscriptGrammar::EscriptParser::EnumListEntryContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    _line_parts.emplace_back( ":=", Range( *expression ).start, TokenPart::SPACE );
    visitExpression( expression );
  }
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitEnumList(
    EscriptGrammar::EscriptParser::EnumListContext* ctx )

{
  auto enums = ctx->enumListEntry();
  for ( size_t i = 0; i < enums.size(); ++i )
  {
    visitEnumListEntry( enums[i] );
    if ( i < enums.size() - 1 )
      _line_parts.emplace_back( ",", Range( *enums[i] ).end,
                                TokenPart::SPACE | TokenPart::ATTACHED );

    buildLine();
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitEnumStatement(
    EscriptGrammar::EscriptParser::EnumStatementContext* ctx )
{
  _line_parts.emplace_back( "enum", Range( *ctx ).start, TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  buildLine();
  ++_currident;
  auto enums = visitEnumList( ctx->enumList() );
  --_currident;

  _line_parts.emplace_back( "endenum", Range( *ctx ).end, TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExitStatement(
    EscriptGrammar::EscriptParser::ExitStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "exit", r.start, TokenPart::NONE );
  _line_parts.emplace_back( ";", r.start, TokenPart::ATTACHED | TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitArrayInitializer(
    EscriptGrammar::EscriptParser::ExplicitArrayInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "array", r.start, TokenPart::NONE );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  visitChildren( ctx );
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitDictInitializer(
    EscriptGrammar::EscriptParser::ExplicitDictInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "dictionary", r.start, TokenPart::NONE );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  visitChildren( ctx );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitErrorInitializer(
    EscriptGrammar::EscriptParser::ExplicitErrorInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "error", r.start, TokenPart::NONE );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  visitChildren( ctx );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBareArrayInitializer(
    EscriptGrammar::EscriptParser::BareArrayInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  visitChildren( ctx );
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitStructInitializer(
    EscriptGrammar::EscriptParser::ExplicitStructInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "struct", r.start, TokenPart::NONE );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  visitChildren( ctx );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExpressionList(
    EscriptGrammar::EscriptParser::ExpressionListContext* ctx )
{
  auto args = ctx->expression();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    if ( i > 0 )
    {
      Range r( *args[i] );
      _line_parts.emplace_back( ",", r.start,
                                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    }
    visitExpression( args[i] );
  }

  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitExpression(
    EscriptGrammar::EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
    visitPrimary( prim );
  else if ( ctx->prefix )
  {
    Range r( ctx->prefix );
    _line_parts.emplace_back( ctx->prefix->getText(), r.start, TokenPart::NONE );
    visitExpression( ctx->expression( 0 ) );
  }
  else if ( ctx->postfix )
  {
    Range r( ctx->postfix );
    visitExpression( ctx->expression( 0 ) );
    _line_parts.emplace_back( ctx->postfix->getText(), r.start,
                              TokenPart::SPACE | TokenPart::ATTACHED );
  }
  else if ( ctx->bop && ctx->expression().size() == 2 )
  {
    visitExpression( ctx->expression( 0 ) );  // left
    Range r( ctx->bop );
    auto op = ctx->bop->getText();
    if ( op == "?:" )  // break before elvis
      _line_parts.back().style |= TokenPart::BREAKPOINT;

    if ( op == "||" || op == "&&" )
      _line_parts.emplace_back( std::move( op ), r.start,
                                TokenPart::SPACE | TokenPart::BREAKPOINT );
    else if ( op == ".+" || op == ".-" || op == ".?" )
      _line_parts.emplace_back( std::move( op ), r.start, TokenPart::ATTACHED );
    else
      _line_parts.emplace_back( std::move( op ), r.start, TokenPart::SPACE );

    visitExpression( ctx->expression( 1 ) );  // right
  }
  else if ( auto suffix = ctx->expressionSuffix() )
  {
    // TODO ?
    expression_suffix( ctx->expression( 0 ), suffix );
  }
  else if ( ctx->QUESTION() )
  {
    visitExpression( ctx->expression( 0 ) );  // conditional
    Range rcond( *ctx->expression( 0 ) );
    _line_parts.emplace_back( "?", rcond.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
    visitExpression( ctx->expression( 1 ) );  // consequent
    Range rcons( *ctx->expression( 1 ) );
    _line_parts.emplace_back( ":", rcons.end, TokenPart::SPACE | TokenPart::BREAKPOINT );

    visitExpression( ctx->expression( 2 ) );  // alternate
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::expression_suffix(
    EscriptGrammar::EscriptParser::ExpressionContext* expr_ctx,
    EscriptGrammar::EscriptParser::ExpressionSuffixContext* expr_suffix_ctx )
{
  if ( auto indexing = expr_suffix_ctx->indexingSuffix() )
  {
    Range r( *indexing );
    visitExpression( expr_ctx );
    _line_parts.emplace_back( "[", r.start, TokenPart::ATTACHED );
    visitExpressionList( indexing->expressionList() );
    _line_parts.emplace_back( "]", r.end, TokenPart::SPACE | TokenPart::ATTACHED );
  }
  else if ( auto member = expr_suffix_ctx->navigationSuffix() )
  {
    Range r( *member );
    visitExpression( expr_ctx );
    _line_parts.emplace_back( ".", r.start, TokenPart::ATTACHED );
    if ( auto string_literal = member->STRING_LITERAL() )
    {
      make_string_literal( string_literal );
    }
    else if ( auto identifier = member->IDENTIFIER() )
    {
      make_identifier( identifier );
    }
  }
  else if ( auto method = expr_suffix_ctx->methodCallSuffix() )
  {
    Range r( *method );
    visitExpression( expr_ctx );
    _line_parts.emplace_back( ".", r.start, TokenPart::ATTACHED );
    make_identifier( method->IDENTIFIER() );
    _line_parts.emplace_back( "(", r.start,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    auto cursize = _line_parts.size();
    visitChildren( method );

    _line_parts.emplace_back( ")", r.end, TokenPart::SPACE );
    if ( _line_parts.size() - 1 == cursize )
      _line_parts.back().style |= TokenPart::ATTACHED;
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFloatLiteral(
    EscriptGrammar::EscriptParser::FloatLiteralContext* ctx )
{
  if ( auto float_literal = ctx->FLOAT_LITERAL() )
    return make_float_literal( float_literal );
  else if ( auto hex_float_literal = ctx->HEX_FLOAT_LITERAL() )
    return make_float_literal( hex_float_literal );

  return visitChildren( ctx );
}

antlrcpp::Any PrettifyFileProcessor::visitForeachIterableExpression(
    EscriptGrammar::EscriptParser::ForeachIterableExpressionContext* ctx )
{
  if ( auto parExpression = ctx->parExpression() )
  {
    visitExpression( parExpression->expression() );
  }
  else if ( auto functionCall = ctx->functionCall() )
  {
    visitFunctionCall( functionCall );
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
    EscriptGrammar::EscriptParser::ForeachStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "foreach", r.start, TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  Range ri( *ctx->IDENTIFIER() );
  _line_parts.emplace_back( "in", ri.end, TokenPart::SPACE );
  _line_parts.emplace_back( "(", ri.end, TokenPart::SPACE );
  make_statement_label( ctx->statementLabel() );
  visitForeachIterableExpression( ctx->foreachIterableExpression() );
  _line_parts.emplace_back( ")", ri.end, TokenPart::SPACE );
  buildLine();

  ++_currident;
  visitBlock( ctx->block() );
  --_currident;

  _line_parts.emplace_back( "endforeach", r.end, TokenPart::SPACE );
  buildLine();

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitForStatement(
    EscriptGrammar::EscriptParser::ForStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );

  auto forGroup = ctx->forGroup();

  if ( auto basicForStatement = forGroup->basicForStatement() )
  {
    visitBasicForStatement( basicForStatement );
  }
  else if ( auto cstyleForStatement = forGroup->cstyleForStatement() )
  {
    visitCstyleForStatement( cstyleForStatement );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionCall(
    EscriptGrammar::EscriptParser::FunctionCallContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );
  Range r( *ctx );

  _line_parts.emplace_back( "(", r.start,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  auto cursize = _line_parts.size();
  if ( auto args = ctx->expressionList() )
    visitExpressionList( args );
  _line_parts.emplace_back( ")", r.end, TokenPart::SPACE );
  if ( _line_parts.size() - 1 == cursize )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitFunctionDeclaration(
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  Range r( *ctx );
  auto exported = ctx->EXPORTED();
  if ( exported )
  {
    Range re( *exported );
    _line_parts.emplace_back( "exported", re.start, TokenPart::SPACE );
  }
  _line_parts.emplace_back( "function", r.start, TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  Range rname( *ctx->IDENTIFIER() );
  _line_parts.emplace_back( "(", rname.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  auto cursize = _line_parts.size();
  visitFunctionParameters( ctx->functionParameters() );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  Range rparam( *ctx->functionParameters() );
  _line_parts.emplace_back( ")", rparam.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == cursize )

    _line_parts.back().style |= TokenPart::ATTACHED;
  ;
  buildLine();

  ++_currident;
  visitBlock( ctx->block() );
  --_currident;

  _line_parts.emplace_back( "endfunction", r.end, TokenPart::SPACE );
  buildLine();
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitFunctionParameter(
    EscriptGrammar::EscriptParser::FunctionParameterContext* ctx )
{
  auto byref = ctx->BYREF();
  if ( byref )
  {
    Range re( *byref );
    _line_parts.emplace_back( "byref", re.start, TokenPart::SPACE );
  }
  auto unused = ctx->UNUSED();
  if ( unused )
  {
    Range re( *unused );
    _line_parts.emplace_back( "unused", re.start, TokenPart::SPACE );
  }
  make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    Range re( *expression );
    _line_parts.emplace_back( ":=", re.start, TokenPart::SPACE );
    visitExpression( expression );
  }
  Range re( *ctx );
  _line_parts.emplace_back( ",", re.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBreakStatement(
    EscriptGrammar::EscriptParser::BreakStatementContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );
  }

  _line_parts.emplace_back( "break", Range( *ctx ).end, TokenPart::NONE );
  _line_parts.emplace_back( ";", Range( *ctx ).end, TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitSwitchBlockStatementGroup(
    EscriptGrammar::EscriptParser::SwitchBlockStatementGroupContext* ctx )
{
  for ( const auto& switchLabel : ctx->switchLabel() )
  {
    visitSwitchLabel( switchLabel );
  }
  ++_currident;
  visitBlock( ctx->block() );
  --_currident;

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitSwitchLabel(
    EscriptGrammar::EscriptParser::SwitchLabelContext* ctx )
{
  if ( auto integerLiteral = ctx->integerLiteral() )
  {
    visitIntegerLiteral( integerLiteral );
    _line_parts.emplace_back( ":", Range( *integerLiteral ).end,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  else if ( auto boolLiteral = ctx->boolLiteral() )
  {
    visitBoolLiteral( boolLiteral );
    _line_parts.emplace_back( ":", Range( *boolLiteral ).end,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  else if ( auto uninit = ctx->UNINIT() )
  {
    _line_parts.emplace_back( "uninit", Range( *uninit ).start, TokenPart::SPACE );
    _line_parts.emplace_back( ":", Range( *uninit ).end,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );
    _line_parts.emplace_back( ":", Range( *identifier ).end,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  else if ( auto string_literal = ctx->STRING_LITERAL() )
  {
    make_string_literal( string_literal );
    _line_parts.emplace_back( ":", Range( *string_literal ).end,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  else if ( auto defaultctx = ctx->DEFAULT() )
  {
    _line_parts.emplace_back( "default", Range( *defaultctx ).start, TokenPart::SPACE );
    _line_parts.emplace_back( ":", Range( *defaultctx ).end,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitCaseStatement(
    EscriptGrammar::EscriptParser::CaseStatementContext* ctx )
{
  Range r( *ctx );
  make_statement_label( ctx->statementLabel() );
  _line_parts.emplace_back( "case", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression() );
  _line_parts.emplace_back( ")", Range( *ctx->expression() ).end,
                            TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  for ( const auto& switchBlockStatementGroup : ctx->switchBlockStatementGroup() )
  {
    visitSwitchBlockStatementGroup( switchBlockStatementGroup );
  }
  _line_parts.emplace_back( "endcase", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitContinueStatement(
    EscriptGrammar::EscriptParser::ContinueStatementContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );
  }

  _line_parts.emplace_back( "continue", Range( *ctx ).end, TokenPart::NONE );
  _line_parts.emplace_back( ";", Range( *ctx ).end, TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBasicForStatement(
    EscriptGrammar::EscriptParser::BasicForStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "for", r.start, TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  _line_parts.emplace_back( ":=", Range( *ctx->IDENTIFIER() ).end, TokenPart::SPACE );
  visitExpression( ctx->expression( 0 ) );
  _line_parts.emplace_back( "to", Range( *ctx->expression( 0 ) ).end, TokenPart::SPACE );
  visitExpression( ctx->expression( 1 ) );
  buildLine();

  ++_currident;
  visitBlock( ctx->block() );
  --_currident;

  _line_parts.emplace_back( "endfor", r.end, TokenPart::SPACE );
  buildLine();

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitCstyleForStatement(
    EscriptGrammar::EscriptParser::CstyleForStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "for", r.start, TokenPart::SPACE );
  _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression( 0 ) );
  _line_parts.emplace_back( ";", r.start,
                            TokenPart::ATTACHED | TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression( 1 ) );
  _line_parts.emplace_back( ";", r.start,
                            TokenPart::ATTACHED | TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression( 2 ) );
  _line_parts.emplace_back( ")", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );

  buildLine();

  ++_currident;
  visitBlock( ctx->block() );
  --_currident;

  _line_parts.emplace_back( "endfor", r.end, TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitRepeatStatement(
    EscriptGrammar::EscriptParser::RepeatStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  Range r( *ctx );
  _line_parts.emplace_back( "repeat", r.start, TokenPart::SPACE );
  buildLine();

  ++_currident;
  visitBlock( ctx->block() );
  --_currident;

  _line_parts.emplace_back( "until", Range( *ctx->expression() ).start, TokenPart::SPACE );
  visitExpression( ctx->expression() );
  _line_parts.emplace_back( ";", Range( *ctx->expression() ).start,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitReturnStatement(
    EscriptGrammar::EscriptParser::ReturnStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "return", r.start, TokenPart::SPACE );

  if ( auto expression = ctx->expression() )
  {
    visitExpression( expression );
  }
  _line_parts.emplace_back( ";", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitScopedFunctionCall(
    EscriptGrammar::EscriptParser::ScopedFunctionCallContext* ctx )
{
  INFO_PRINTLN( "SCOPEDFUNCTIONCALL" );
  // todo when?
  make_identifier( ctx->functionCall()->IDENTIFIER() );  // callee
  visitChildren( ctx->functionCall() );                  // arguments
  make_identifier( ctx->IDENTIFIER() );                  // scope
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionReference(
    EscriptGrammar::EscriptParser::FunctionReferenceContext* ctx )
{
  _line_parts.emplace_back( "@", Range( *ctx ).start, TokenPart::NONE );
  make_identifier( ctx->IDENTIFIER() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitGotoStatement(
    EscriptGrammar::EscriptParser::GotoStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "goto", r.start, TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  _line_parts.emplace_back( ";", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitIfStatement(
    EscriptGrammar::EscriptParser::IfStatementContext* ctx )
{
  auto blocks = ctx->block();
  auto par_expression = ctx->parExpression();
  size_t num_expressions = par_expression.size();
  for ( size_t clause_index = 0; clause_index < num_expressions; ++clause_index )
  {
    auto expression_ctx = par_expression.at( clause_index );
    Range rif( *expression_ctx );
    if ( !clause_index )
      _line_parts.emplace_back( "if", rif.start, TokenPart::SPACE );
    else
      _line_parts.emplace_back( "elseif", rif.start, TokenPart::SPACE );
    _line_parts.emplace_back( "(", rif.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
    auto expression_ast = visitExpression( expression_ctx->expression() );
    _line_parts.emplace_back( ")", rif.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
    buildLine();

    if ( blocks.size() > clause_index )
    {
      ++_currident;
      visitBlock( blocks.at( clause_index ) );
      --_currident;
    }
  }
  if ( ctx->ELSE() )
  {
    Range r( *ctx->ELSE() );
    _line_parts.emplace_back( "else", r.start, TokenPart::SPACE );
    buildLine();
    ++_currident;
    visitBlock( blocks.at( blocks.size() - 1 ) );
    --_currident;
  }
  _line_parts.emplace_back( "endif", Range( *ctx ).end, TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitIncludeDeclaration(
    EscriptGrammar::EscriptParser::IncludeDeclarationContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "include", r.start, TokenPart::SPACE );
  visitStringIdentifier( ctx->stringIdentifier() );
  _line_parts.emplace_back( ";", r.end, TokenPart::SPACE | TokenPart::ATTACHED );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitInterpolatedString(
    EscriptGrammar::EscriptParser::InterpolatedStringContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "$", r.start, TokenPart::SPACE );
  _line_parts.emplace_back( "\"", r.start, TokenPart::ATTACHED );
  visitChildren( ctx );
  _line_parts.emplace_back( "\"", r.end, TokenPart::ATTACHED | TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitInterpolatedStringPart(
    EscriptGrammar::EscriptParser::InterpolatedStringPartContext* ctx )
{
  if ( auto expression_ctx = ctx->expression() )
  {
    Range r( *expression_ctx );
    _line_parts.emplace_back( "{", r.start, TokenPart::ATTACHED );
    visitExpression( expression_ctx );

    if ( auto format_string = ctx->FORMAT_STRING() )
    {
      _line_parts.emplace_back( ":", Range( *format_string ).start, TokenPart::ATTACHED );
      _line_parts.emplace_back( format_string->getText(), Range( *format_string ).start,
                                TokenPart::ATTACHED );
    }
    _line_parts.emplace_back( "}", r.end, TokenPart::ATTACHED );
  }

  else if ( auto string_literal = ctx->STRING_LITERAL_INSIDE() )
  {
    _line_parts.emplace_back( string_literal->getText(), Range( *string_literal ).start,
                              TokenPart::ATTACHED );
  }
  else if ( auto lbrace = ctx->DOUBLE_LBRACE_INSIDE() )
  {
    _line_parts.emplace_back( "{{", Range( *lbrace ).start, TokenPart::ATTACHED );
  }
  else if ( auto rbrace = ctx->DOUBLE_RBRACE() )
  {
    _line_parts.emplace_back( "}}", Range( *rbrace ).start, TokenPart::ATTACHED );
  }
  else if ( auto escaped = ctx->REGULAR_CHAR_INSIDE() )
  {
    _line_parts.emplace_back( escaped->getText(), Range( *escaped ).start, TokenPart::ATTACHED );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBoolLiteral(
    EscriptGrammar::EscriptParser::BoolLiteralContext* ctx )
{
  if ( auto bool_false = ctx->BOOL_FALSE() )
  {
    make_bool_literal( bool_false );
  }
  else if ( auto bool_true = ctx->BOOL_TRUE() )
  {
    make_bool_literal( bool_true );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitIntegerLiteral(
    EscriptGrammar::EscriptParser::IntegerLiteralContext* ctx )
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

antlrcpp::Any PrettifyFileProcessor::visitLiteral(
    EscriptGrammar::EscriptParser::LiteralContext* ctx )
{
  if ( auto string_literal = ctx->STRING_LITERAL() )
  {
    return make_string_literal( string_literal );
  }
  else if ( auto integer_literal = ctx->integerLiteral() )
  {
    return visitIntegerLiteral( integer_literal );
  }
  else if ( auto float_literal = ctx->floatLiteral() )
  {
    return visitFloatLiteral( float_literal );
  }
  return visitChildren( ctx );
}

antlrcpp::Any PrettifyFileProcessor::visitModuleFunctionDeclaration(
    EscriptGrammar::EscriptParser::ModuleFunctionDeclarationContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );
  Range rname( *ctx->IDENTIFIER() );
  _line_parts.emplace_back( "(", rname.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  auto cursize = _line_parts.size();
  if ( auto moduleFunctionParameterList = ctx->moduleFunctionParameterList() )
  {
    visitModuleFunctionParameterList( moduleFunctionParameterList );
  }
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  Range rparam( *ctx );
  _line_parts.emplace_back( ")", rparam.end, TokenPart::NONE );
  if ( _line_parts.size() - 1 == cursize )

    _line_parts.back().style |= TokenPart::ATTACHED;
  _line_parts.emplace_back( ";", rparam.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitModuleFunctionParameter(
    EscriptGrammar::EscriptParser::ModuleFunctionParameterContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    Range re( *expression );
    _line_parts.emplace_back( ":=", re.start, TokenPart::SPACE );
    visitExpression( expression );
  }
  Range r( *ctx );
  _line_parts.emplace_back( ",", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitModuleUnit(
    EscriptGrammar::EscriptParser::ModuleUnitContext* ctx )
{
  visitChildren( ctx );
  while ( !_comments.empty() )
  {
    _lines.push_back( std::string( _currident * 2, ' ' ) + _comments.front().text );
    _comments.erase( _comments.begin() );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitPrimary(
    EscriptGrammar::EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
  {
    return visitLiteral( literal );
  }
  else if ( auto parExpression = ctx->parExpression() )
  {
    Range r( *parExpression );
    _line_parts.emplace_back( "(", r.start, TokenPart::SPACE );
    visitExpression( parExpression->expression() );

    _line_parts.emplace_back( ")", r.end, TokenPart::SPACE );
    return {};
  }
  else if ( auto functionCall = ctx->functionCall() )
  {
    return visitFunctionCall( functionCall );
  }
  else if ( auto scopedFunctionCall = ctx->scopedFunctionCall() )
  {
    return visitScopedFunctionCall( scopedFunctionCall );
  }
  else if ( auto uninit = ctx->UNINIT() )
  {
    _line_parts.emplace_back( "uninit", Range( *uninit ).start, TokenPart::SPACE );
    return {};
  }
  else if ( auto bool_true = ctx->BOOL_TRUE() )
  {
    return make_bool_literal( bool_true );
  }
  else if ( auto bool_false = ctx->BOOL_FALSE() )
  {
    return make_bool_literal( bool_false );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    return make_identifier( identifier );
  }
  else if ( auto functionReference = ctx->functionReference() )
  {
    return visitFunctionReference( functionReference );
  }
  else if ( auto explicitArrayInitializer = ctx->explicitArrayInitializer() )
  {
    return visitExplicitArrayInitializer( explicitArrayInitializer );
  }
  else if ( auto explicitStructInitializer = ctx->explicitStructInitializer() )
  {
    return visitExplicitStructInitializer( explicitStructInitializer );
  }
  else if ( auto explicitDictInitializer = ctx->explicitDictInitializer() )
  {
    return visitExplicitDictInitializer( explicitDictInitializer );
  }
  else if ( auto explicitErrorInitializer = ctx->explicitErrorInitializer() )
  {
    return visitExplicitErrorInitializer( explicitErrorInitializer );
  }
  else if ( auto bareArrayInitializer = ctx->bareArrayInitializer() )
  {
    return visitBareArrayInitializer( bareArrayInitializer );
  }
  else if ( auto interpolatedString = ctx->interpolatedString() )
  {
    return visitInterpolatedString( interpolatedString );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitProgramDeclaration(
    EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "program", r.start, TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::ATTACHED );
  auto cursize = _line_parts.size();
  visitProgramParameters( ctx->programParameters() );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  _line_parts.emplace_back( ")", r.start, TokenPart::SPACE );
  if ( _line_parts.size() - 1 == cursize )
    _line_parts.back().style |= TokenPart::ATTACHED;
  buildLine();
  ++_currident;
  visitBlock( ctx->block() );
  --_currident;
  _line_parts.emplace_back( "endprogram", r.end, TokenPart::SPACE );
  buildLine();

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitProgramParameter(
    EscriptGrammar::EscriptParser::ProgramParameterContext* ctx )
{
  auto unused = ctx->UNUSED();
  if ( unused )
  {
    Range r( *unused );
    _line_parts.emplace_back( "unused", r.start, TokenPart::SPACE );
  }

  make_identifier( ctx->IDENTIFIER() );
  if ( auto expression = ctx->expression() )
  {
    Range r( *expression );
    _line_parts.emplace_back( ":=", r.start, TokenPart::SPACE );
    visitExpression( expression );
  }
  Range r( *ctx );
  _line_parts.emplace_back( ",", r.end, TokenPart::SPACE | TokenPart::ATTACHED );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStatement(
    EscriptGrammar::EscriptParser::StatementContext* ctx )
{
  if ( auto ifStatement = ctx->ifStatement() )
  {
    return visitIfStatement( ifStatement );
  }
  else if ( auto gotoStatement = ctx->gotoStatement() )
  {
    return visitGotoStatement( gotoStatement );
  }
  else if ( auto returnStatement = ctx->returnStatement() )
  {
    return visitReturnStatement( returnStatement );
  }
  else if ( auto constStatement = ctx->constStatement() )
  {
    return visitConstStatement( constStatement );
  }
  else if ( auto varStatement = ctx->varStatement() )
  {
    return visitVarStatement( varStatement );
  }
  else if ( auto doStatement = ctx->doStatement() )
  {
    return visitDoStatement( doStatement );
  }
  else if ( auto whileStatement = ctx->whileStatement() )
  {
    return visitWhileStatement( whileStatement );
  }
  else if ( auto exitStatement = ctx->exitStatement() )
  {
    return visitExitStatement( exitStatement );
  }
  else if ( auto breakStatement = ctx->breakStatement() )
  {
    return visitBreakStatement( breakStatement );
  }
  else if ( auto continueStatement = ctx->continueStatement() )
  {
    return visitContinueStatement( continueStatement );
  }
  else if ( auto forStatement = ctx->forStatement() )
  {
    return visitForStatement( forStatement );
  }
  else if ( auto foreachStatement = ctx->foreachStatement() )
  {
    return visitForeachStatement( foreachStatement );
  }
  else if ( auto repeatStatement = ctx->repeatStatement() )
  {
    return visitRepeatStatement( repeatStatement );
  }
  else if ( auto caseStatement = ctx->caseStatement() )
  {
    return visitCaseStatement( caseStatement );
  }
  else if ( auto enumStatement = ctx->enumStatement() )
  {
    return visitEnumStatement( enumStatement );
  }
  else if ( auto expression = ctx->statementExpression )
  {
    visitExpression( expression );

    Range r( *ctx );
    _line_parts.emplace_back( ";", r.end,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    buildLine();
    return {};
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStringIdentifier(
    EscriptGrammar::EscriptParser::StringIdentifierContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    return make_identifier( identifier );
  }
  else if ( auto string_literal = ctx->STRING_LITERAL() )
  {
    return make_string_literal( string_literal );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStructInitializerExpression(
    EscriptGrammar::EscriptParser::StructInitializerExpressionContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    make_identifier( identifier );
  }
  else if ( auto string_literal = ctx->STRING_LITERAL() )
  {
    make_string_literal( string_literal );
  }

  if ( auto expression = ctx->expression() )
  {
    Range r( *expression );
    _line_parts.emplace_back( ":=", r.start, TokenPart::SPACE );
    visitExpression( expression );
  }
  Range r( *ctx );
  _line_parts.emplace_back( ",", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitUseDeclaration(
    EscriptParser::UseDeclarationContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "use", r.start, TokenPart::SPACE );
  visitStringIdentifier( ctx->stringIdentifier() );
  _line_parts.emplace_back( ";", r.end, TokenPart::SPACE | TokenPart::ATTACHED );

  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_statement_label(
    EscriptGrammar::EscriptParser::StatementLabelContext* ctx )
{
  if ( ctx )
  {
    make_identifier( ctx->IDENTIFIER() );
    Range r( *ctx->IDENTIFIER() );
    _line_parts.emplace_back( ":", r.end, TokenPart::SPACE | TokenPart::ATTACHED );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_identifier( antlr4::tree::TerminalNode* terminal )
{
  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_string_literal( antlr4::tree::TerminalNode* terminal )
{
  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_integer_literal( antlr4::tree::TerminalNode* terminal )
{
  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_float_literal( antlr4::tree::TerminalNode* terminal )
{
  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_bool_literal( antlr4::tree::TerminalNode* terminal )
{
  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return {};
}

}  // namespace Pol::Bscript::Compiler
