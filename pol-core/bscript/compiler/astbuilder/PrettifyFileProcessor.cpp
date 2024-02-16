#include "PrettifyFileProcessor.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compilercfg.h"
#include "clib/filecont.h"
#include "clib/logfacility.h"

#include <algorithm>
#include <iostream>
#include <utility>

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{

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
    load_raw_file();
    collectComments( sf );
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
    load_raw_file();
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
          if ( info.text[firstchar] == '/' || info.text.back() == '/' )
          {
            // assuming its //    // -> keep it
            info.text = std::string( "//" ) + info.text;
          }
          else
          {
            info.text.erase( 0, firstchar );  // remove remaining whitespace
            info.text = std::string( "// " ) + info.text;
          }
        }
        auto lastchar = info.text.find_last_not_of( ' ' );
        info.text.erase( info.text.begin() + lastchar + 1, info.text.end() );
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
        if ( info.text.empty() || ( info.text.front() == '*' && info.text.back() == '*' ) )
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

  for ( const auto& c : _comments )
  {
    if ( c.text.find( "format-off" ) != std::string::npos )
      _skiplines.emplace_back( c.pos, c.pos );
    else if ( c.text.find( "format-on" ) != std::string::npos )
      if ( !_skiplines.empty() )
        _skiplines.back().end = c.pos_end;
  }
  if ( !_skiplines.empty() )
  {
    // set unmatched end to file eof
    if ( _skiplines.back().start.line_number == _skiplines.back().end.line_number )
      _skiplines.back().end.line_number = _rawlines.size() + 1;
  }
  // remove all comments between noformat ranges
  _comments.erase( std::remove_if( _comments.begin(), _comments.end(),
                                   [&]( auto& c )
                                   {
                                     for ( const auto& s : _skiplines )
                                     {
                                       if ( s.start.line_number <= c.pos.line_number &&
                                            s.end.line_number >= c.pos.line_number )
                                         return true;
                                     }
                                     return false;
                                   } ),
                   _comments.end() );
}

void PrettifyFileProcessor::mergeRawContent( int nextlineno )
{
  for ( auto itr = _skiplines.begin(); itr != _skiplines.end(); )
  {
    if ( itr->start.line_number < nextlineno )
    {
      mergeCommentsBefore( itr->start.line_number );
      addEmptyLines( itr->start.line_number );
      for ( size_t i = itr->start.line_number - 1; i < itr->end.line_number && i < _rawlines.size();
            ++i )
      {
        _lines.emplace_back( _rawlines[i] );
        _last_line = i + 1;
      }
      itr = _skiplines.erase( itr );
      continue;
    }
    ++itr;
  }
}

void PrettifyFileProcessor::mergeCommentsBefore( int nextlineno )
{
  // add comments before current line
  while ( !_comments.empty() )
  {
    if ( _comments.front().pos.line_number < nextlineno )
    {
      addEmptyLines( _comments.front().pos.line_number );
      _lines.push_back( std::string( _currident * compilercfg.FormatterIdentLevel, ' ' ) +
                        _comments.front().text );
      _last_line = _comments.front().pos_end.line_number;
      _comments.erase( _comments.begin() );
    }
    else
      break;
  }
}

void PrettifyFileProcessor::mergeComments()
{
  if ( _line_parts.empty() )
    return;
  mergeRawContent( _line_parts.front().lineno );
  mergeCommentsBefore( _line_parts.front().lineno );
  // add comments inbetween
  for ( size_t i = 0; i < _line_parts.size(); )
  {
    if ( _comments.empty() )
      break;
    if ( _line_parts[i].tokenid > _comments.front().pos.token_index )
    {
      TokenPart p{ std::move( _comments.front().text ), _comments.front().pos, TokenPart::SPACE,
                   _currentgroup };
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
        addToken( std::move( _comments.front().text ), _comments.front().pos, TokenPart::SPACE );
        _comments.erase( _comments.begin() );
      }
    }
  }
}

void PrettifyFileProcessor::buildLine()
{
  if ( _line_parts.empty() )
    return;
  auto stripline = []( std::string& line )
  {
    if ( !line.empty() )
    {
      auto lastchar = line.find_last_not_of( ' ' );
      line.erase( line.begin() + lastchar + 1, line.end() );
    }
  };
  mergeComments();

  // fill lines with final strings splitted at breakpoints
  std::vector<std::tuple<std::string, bool, size_t>> lines;
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
      lines.emplace_back( std::make_tuple( std::move( line ), true, _line_parts[i].group ) );
      line.clear();
    }
    // start a new line if breakpoint
    else if ( _line_parts[i].style & TokenPart::BREAKPOINT )
    {
      // if the next part is attached, dont break now and instead later
      // eg dont split blubb[1]()
      bool skip = false;
      if ( i + 1 < _line_parts.size() )
      {
        // next one is attached
        if ( _line_parts[i + 1].style & TokenPart::ATTACHED )
        {
          // search for space or breakpoint
          for ( size_t j = i + 1; j < _line_parts.size(); ++j )
          {
            if ( _line_parts[j].style & TokenPart::ATTACHED )
              skip = true;
            else if ( _line_parts[j].style & TokenPart::SPACE ||
                      _line_parts[j].style & TokenPart::BREAKPOINT )
            {
              _line_parts[j].style |= TokenPart::BREAKPOINT;
              skip = true;
              break;
            }
          }
        }
      }
      if ( skip )
        continue;
      lines.emplace_back( std::make_tuple( std::move( line ), false, _line_parts[i].group ) );
      line.clear();
    }
  }
  // add remainings
  if ( !line.empty() )
  {
    lines.emplace_back( std::make_tuple( std::move( line ), false, _line_parts.back().group ) );
    line.clear();
  }
  INFO_PRINTLN( "BREAK " );
  for ( auto& [l, forced, group] : lines )
    INFO_PRINTLN( "\"{}\" {}", l, group );
  // add newline from original sourcecode
  addEmptyLines( _line_parts.front().lineno );

  // sum up linelength, are groups inside
  bool groups = false;
  size_t linelength = 0;
  for ( auto& [l, forced, group] : lines )
  {
    if ( group != 0 )
      groups = true;
    linelength += l.size();
  }

  // split based on groups
  // TODO groups could be the best for variable declarations
  // other option eg function declaration
  if ( groups && linelength > compilercfg.FormatterLineWidth )
  {
    std::vector<size_t> alignmentspace = {};
    size_t lastgroup = 0xffffFFFF;
    for ( auto& [l, forced, group] : lines )
    {
      if ( lastgroup == 0xffffFFFF )
      {
        line += l;  // first part
        alignmentspace.push_back( line.size() );
      }
      else if ( lastgroup < group )  // new group
      {
        // first store current size as alignment
        alignmentspace.push_back( line.size() );
        line += l;
      }
      else if ( lastgroup > group )  // descending
      {
        _lines.emplace_back( std::move( line ) );
        // use last group alignment
        line = std::string( alignmentspace.back(), ' ' );
        // remove last group alignment
        alignmentspace.pop_back();
        line += l;
      }
      else  // same group
      {
        _lines.emplace_back( std::move( line ) );
        // use current group alignment
        line = std::string( alignmentspace.back(), ' ' );
        line += l;
      }

      lastgroup = group;
    }
  }
  else
  {
    // build final lines
    size_t alignmentspace = 0;
    for ( auto& [l, forced, group] : lines )
    {
      // following lines need to be aligned
      if ( line.empty() && alignmentspace )
        line = std::string( alignmentspace, ' ' );
      // first breakpoint defines the alignment and add initial ident level
      if ( !alignmentspace )
      {
        alignmentspace = l.size() + _currident * compilercfg.FormatterIdentLevel;
        line += std::string( _currident * compilercfg.FormatterIdentLevel, ' ' );
      }
      line += l;
      // linewidth reached add current line, start a new one
      if ( line.size() > compilercfg.FormatterLineWidth || forced )
      {
        stripline( line );
        _lines.emplace_back( std::move( line ) );
        line.clear();
      }
    }
  }
  if ( !line.empty() )
  {
    stripline( line );
    _lines.emplace_back( std::move( line ) );
  }
  _last_line = _line_parts.back().lineno;
  _line_parts.clear();
}


void PrettifyFileProcessor::addEmptyLines( int line_number )
{
  if ( compilercfg.FormatterMergeEmptyLines )
  {
    if ( line_number > _last_line + 1 )
      _lines.emplace_back( "" );
  }
  else
  {
    while ( line_number > _last_line + 1 )
    {
      _lines.emplace_back( "" );
      ++_last_line;
    }
  }
}

antlrcpp::Any PrettifyFileProcessor::visitCompilationUnit(
    EscriptGrammar::EscriptParser::CompilationUnitContext* ctx )
{
  visitChildren( ctx );
  mergeRawContent( _last_line );
  while ( !_comments.empty() )
  {
    addEmptyLines( _comments.front().pos.line_number );
    _lines.push_back( std::string( _currident * compilercfg.FormatterIdentLevel, ' ' ) +
                      _comments.front().text );
    _last_line = _comments.front().pos_end.line_number;
    mergeRawContent( _last_line );
    _comments.erase( _comments.begin() );
  }
  if ( !_line_parts.empty() )
    report.error( source_file_identifier, "left over formatting lines {}", _line_parts );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitVarStatement(
    EscriptGrammar::EscriptParser::VarStatementContext* ctx )
{
  addToken( "var", ctx->VAR(), TokenPart::SPACE );
  visitVariableDeclarationList( ctx->variableDeclarationList() );
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitParExpression(
    EscriptGrammar::EscriptParser::ParExpressionContext* ctx )
{
  addToken( "(", ctx->LPAREN(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression() );
  addToken( ")", ctx->RPAREN(), TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBlock( EscriptGrammar::EscriptParser::BlockContext* ctx )
{
  ++_currident;
  visitChildren( ctx );
  --_currident;
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitWhileStatement(
    EscriptGrammar::EscriptParser::WhileStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "while", ctx->WHILE(), TokenPart::SPACE );
  visitParExpression( ctx->parExpression() );
  buildLine();

  visitBlock( ctx->block() );
  addToken( "endwhile", ctx->ENDWHILE(), TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitVariableDeclarationList(
    EscriptGrammar::EscriptParser::VariableDeclarationListContext* ctx )
{
  auto args = ctx->variableDeclaration();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    visitVariableDeclaration( args[i] );
    if ( i < args.size() - 1 )
      addToken( ",", ctx->COMMA( i ),
                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitVariableDeclaration(
    EscriptGrammar::EscriptParser::VariableDeclarationContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  if ( auto variable_declaration_initializer = ctx->variableDeclarationInitializer() )
  {
    if ( auto assign = variable_declaration_initializer->ASSIGN() )
      addToken( ":=", assign, TokenPart::SPACE );

    if ( auto expression = variable_declaration_initializer->expression() )
      visitExpression( expression );
    else if ( auto arr = variable_declaration_initializer->ARRAY() )
      addToken( "array", arr, TokenPart::SPACE );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitConstStatement(
    EscriptGrammar::EscriptParser::ConstStatementContext* ctx )
{
  addToken( "const", ctx->TOK_CONST(), TokenPart::SPACE );
  auto const_declaration_ctx = ctx->constantDeclaration();
  make_identifier( const_declaration_ctx->IDENTIFIER() );

  if ( auto variable_declaration_initializer =
           const_declaration_ctx->variableDeclarationInitializer() )
  {
    if ( auto assign = variable_declaration_initializer->ASSIGN() )
      addToken( ":=", assign, TokenPart::SPACE );
    if ( auto expression = variable_declaration_initializer->expression() )
      visitExpression( expression );
    else if ( auto array = variable_declaration_initializer->ARRAY() )
      addToken( "array", array, TokenPart::SPACE );
  }
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDictInitializerExpression(
    EscriptGrammar::EscriptParser::DictInitializerExpressionContext* ctx )
{
  visitExpression( ctx->expression( 0 ) );  // key

  if ( ctx->expression().size() > 1 )
  {
    addToken( "->", ctx->ARROW(), TokenPart::SPACE );
    visitExpression( ctx->expression( 1 ) );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDoStatement(
    EscriptGrammar::EscriptParser::DoStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "do", ctx->DO(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();

  visitBlock( ctx->block() );

  addToken( "dowhile", ctx->DOWHILE(), TokenPart::SPACE );
  visitParExpression( ctx->parExpression() );
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::BREAKPOINT | TokenPart::ATTACHED );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitEnumListEntry(
    EscriptGrammar::EscriptParser::EnumListEntryContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), TokenPart::SPACE );
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
      addToken( ",", ctx->COMMA( i ), TokenPart::SPACE | TokenPart::ATTACHED );

    buildLine();
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitEnumStatement(
    EscriptGrammar::EscriptParser::EnumStatementContext* ctx )
{
  addToken( "enum", ctx->ENUM(), TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  buildLine();
  ++_currident;
  auto enums = visitEnumList( ctx->enumList() );
  --_currident;

  addToken( "endenum", ctx->ENDENUM(), TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExitStatement(
    EscriptGrammar::EscriptParser::ExitStatementContext* ctx )
{
  addToken( "exit", ctx->EXIT(), TokenPart::SPACE );
  addToken( ";", ctx->SEMI(), TokenPart::ATTACHED | TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitArrayInitializer(
    EscriptGrammar::EscriptParser::ArrayInitializerContext* ctx )
{
  if ( auto lbrace = ctx->LBRACE() )
    addToken( "{", lbrace, TokenPart::ATTACHED | TokenPart::SPACE | TokenPart::BREAKPOINT );
  else if ( auto lparen = ctx->LPAREN() )
    addToken( "(", lparen, TokenPart::ATTACHED | TokenPart::SPACE | TokenPart::BREAKPOINT );

  ++_currentgroup;
  size_t curcount = _line_parts.size();
  if ( auto expr = ctx->expressionList() )
    visitExpressionList( expr );
  --_currentgroup;

  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == curcount )
    style |= TokenPart::ATTACHED;
  if ( auto rbrace = ctx->RBRACE() )
    addToken( "}", rbrace, style );
  else if ( auto rparen = ctx->RPAREN() )
    addToken( ")", rparen, style );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitArrayInitializer(
    EscriptGrammar::EscriptParser::ExplicitArrayInitializerContext* ctx )
{
  ++_currentgroup;
  addToken( "array", ctx->ARRAY(), TokenPart::SPACE );
  if ( auto init = ctx->arrayInitializer() )
    visitArrayInitializer( init );
  --_currentgroup;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDictInitializerExpressionList(
    EscriptGrammar::EscriptParser::DictInitializerExpressionListContext* ctx )
{
  auto inits = ctx->dictInitializerExpression();
  ++_currentgroup;
  for ( size_t i = 0; i < inits.size(); ++i )
  {
    visitDictInitializerExpression( inits[i] );
    if ( i < inits.size() - 1 )
      addToken( ",", ctx->COMMA( i ),
                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  --_currentgroup;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitDictInitializer(
    EscriptGrammar::EscriptParser::DictInitializerContext* ctx )
{
  addToken( "{", ctx->LBRACE(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  if ( auto expr = ctx->dictInitializerExpressionList() )
    visitDictInitializerExpressionList( expr );

  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == curcount )
    style |= TokenPart::ATTACHED;
  addToken( "}", ctx->RBRACE(), style );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitDictInitializer(
    EscriptGrammar::EscriptParser::ExplicitDictInitializerContext* ctx )
{
  ++_currentgroup;
  addToken( "dictionary", ctx->DICTIONARY(), TokenPart::SPACE );
  if ( auto init = ctx->dictInitializer() )
    visitDictInitializer( init );
  --_currentgroup;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStructInitializerExpressionList(
    EscriptGrammar::EscriptParser::StructInitializerExpressionListContext* ctx )
{
  auto inits = ctx->structInitializerExpression();
  for ( size_t i = 0; i < inits.size(); ++i )
  {
    visitStructInitializerExpression( inits[i] );
    if ( i < inits.size() - 1 )
      addToken( ",", ctx->COMMA( i ),
                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStructInitializer(
    EscriptGrammar::EscriptParser::StructInitializerContext* ctx )
{
  addToken( "{", ctx->LBRACE(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  ++_currentgroup;
  size_t curcount = _line_parts.size();
  if ( auto expr = ctx->structInitializerExpressionList() )
    visitStructInitializerExpressionList( expr );
  --_currentgroup;

  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == curcount )
    style |= TokenPart::ATTACHED;
  addToken( "}", ctx->RBRACE(), style );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitErrorInitializer(
    EscriptGrammar::EscriptParser::ExplicitErrorInitializerContext* ctx )
{
  addToken( "error", ctx->TOK_ERROR(), TokenPart::SPACE );
  if ( auto init = ctx->structInitializer() )
    visitStructInitializer( init );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBareArrayInitializer(
    EscriptGrammar::EscriptParser::BareArrayInitializerContext* ctx )
{
  addToken( "{", ctx->LBRACE(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  ++_currentgroup;
  size_t curcount = _line_parts.size();
  if ( auto expr = ctx->expressionList() )
    visitExpressionList( expr );
  --_currentgroup;
  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == curcount )
    style |= TokenPart::ATTACHED;
  addToken( "}", ctx->RBRACE(), style );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExplicitStructInitializer(
    EscriptGrammar::EscriptParser::ExplicitStructInitializerContext* ctx )
{
  ++_currentgroup;
  addToken( "struct", ctx->STRUCT(), TokenPart::SPACE );
  if ( auto init = ctx->structInitializer() )
    visitStructInitializer( init );
  --_currentgroup;
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitExpressionList(
    EscriptGrammar::EscriptParser::ExpressionListContext* ctx )
{
  auto args = ctx->expression();
  for ( size_t i = 0; i < args.size(); ++i )
  {
    visitExpression( args[i] );
    if ( i < args.size() - 1 )
      addToken( ",", ctx->COMMA( i ),
                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
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
    addToken( ctx->prefix->getText(), ctx->prefix,
              ctx->prefix->getType() == EscriptGrammar::EscriptLexer::BANG_B ? TokenPart::SPACE
                                                                             : TokenPart::NONE );
    visitExpression( ctx->expression( 0 ) );
  }
  else if ( ctx->postfix )
  {
    visitExpression( ctx->expression( 0 ) );
    addToken( ctx->postfix->getText(), ctx->postfix, TokenPart::SPACE | TokenPart::ATTACHED );
  }
  else if ( ctx->bop && ctx->expression().size() == 2 )
  {
    visitExpression( ctx->expression( 0 ) );  // left
    if ( ctx->ELVIS() )                       // break before elvis
      _line_parts.back().style |= TokenPart::BREAKPOINT;
    int style = TokenPart::NONE;
    if ( ctx->OR_A() || ctx->OR_B() || ctx->AND_A() || ctx->AND_B() )
      style = TokenPart::SPACE | TokenPart::BREAKPOINT;
    else if ( ctx->ADDMEMBER() || ctx->DELMEMBER() || ctx->CHKMEMBER() )
      style = TokenPart::ATTACHED;
    else
      style = TokenPart::SPACE;
    addToken( ctx->bop->getText(), ctx->bop, style );

    visitExpression( ctx->expression( 1 ) );  // right
  }
  else if ( auto suffix = ctx->expressionSuffix() )
  {
    expression_suffix( ctx->expression( 0 ), suffix );
  }
  else if ( ctx->QUESTION() )
  {
    visitExpression( ctx->expression( 0 ) );  // conditional
    addToken( "?", ctx->QUESTION(), TokenPart::SPACE | TokenPart::BREAKPOINT );
    visitExpression( ctx->expression( 1 ) );  // consequent
    addToken( ":", ctx->COLON(), TokenPart::SPACE | TokenPart::BREAKPOINT );

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
    visitExpression( expr_ctx );
    addToken( "[", indexing->LBRACK(), TokenPart::ATTACHED );
    visitExpressionList( indexing->expressionList() );
    addToken( "]", indexing->RBRACK(), TokenPart::SPACE | TokenPart::ATTACHED );
  }
  else if ( auto member = expr_suffix_ctx->navigationSuffix() )
  {
    visitExpression( expr_ctx );
    addToken( ".", member->DOT(), TokenPart::ATTACHED );
    if ( auto string_literal = member->STRING_LITERAL() )
      make_string_literal( string_literal );
    else if ( auto identifier = member->IDENTIFIER() )
      make_identifier( identifier );
  }
  else if ( auto method = expr_suffix_ctx->methodCallSuffix() )
  {
    visitExpression( expr_ctx );
    addToken( ".", method->DOT(), TokenPart::ATTACHED );
    make_identifier( method->IDENTIFIER() );
    addToken( "(", method->LPAREN(),
              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    size_t curcount = _line_parts.size();
    if ( auto expr = method->expressionList() )
      visitExpressionList( expr );
    auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
    if ( _line_parts.size() == curcount )
      style |= TokenPart::ATTACHED;

    addToken( ")", method->RPAREN(), style );
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
    visitParExpression( parExpression );
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
  make_statement_label( ctx->statementLabel() );
  addToken( "foreach", ctx->FOREACH(), TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  addToken( "in", ctx->TOK_IN(), TokenPart::SPACE );
  visitForeachIterableExpression( ctx->foreachIterableExpression() );
  buildLine();

  visitBlock( ctx->block() );

  addToken( "endforeach", ctx->ENDFOREACH(), TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitForStatement(
    EscriptGrammar::EscriptParser::ForStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );

  auto forGroup = ctx->forGroup();

  addToken( "for", ctx->FOR(), TokenPart::SPACE );
  if ( auto basicForStatement = forGroup->basicForStatement() )
    visitBasicForStatement( basicForStatement );
  else if ( auto cstyleForStatement = forGroup->cstyleForStatement() )
    visitCstyleForStatement( cstyleForStatement );
  addToken( "endfor", ctx->ENDFOR(), TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionCall(
    EscriptGrammar::EscriptParser::FunctionCallContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  addToken( "(", ctx->LPAREN(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  size_t curcount = _line_parts.size();
  if ( auto args = ctx->expressionList() )
    visitExpressionList( args );

  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == curcount )
    style |= TokenPart::ATTACHED;
  addToken( ")", ctx->RPAREN(), style );
  return {};
}
antlrcpp::Any PrettifyFileProcessor::visitFunctionDeclaration(
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  if ( auto exported = ctx->EXPORTED() )
    addToken( "exported", exported, TokenPart::SPACE );
  addToken( "function", ctx->FUNCTION(), TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  visitFunctionParameters( ctx->functionParameters() );

  visitBlock( ctx->block() );

  addToken( "endfunction", ctx->ENDFUNCTION(), TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionParameters(
    EscriptGrammar::EscriptParser::FunctionParametersContext* ctx )
{
  addToken( "(", ctx->LPAREN(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  size_t curcount = _line_parts.size();
  if ( auto args = ctx->functionParameterList() )
    visitFunctionParameterList( args );

  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == curcount )
    style |= TokenPart::ATTACHED;
  addToken( ")", ctx->RPAREN(), style );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionParameterList(
    EscriptGrammar::EscriptParser::FunctionParameterListContext* ctx )
{
  auto params = ctx->functionParameter();
  for ( size_t i = 0; i < params.size(); ++i )
  {
    visitFunctionParameter( params[i] );
    if ( i < params.size() - 1 )
      addToken( ",", ctx->COMMA( i ),
                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionParameter(
    EscriptGrammar::EscriptParser::FunctionParameterContext* ctx )
{
  if ( auto byref = ctx->BYREF() )
    addToken( "byref", byref, TokenPart::SPACE );
  if ( auto unused = ctx->UNUSED() )
    addToken( "unused", unused, TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), TokenPart::SPACE );
    visitExpression( expression );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBreakStatement(
    EscriptGrammar::EscriptParser::BreakStatementContext* ctx )
{
  addToken( "break", ctx->BREAK(), TokenPart::SPACE );
  if ( auto identifier = ctx->IDENTIFIER() )
    make_identifier( identifier );
  addToken( ";", ctx->SEMI(), TokenPart::ATTACHED );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitSwitchBlockStatementGroup(
    EscriptGrammar::EscriptParser::SwitchBlockStatementGroupContext* ctx )
{
  for ( const auto& switchLabel : ctx->switchLabel() )
  {
    visitSwitchLabel( switchLabel );
    buildLine();
  }
  visitBlock( ctx->block() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitSwitchLabel(
    EscriptGrammar::EscriptParser::SwitchLabelContext* ctx )
{
  if ( auto integerLiteral = ctx->integerLiteral() )
    visitIntegerLiteral( integerLiteral );
  else if ( auto boolLiteral = ctx->boolLiteral() )
    visitBoolLiteral( boolLiteral );
  else if ( auto uninit = ctx->UNINIT() )
    addToken( "uninit", uninit, TokenPart::SPACE );
  else if ( auto identifier = ctx->IDENTIFIER() )
    make_identifier( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    make_string_literal( string_literal );
  else if ( auto defaultctx = ctx->DEFAULT() )
    addToken( "default", defaultctx, TokenPart::SPACE );

  addToken( ":", ctx->COLON(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitCaseStatement(
    EscriptGrammar::EscriptParser::CaseStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "case", ctx->CASE(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  addToken( "(", ctx->LPAREN(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression() );
  addToken( ")", ctx->RPAREN(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  ++_currident;
  for ( const auto& switchBlockStatementGroup : ctx->switchBlockStatementGroup() )
  {
    visitSwitchBlockStatementGroup( switchBlockStatementGroup );
  }
  --_currident;
  addToken( "endcase", ctx->ENDCASE(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitContinueStatement(
    EscriptGrammar::EscriptParser::ContinueStatementContext* ctx )
{
  addToken( "continue", ctx->CONTINUE(), TokenPart::SPACE );

  if ( auto identifier = ctx->IDENTIFIER() )
    make_identifier( identifier );

  addToken( ";", ctx->SEMI(), TokenPart::ATTACHED );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBasicForStatement(
    EscriptGrammar::EscriptParser::BasicForStatementContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );
  addToken( ":=", ctx->ASSIGN(), TokenPart::SPACE );
  visitExpression( ctx->expression( 0 ) );
  addToken( "to", ctx->TO(), TokenPart::SPACE );
  visitExpression( ctx->expression( 1 ) );
  buildLine();

  visitBlock( ctx->block() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitCstyleForStatement(
    EscriptGrammar::EscriptParser::CstyleForStatementContext* ctx )
{
  addToken( "(", ctx->LPAREN(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression( 0 ) );
  addToken( ";", ctx->SEMI( 0 ), TokenPart::ATTACHED | TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression( 1 ) );
  addToken( ";", ctx->SEMI( 1 ), TokenPart::ATTACHED | TokenPart::SPACE | TokenPart::BREAKPOINT );
  visitExpression( ctx->expression( 2 ) );
  addToken( ")", ctx->RPAREN(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();

  visitBlock( ctx->block() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitRepeatStatement(
    EscriptGrammar::EscriptParser::RepeatStatementContext* ctx )
{
  make_statement_label( ctx->statementLabel() );
  addToken( "repeat", ctx->REPEAT(), TokenPart::SPACE );
  buildLine();

  visitBlock( ctx->block() );

  addToken( "until", ctx->UNTIL(), TokenPart::SPACE );
  visitExpression( ctx->expression() );
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitReturnStatement(
    EscriptGrammar::EscriptParser::ReturnStatementContext* ctx )
{
  addToken( "return", ctx->RETURN(), TokenPart::SPACE );
  if ( auto expression = ctx->expression() )
    visitExpression( expression );
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitScopedFunctionCall(
    EscriptGrammar::EscriptParser::ScopedFunctionCallContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );  // scope
  addToken( "::", ctx->COLONCOLON(), TokenPart::ATTACHED );
  visitFunctionCall( ctx->functionCall() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitFunctionReference(
    EscriptGrammar::EscriptParser::FunctionReferenceContext* ctx )
{
  addToken( "@", ctx->AT(), TokenPart::NONE );
  make_identifier( ctx->IDENTIFIER() );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitGotoStatement(
    EscriptGrammar::EscriptParser::GotoStatementContext* ctx )
{
  addToken( "goto", ctx->GOTO(), TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
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
    if ( !clause_index )
      addToken( "if", ctx->IF(), TokenPart::SPACE );
    else
      addToken( "elseif", ctx->ELSEIF( clause_index - 1 ), TokenPart::SPACE );
    auto expression_ast = visitParExpression( expression_ctx );
    if ( !clause_index && ctx->THEN() )
      addToken( "then", ctx->THEN(), TokenPart::SPACE );

    buildLine();

    if ( blocks.size() > clause_index )
      visitBlock( blocks.at( clause_index ) );
  }
  if ( ctx->ELSE() )
  {
    addToken( "else", ctx->ELSE(), TokenPart::SPACE );
    buildLine();
    visitBlock( blocks.at( blocks.size() - 1 ) );
  }
  addToken( "endif", ctx->ENDIF(), TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitIncludeDeclaration(
    EscriptGrammar::EscriptParser::IncludeDeclarationContext* ctx )
{
  addToken( "include", ctx->INCLUDE(), TokenPart::SPACE );
  visitStringIdentifier( ctx->stringIdentifier() );
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::ATTACHED );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitInterpolatedString(
    EscriptGrammar::EscriptParser::InterpolatedStringContext* ctx )
{
  addToken( "$\"", ctx->INTERPOLATED_STRING_START(), TokenPart::SPACE );
  visitChildren( ctx );
  addToken( "\"", ctx->DOUBLE_QUOTE_INSIDE(), TokenPart::ATTACHED | TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitInterpolatedStringPart(
    EscriptGrammar::EscriptParser::InterpolatedStringPartContext* ctx )
{
  if ( auto expression_ctx = ctx->expression() )
  {
    addToken( "{", ctx->LBRACE_INSIDE(), TokenPart::ATTACHED );
    visitExpression( expression_ctx );

    if ( auto format_string = ctx->FORMAT_STRING() )
    {
      addToken( ":", ctx->COLON(), TokenPart::ATTACHED );
      addToken( format_string->getText(), format_string, TokenPart::ATTACHED );
    }
    // TODO: this seems to be the only non token
    Range r( *expression_ctx );
    addToken( "}", r.end, TokenPart::ATTACHED );
  }

  else if ( auto string_literal = ctx->STRING_LITERAL_INSIDE() )
  {
    addToken( string_literal->getText(), string_literal, TokenPart::ATTACHED );
  }
  else if ( auto lbrace = ctx->DOUBLE_LBRACE_INSIDE() )
  {
    addToken( "{{", lbrace, TokenPart::ATTACHED );
  }
  else if ( auto rbrace = ctx->DOUBLE_RBRACE() )
  {
    addToken( "}}", rbrace, TokenPart::ATTACHED );
  }
  else if ( auto escaped = ctx->REGULAR_CHAR_INSIDE() )
  {
    addToken( escaped->getText(), escaped, TokenPart::ATTACHED );
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitBoolLiteral(
    EscriptGrammar::EscriptParser::BoolLiteralContext* ctx )
{
  if ( auto bool_false = ctx->BOOL_FALSE() )
    make_bool_literal( bool_false );
  else if ( auto bool_true = ctx->BOOL_TRUE() )
    make_bool_literal( bool_true );
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
    return make_string_literal( string_literal );
  else if ( auto integer_literal = ctx->integerLiteral() )
    return visitIntegerLiteral( integer_literal );
  else if ( auto float_literal = ctx->floatLiteral() )
    return visitFloatLiteral( float_literal );
  else if ( auto uninit = ctx->UNINIT() )
    addToken( "uninit", uninit, TokenPart::SPACE );
  else if ( auto bool_literal = ctx->boolLiteral() )
    return visitBoolLiteral( bool_literal );
  return visitChildren( ctx );
}

antlrcpp::Any PrettifyFileProcessor::visitModuleFunctionDeclaration(
    EscriptGrammar::EscriptParser::ModuleFunctionDeclarationContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );
  addToken( "(", ctx->LPAREN(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  size_t curcount = _line_parts.size();
  if ( auto moduleFunctionParameterList = ctx->moduleFunctionParameterList() )
    visitModuleFunctionParameterList( moduleFunctionParameterList );

  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == curcount )
    style |= TokenPart::ATTACHED;
  addToken( ")", ctx->RPAREN(), style );
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitModuleFunctionParameterList(
    EscriptGrammar::EscriptParser::ModuleFunctionParameterListContext* ctx )
{
  auto params = ctx->moduleFunctionParameter();
  for ( size_t i = 0; i < params.size(); ++i )
  {
    visitModuleFunctionParameter( params[i] );
    if ( i < params.size() - 1 )
      addToken( ",", ctx->COMMA( i ),
                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitModuleFunctionParameter(
    EscriptGrammar::EscriptParser::ModuleFunctionParameterContext* ctx )
{
  make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), TokenPart::SPACE );
    visitExpression( expression );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitModuleUnit(
    EscriptGrammar::EscriptParser::ModuleUnitContext* ctx )
{
  visitChildren( ctx );
  while ( !_comments.empty() )
  {
    _lines.push_back( std::string( _currident * compilercfg.FormatterIdentLevel, ' ' ) +
                      _comments.front().text );
    _comments.erase( _comments.begin() );
  }
  if ( !_line_parts.empty() )
    report.error( source_file_identifier, "left over formatting lines {}", _line_parts );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitPrimary(
    EscriptGrammar::EscriptParser::PrimaryContext* ctx )
{
  if ( auto literal = ctx->literal() )
    return visitLiteral( literal );
  else if ( auto parExpression = ctx->parExpression() )
    return visitParExpression( parExpression );
  else if ( auto functionCall = ctx->functionCall() )
    return visitFunctionCall( functionCall );
  else if ( auto scopedFunctionCall = ctx->scopedFunctionCall() )
    return visitScopedFunctionCall( scopedFunctionCall );
  else if ( auto identifier = ctx->IDENTIFIER() )
    return make_identifier( identifier );
  else if ( auto functionReference = ctx->functionReference() )
    return visitFunctionReference( functionReference );
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
    EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx )
{
  addToken( "program", ctx->PROGRAM(), TokenPart::SPACE );
  make_identifier( ctx->IDENTIFIER() );
  visitProgramParameters( ctx->programParameters() );
  buildLine();
  visitBlock( ctx->block() );
  addToken( "endprogram", ctx->ENDPROGRAM(), TokenPart::SPACE );
  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitProgramParameterList(
    EscriptGrammar::EscriptParser::ProgramParameterListContext* ctx )
{
  auto params = ctx->programParameter();
  for ( size_t i = 0; i < params.size(); ++i )
  {
    visitProgramParameter( params[i] );
    if ( i < params.size() - 1 )
      addToken( ",", ctx->COMMA( i ),
                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitProgramParameters(
    EscriptGrammar::EscriptParser::ProgramParametersContext* ctx )
{
  addToken( "(", ctx->LPAREN(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  size_t curcount = _line_parts.size();
  if ( auto programParameterList = ctx->programParameterList() )
    visitProgramParameterList( programParameterList );

  auto style = TokenPart::SPACE | TokenPart::BREAKPOINT;
  if ( _line_parts.size() == curcount )
    style |= TokenPart::ATTACHED;
  addToken( ")", ctx->RPAREN(), style );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitProgramParameter(
    EscriptGrammar::EscriptParser::ProgramParameterContext* ctx )
{
  if ( auto unused = ctx->UNUSED() )
    addToken( "unused", unused, TokenPart::SPACE );

  make_identifier( ctx->IDENTIFIER() );
  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), TokenPart::SPACE );
    visitExpression( expression );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStatement(
    EscriptGrammar::EscriptParser::StatementContext* ctx )
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

    addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    buildLine();
    return {};
  }

  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStringIdentifier(
    EscriptGrammar::EscriptParser::StringIdentifierContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
    return make_identifier( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    return make_string_literal( string_literal );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitStructInitializerExpression(
    EscriptGrammar::EscriptParser::StructInitializerExpressionContext* ctx )
{
  if ( auto identifier = ctx->IDENTIFIER() )
    make_identifier( identifier );
  else if ( auto string_literal = ctx->STRING_LITERAL() )
    make_string_literal( string_literal );

  if ( auto expression = ctx->expression() )
  {
    addToken( ":=", ctx->ASSIGN(), TokenPart::SPACE );
    visitExpression( expression );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::visitUseDeclaration(
    EscriptParser::UseDeclarationContext* ctx )
{
  addToken( "use", ctx->USE(), TokenPart::SPACE );
  visitStringIdentifier( ctx->stringIdentifier() );
  addToken( ";", ctx->SEMI(), TokenPart::SPACE | TokenPart::ATTACHED );

  buildLine();
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_statement_label(
    EscriptGrammar::EscriptParser::StatementLabelContext* ctx )
{
  if ( ctx )
  {
    make_identifier( ctx->IDENTIFIER() );
    addToken( ":", ctx->COLON(), TokenPart::SPACE | TokenPart::ATTACHED );
  }
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_identifier( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_string_literal( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_integer_literal( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_float_literal( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, TokenPart::SPACE );
  return {};
}

antlrcpp::Any PrettifyFileProcessor::make_bool_literal( antlr4::tree::TerminalNode* terminal )
{
  addToken( terminal->getText(), terminal, TokenPart::SPACE );
  return {};
}

void PrettifyFileProcessor::addToken( std::string&& text, const Position& pos, int style )
{
  for ( const auto& skip : _skiplines )
    if ( skip.contains( pos ) )
      return;
  _line_parts.emplace_back( std::forward<std::string>( text ), pos, style, _currentgroup );
}
void PrettifyFileProcessor::addToken( std::string&& text, antlr4::tree::TerminalNode* terminal,
                                      int style )
{
  if ( compilercfg.FormatterKeepKeywords )
    addToken( terminal->getSymbol()->getText(), Range( *terminal ).start, style );
  else
    addToken( std::forward<std::string>( text ), Range( *terminal ).start, style );
}

void PrettifyFileProcessor::addToken( std::string&& text, antlr4::Token* token, int style )
{
  if ( compilercfg.FormatterKeepKeywords )
    addToken( token->getText(), Range( token ).start, style );
  else
    addToken( std::forward<std::string>( text ), Range( token ).start, style );
}

void PrettifyFileProcessor::load_raw_file()
{
  Clib::FileContents fc( source_file_identifier.pathname.c_str(), true );
  const auto& contents = fc.str_contents();
  std::string currline;
  for ( size_t i = 0; i < contents.size(); ++i )
  {
    if ( contents[i] == '\r' && i + 1 < contents.size() && contents[i + 1] == '\n' )
    {
      ++i;
      _rawlines.emplace_back( std::move( currline ) );
      currline.clear();
      continue;
    }
    if ( contents[i] == '\r' || contents[i] == '\n' )
    {
      _rawlines.emplace_back( std::move( currline ) );
      currline.clear();
    }
    else
      currline += contents[i];
  }
  if ( !currline.empty() )
    _rawlines.emplace_back( std::move( currline ) );
}
}  // namespace Pol::Bscript::Compiler

fmt::format_context::iterator fmt::formatter<Pol::Bscript::Compiler::TokenPart>::format(
    const Pol::Bscript::Compiler::TokenPart& t, fmt::format_context& ctx ) const
{
  return fmt::formatter<std::string>::format(
      fmt::format( "{} ({}:{}:{})", t.text, t.style, t.lineno, t.tokenid ), ctx );
}
