#include "JsonAstFileProcessor.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Statement.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/JsonAst.h"
#include "clib/logfacility.h"

#include <picojson/picojson.h>

#include <iostream>

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
const int IDENT_LEVEL = 2;
const int LINEWIDTH = 40;

JsonAstFileProcessor::JsonAstFileProcessor( const SourceFileIdentifier& source_file_identifier,
                                            Profile& profile, Report& report )
    : source_file_identifier( source_file_identifier ), profile( profile ), report( report )
{
}

antlrcpp::Any JsonAstFileProcessor::process_compilation_unit( SourceFile& sf )
{
  if ( auto compilation_unit = sf.get_compilation_unit( report, source_file_identifier ) )
  {
    collectComments( sf );
    return compilation_unit->accept( this );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}

antlrcpp::Any JsonAstFileProcessor::process_module_unit( SourceFile& sf )
{
  if ( auto module_unit = sf.get_module_unit( report, source_file_identifier ) )
  {
    collectComments( sf );
    return module_unit->accept( this );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}
std::string JsonAstFileProcessor::pretty()
{
  return fmt::format( "{}", fmt::join( _lines, "\n" ) );
}

void JsonAstFileProcessor::collectComments( SourceFile& sf )
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
antlrcpp::Any JsonAstFileProcessor::defaultResult()
{
  return picojson::value( picojson::array_type, false );
}

antlrcpp::Any JsonAstFileProcessor::aggregateResult( antlrcpp::Any /*picojson::array*/ aggregate,
                                                     antlrcpp::Any /*picojson::array*/ nextResult )
{
  auto* accum = std::any_cast<picojson::value>( &aggregate );

  if ( accum->is<picojson::array>() )
  {
    auto* next_res = std::any_cast<picojson::value>( &nextResult );
    if ( !next_res )
      return aggregate;
    auto& a = accum->get<picojson::array>();
    if ( next_res->is<picojson::array>() )
    {
      auto& b = next_res->get<picojson::array>();
      a.insert( a.end(), std::make_move_iterator( b.begin() ), std::make_move_iterator( b.end() ) );
    }
    else
    {
      a.emplace_back( std::move( *next_res ) );
    }
  }

  return aggregate;
}

void add( picojson::value* )
{
  return;
}

template <typename T1, typename... Types>
void add( picojson::value* v, std::string&& var1, T1&& var2, Types&&... var3 )
{
  if ( v->is<picojson::object>() )
  {
    auto& o = v->get<picojson::object>();
    if constexpr ( std::is_same_v<std::decay_t<T1>, int> )
    {
      o[var1] = picojson::value( static_cast<double>( var2 ) );
    }
    else if constexpr ( std::is_same_v<std::decay_t<T1>, antlrcpp::Any> )
    {
      if ( var2.has_value() )
      {
        auto* v2 = std::any_cast<picojson::value>( &var2 );
        o[var1] = std::move( *v2 );
      }
      else
        o[var1];
    }
    else if constexpr ( std::is_same_v<std::decay_t<T1>, picojson::value> )
    {
      o[var1] = std::move( var2 );
    }
    else
    {
      o[var1] = picojson::value( var2 );
    }
  }
  add( v, std::forward<Types>( var3 )... );
}

template <typename T1, typename... Types>
void add( antlrcpp::Any* any_v, std::string&& var1, T1&& var2, Types&&... var3 )
{
  auto* v = std::any_cast<picojson::value>( any_v );
  add( v, std::forward<std::string>( var1 ), std::forward<T1>( var2 ),
       std::forward<Types>( var3 )... );
}

template <typename Rangeable, typename... Types>
picojson::value new_node( Rangeable* ctx, const std::string&& type, Types&&... var3 )
{
  auto value = picojson::value( picojson::object_type, false );
  Range range( *ctx );
  auto& obj = value.get<picojson::object>();
  obj["type"] = picojson::value( type );
  obj["start"] = picojson::value( picojson::object_type, false );
  auto& start = obj["start"].get<picojson::object>();
  start["line_number"] = picojson::value( static_cast<double>( range.start.line_number ) );
  start["character_column"] =
      picojson::value( static_cast<double>( range.start.character_column ) );
  start["token_index"] = picojson::value( static_cast<double>( range.start.token_index ) );

  obj["end"] = picojson::value( picojson::object_type, false );
  auto& end = obj["end"].get<picojson::object>();
  end["line_number"] = picojson::value( static_cast<double>( range.end.line_number ) );
  end["character_column"] = picojson::value( static_cast<double>( range.end.character_column ) );
  end["token_index"] = picojson::value( static_cast<double>( range.end.token_index ) );

  add( &value, std::forward<Types>( var3 )... );
  return value;
};

void JsonAstFileProcessor::mergeComments()
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

void JsonAstFileProcessor::buildLine()
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

antlrcpp::Any JsonAstFileProcessor::visitCompilationUnit(
    EscriptGrammar::EscriptParser::CompilationUnitContext* ctx )
{
  auto body = new_node( ctx, "file",                   //
                        "body", visitChildren( ctx ),  //
                        "module", false                //
  );
  while ( !_comments.empty() )
  {
    _lines.push_back( std::string( _currident * 2, ' ' ) + _comments.front().text );
    _comments.erase( _comments.begin() );
  }
  return body;
}

antlrcpp::Any JsonAstFileProcessor::visitVarStatement(
    EscriptGrammar::EscriptParser::VarStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "var", r.start, TokenPart::SPACE );
  auto declarations = visitVariableDeclarationList( ctx->variableDeclarationList() );
  _line_parts.emplace_back( ";", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();
  return new_node( ctx, "var-statement",         //
                   "declarations", declarations  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitWhileStatement(
    EscriptGrammar::EscriptParser::WhileStatementContext* ctx )
{
  auto label = make_statement_label( ctx->statementLabel() );
  Range r( *ctx );
  _line_parts.emplace_back( "while", r.start, TokenPart::SPACE );
  _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  auto testc = ctx->parExpression()->expression();
  auto test = visitExpression( testc );
  _line_parts.emplace_back( ")", Range( *testc ).end, TokenPart::SPACE );
  buildLine();

  ++_currident;
  auto body = visitBlock( ctx->block() );
  --_currident;
  _line_parts.emplace_back( "endwhile", r.end, TokenPart::SPACE );
  buildLine();
  return new_node( ctx, "while-statement",       //
                   "label", std::move( label ),  //
                   "test", std::move( test ),    //
                   "body", std::move( body )     //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitVariableDeclarationList(
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
    auto test = visitVariableDeclaration( args[i] );
  }

  return {};
}
antlrcpp::Any JsonAstFileProcessor::visitVariableDeclaration(
    EscriptGrammar::EscriptParser::VariableDeclarationContext* ctx )
{
  auto i = make_identifier( ctx->IDENTIFIER() );
  // auto const_declaration_ctx = ctx->constantDeclaration();
  antlrcpp::Any init;

  if ( auto variable_declaration_initializer = ctx->variableDeclarationInitializer() )
  {
    if ( auto expression = variable_declaration_initializer->expression() )
    {
      Range r( *expression );
      _line_parts.emplace_back( ":=", r.start, TokenPart::SPACE );
      init = visitExpression( expression );
    }

    else if ( auto arr = variable_declaration_initializer->ARRAY() )
    {
      Range r( *arr );
      _line_parts.emplace_back( "array", r.end, TokenPart::SPACE );
      init = new_node( ctx, "array-expression",     //
                       "elements", defaultResult()  //
      );
    }
  }

  return new_node( ctx, "variable-declaration",  //
                   "name", i,                    //
                   "init", std::move( init )     //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitConstStatement(
    EscriptGrammar::EscriptParser::ConstStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "const", r.start, TokenPart::SPACE );
  auto const_declaration_ctx = ctx->constantDeclaration();
  antlrcpp::Any init;
  auto ident = make_identifier( const_declaration_ctx->IDENTIFIER() );

  if ( auto variable_declaration_initializer =
           const_declaration_ctx->variableDeclarationInitializer() )
  {
    if ( auto expression = variable_declaration_initializer->expression() )
    {
      Range re( *expression );
      _line_parts.emplace_back( ":=", re.start, TokenPart::SPACE );
      init = visitExpression( expression );
      _line_parts.emplace_back( ";", re.end,
                                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    }

    else if ( variable_declaration_initializer->ARRAY() )
    {
      Range ra( *expression );
      _line_parts.emplace_back( ":=", ra.start, TokenPart::SPACE );
      init = new_node( ctx, "array-expression",     //
                       "elements", defaultResult()  //
      );
      _line_parts.emplace_back( ";", ra.end,
                                TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    }
  }

  buildLine();
  return new_node( ctx, "const-statement",      //
                   "name", std::move( ident ),  //
                   "init", std::move( init )    //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitDictInitializerExpression(
    EscriptGrammar::EscriptParser::DictInitializerExpressionContext* ctx )
{
  antlrcpp::Any init;

  auto key = visitExpression( ctx->expression( 0 ) );

  if ( ctx->expression().size() > 1 )
  {
    Range r( *ctx->expression( 1 ) );
    _line_parts.emplace_back( "->", r.start, TokenPart::SPACE );
    init = visitExpression( ctx->expression( 1 ) );
  }
  Range r( *ctx );
  _line_parts.emplace_back( ",", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  return new_node( ctx, "dictionary-initializer",  //
                   "key", key,                     //
                   "value", std::move( init )      //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitDoStatement(
    EscriptGrammar::EscriptParser::DoStatementContext* ctx )
{
  auto label = make_statement_label( ctx->statementLabel() );
  Range r( *ctx );
  _line_parts.emplace_back( "do", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();

  ++_currident;
  auto body = visitBlock( ctx->block() );
  --_currident;

  auto exp = ctx->parExpression()->expression();
  _line_parts.emplace_back( "dowhile", Range( *exp ).start, TokenPart::SPACE );
  _line_parts.emplace_back( "(", Range( *exp ).start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  auto test = visitExpression( exp );
  _line_parts.emplace_back( ")", Range( *exp ).start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  return new_node( ctx, "do-statement",          //
                   "label", std::move( label ),  //
                   "test", std::move( test ),    //
                   "body", std::move( body )     //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitEnumListEntry(
    EscriptGrammar::EscriptParser::EnumListEntryContext* ctx )
{
  antlrcpp::Any value;
  auto identifier = make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    _line_parts.emplace_back( ":=", Range( *expression ).start, TokenPart::SPACE );
    value = visitExpression( expression );
  }
  return new_node( ctx, "enum-entry",                      //
                   "identifier", std::move( identifier ),  //
                   "value", std::move( value )             //
  );
}
antlrcpp::Any JsonAstFileProcessor::visitEnumList(
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
  return new_node( ctx, "enum-entry",          //
                   "value", picojson::value()  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitEnumStatement(
    EscriptGrammar::EscriptParser::EnumStatementContext* ctx )
{
  _line_parts.emplace_back( "enum", Range( *ctx ).start, TokenPart::SPACE );
  auto identifier = make_identifier( ctx->IDENTIFIER() );
  buildLine();
  ++_currident;
  auto enums = visitEnumList( ctx->enumList() );
  --_currident;

  _line_parts.emplace_back( "endenum", Range( *ctx ).end, TokenPart::SPACE );
  buildLine();
  return new_node( ctx, "enum-statement",                  //
                   "identifier", std::move( identifier ),  //
                   "enums", std::move( enums )             //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExitStatement(
    EscriptGrammar::EscriptParser::ExitStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "exit", r.start, TokenPart::NONE );
  _line_parts.emplace_back( ";", r.start, TokenPart::ATTACHED | TokenPart::SPACE );
  buildLine();
  return new_node( ctx, "exit-statement" );
}

antlrcpp::Any JsonAstFileProcessor::visitExplicitArrayInitializer(
    EscriptGrammar::EscriptParser::ExplicitArrayInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "array", r.start, TokenPart::NONE );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  auto childs = visitChildren( ctx );
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return new_node( ctx, "array-expression",  //
                   "elements", childs        //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExplicitDictInitializer(
    EscriptGrammar::EscriptParser::ExplicitDictInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "dictionary", r.start, TokenPart::NONE );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  auto childs = visitChildren( ctx );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return new_node( ctx, "dictionary-expression",  //
                   "entries", childs              //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExplicitErrorInitializer(
    EscriptGrammar::EscriptParser::ExplicitErrorInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "error", r.start, TokenPart::NONE );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  auto childs = visitChildren( ctx );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return new_node( ctx, "error-expression",  //
                   "members", childs         //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitBareArrayInitializer(
    EscriptGrammar::EscriptParser::BareArrayInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  auto childs = visitChildren( ctx );
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return new_node( ctx, "array-expression",  //
                   "elements", childs        //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExplicitStructInitializer(
    EscriptGrammar::EscriptParser::ExplicitStructInitializerContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "struct", r.start, TokenPart::NONE );
  _line_parts.emplace_back( "{", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  size_t curcount = _line_parts.size();
  auto childs = visitChildren( ctx );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  _line_parts.emplace_back( "}", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == curcount )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return new_node( ctx, "struct-expression",  //
                   "members", childs          //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExpressionList(
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
    auto test = visitExpression( args[i] );
  }

  return {};
}
antlrcpp::Any JsonAstFileProcessor::visitExpression(
    EscriptGrammar::EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
    return visitPrimary( prim );
  else if ( ctx->prefix )
  {
    Range r( ctx->prefix );
    _line_parts.emplace_back( ctx->prefix->getText(), r.start, TokenPart::NONE );
    auto e = visitExpression( ctx->expression( 0 ) );
    return new_node( ctx, "unary-expression",             //
                     "prefix", true,                      //
                     "operator", ctx->prefix->getText(),  //
                     "argument", e                        //
    );
  }
  else if ( ctx->postfix )
  {
    Range r( ctx->postfix );
    auto e = visitExpression( ctx->expression( 0 ) );
    _line_parts.emplace_back( ctx->postfix->getText(), r.start,
                              TokenPart::SPACE | TokenPart::ATTACHED );
    return new_node( ctx, "unary-expression",              //
                     "prefix", false,                      //
                     "operator", ctx->postfix->getText(),  //
                     "argument", e                         //
    );
  }

  else if ( ctx->bop && ctx->expression().size() == 2 )
  {
    auto left = visitExpression( ctx->expression( 0 ) );
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

    auto right = visitExpression( ctx->expression( 1 ) );
    return new_node( ctx, "binary-expression",         //
                     "left", left,                     //
                     "operator", ctx->bop->getText(),  //
                     "right", right                    //
    );
  }
  else if ( auto suffix = ctx->expressionSuffix() )
  {
    return expression_suffix( ctx->expression( 0 ), suffix );
  }
  else if ( ctx->QUESTION() )
  {
    auto cond = visitExpression( ctx->expression( 0 ) );
    Range rcond( *ctx->expression( 0 ) );
    _line_parts.emplace_back( "?", rcond.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
    auto cons = visitExpression( ctx->expression( 1 ) );
    Range rcons( *ctx->expression( 1 ) );
    _line_parts.emplace_back( ":", rcons.end, TokenPart::SPACE | TokenPart::BREAKPOINT );

    auto alt = visitExpression( ctx->expression( 2 ) );

    return new_node( ctx, "conditional-expression",  //
                     "conditional", cond,            //
                     "consequent", cons,             //
                     "alternate", alt                //
    );
  }

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::expression_suffix(
    EscriptGrammar::EscriptParser::ExpressionContext* expr_ctx,
    EscriptGrammar::EscriptParser::ExpressionSuffixContext* expr_suffix_ctx )
{
  if ( auto indexing = expr_suffix_ctx->indexingSuffix() )
  {
    Range r( *indexing );
    auto entity = visitExpression( expr_ctx );
    _line_parts.emplace_back( "[", r.start, TokenPart::ATTACHED );
    auto index = visitExpressionList( indexing->expressionList() );
    _line_parts.emplace_back( "]", r.end, TokenPart::SPACE | TokenPart::ATTACHED );
    auto n = new_node( expr_ctx, "element-access-expression",  //
                       "indexes", index,                       //
                       "entity", entity                        //
    );
    return n;
  }
  else if ( auto member = expr_suffix_ctx->navigationSuffix() )
  {
    antlrcpp::Any accessor;
    Range r( *member );
    auto expr = visitExpression( expr_ctx );
    _line_parts.emplace_back( ".", r.start, TokenPart::ATTACHED );
    if ( auto string_literal = member->STRING_LITERAL() )
    {
      accessor = make_string_literal( string_literal );
    }
    else if ( auto identifier = member->IDENTIFIER() )
    {
      accessor = make_identifier( identifier );
    }

    return new_node( expr_ctx, "member-access-expression",  //
                     "name", std::move( accessor ),         //
                     "entity", std::move( expr )            //
    );
  }
  else if ( auto method = expr_suffix_ctx->methodCallSuffix() )
  {
    Range r( *method );
    auto entity = visitExpression( expr_ctx );
    _line_parts.emplace_back( ".", r.start, TokenPart::ATTACHED );
    auto ident = make_identifier( method->IDENTIFIER() );
    _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::ATTACHED );
    auto cursize = _line_parts.size();
    auto args = visitChildren( method );

    _line_parts.emplace_back( ")", r.end, TokenPart::SPACE );
    if ( _line_parts.size() - 1 == cursize )
      _line_parts.back().style |= TokenPart::ATTACHED;
    return new_node( expr_ctx, "method-call-expression",  //
                     "name", ident,                       //
                     "arguments", args,                   //
                     "entity", entity                     //
    );
  }

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitFloatLiteral(
    EscriptGrammar::EscriptParser::FloatLiteralContext* ctx )
{
  if ( auto float_literal = ctx->FLOAT_LITERAL() )
    return make_float_literal( float_literal );
  else if ( auto hex_float_literal = ctx->HEX_FLOAT_LITERAL() )
    return make_float_literal( hex_float_literal );

  return visitChildren( ctx );
}

antlrcpp::Any JsonAstFileProcessor::visitForeachIterableExpression(
    EscriptGrammar::EscriptParser::ForeachIterableExpressionContext* ctx )
{
  if ( auto parExpression = ctx->parExpression() )
  {
    return visitExpression( parExpression->expression() );
  }
  else if ( auto functionCall = ctx->functionCall() )
  {
    return visitFunctionCall( functionCall );
  }
  else if ( auto scopedFunctionCall = ctx->scopedFunctionCall() )
  {
    return visitScopedFunctionCall( scopedFunctionCall );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    return make_identifier( identifier );
  }
  else if ( auto explicitArrayInitializer = ctx->explicitArrayInitializer() )
  {
    return visitExplicitArrayInitializer( explicitArrayInitializer );
  }
  else if ( auto bareArrayInitializer = ctx->bareArrayInitializer() )
  {
    return visitBareArrayInitializer( bareArrayInitializer );
  }

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitForeachStatement(
    EscriptGrammar::EscriptParser::ForeachStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "foreach", r.start, TokenPart::SPACE );
  auto identifier = make_identifier( ctx->IDENTIFIER() );
  Range ri( *ctx->IDENTIFIER() );
  _line_parts.emplace_back( "in", ri.end, TokenPart::SPACE );
  _line_parts.emplace_back( "(", ri.end, TokenPart::SPACE );
  auto label = make_statement_label( ctx->statementLabel() );
  auto expression = visitForeachIterableExpression( ctx->foreachIterableExpression() );
  _line_parts.emplace_back( ")", ri.end, TokenPart::SPACE );
  buildLine();
  ++_currident;

  auto body = visitBlock( ctx->block() );
  --_currident;
  _line_parts.emplace_back( "endforeach", r.end, TokenPart::SPACE );
  buildLine();

  return new_node( ctx, "foreach-statement",               //
                   "identifier", std::move( identifier ),  //
                   "expression", std::move( expression ),  //
                   "label", std::move( label ),            //
                   "body", std::move( body )               //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitForStatement(
    EscriptGrammar::EscriptParser::ForStatementContext* ctx )
{
  auto label = make_statement_label( ctx->statementLabel() );

  auto forGroup = ctx->forGroup();

  if ( auto basicForStatement = forGroup->basicForStatement() )
  {
    auto node = visitBasicForStatement( basicForStatement );
    add( &node,                       //
         "label", std::move( label )  //
    );
    return node;
  }
  else if ( auto cstyleForStatement = forGroup->cstyleForStatement() )
  {
    auto node = visitCstyleForStatement( cstyleForStatement );
    add( &node,                       //
         "label", std::move( label )  //
    );
    return node;
  }

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitFunctionCall(
    EscriptGrammar::EscriptParser::FunctionCallContext* ctx )
{
  auto i = make_identifier( ctx->IDENTIFIER() );
  Range r( *ctx );

  _line_parts.emplace_back( "(", r.start,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  auto cursize = _line_parts.size();
  std::any any_args;
  if ( auto args = ctx->expressionList() )
    any_args = visitExpressionList( args );
  auto n = new_node( ctx, "function-call-expression",  //
                     "callee", i,                      //
                     "arguments", any_args,            //
                     "scope", picojson::value()        //
  );
  _line_parts.emplace_back( ")", r.end, TokenPart::SPACE );
  if ( _line_parts.size() - 1 == cursize )
    _line_parts.back().style |= TokenPart::ATTACHED;
  return n;
}
antlrcpp::Any JsonAstFileProcessor::visitFunctionDeclaration(
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
  auto name = make_identifier( ctx->IDENTIFIER() );
  Range rname( *ctx->IDENTIFIER() );
  _line_parts.emplace_back( "(", rname.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  auto cursize = _line_parts.size();
  auto parameters = visitFunctionParameters( ctx->functionParameters() );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  Range rparam( *ctx->functionParameters() );
  _line_parts.emplace_back( ")", rparam.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  if ( _line_parts.size() - 1 == cursize )

    _line_parts.back().style |= TokenPart::ATTACHED;
  ;
  buildLine();
  ++_currident;
  auto body = visitBlock( ctx->block() );
  --_currident;
  _line_parts.emplace_back( "endfunction", r.end, TokenPart::SPACE );
  buildLine();

  return new_node( ctx, "function-declaration",            //
                   "name", std::move( name ),              //
                   "parameters", std::move( parameters ),  //
                   "exported", (bool)exported,             //
                   "body", std::move( body )               //
  );
}
antlrcpp::Any JsonAstFileProcessor::visitFunctionParameter(
    EscriptGrammar::EscriptParser::FunctionParameterContext* ctx )
{
  antlrcpp::Any init;
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
  auto name = make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    Range re( *expression );
    _line_parts.emplace_back( ":=", re.start, TokenPart::SPACE );
    init = visitExpression( expression );
  }
  Range re( *ctx );
  _line_parts.emplace_back( ",", re.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );

  return new_node( ctx, "function-parameter",  //
                   "name", std::move( name ),  //
                   "init", std::move( init ),  //
                   "byref", (bool)byref,       //
                   "unused", (bool)unused      //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitBreakStatement(
    EscriptGrammar::EscriptParser::BreakStatementContext* ctx )
{
  antlrcpp::Any label;

  if ( auto identifier = ctx->IDENTIFIER() )
  {
    label = make_identifier( identifier );
  }

  _line_parts.emplace_back( "break", Range( *ctx ).end, TokenPart::NONE );
  _line_parts.emplace_back( ";", Range( *ctx ).end, TokenPart::SPACE );
  buildLine();
  return new_node( ctx, "break-statement",      //
                   "label", std::move( label )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitSwitchBlockStatementGroup(
    EscriptGrammar::EscriptParser::SwitchBlockStatementGroupContext* ctx )
{
  auto labels = defaultResult();
  for ( const auto& switchLabel : ctx->switchLabel() )
  {
    visitSwitchLabel( switchLabel );
  }

  auto body = visitBlock( ctx->block() );

  return new_node( ctx, "switch-block",            //
                   "labels", std::move( labels ),  //
                   "body", std::move( body )       //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitSwitchLabel(
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

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitCaseStatement(
    EscriptGrammar::EscriptParser::CaseStatementContext* ctx )
{
  Range r( *ctx );
  auto label = make_statement_label( ctx->statementLabel() );
  _line_parts.emplace_back( "case", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  auto test = visitExpression( ctx->expression() );
  _line_parts.emplace_back( ")", Range( *ctx->expression() ).end,
                            TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  ++_currident;
  auto cases = defaultResult();
  for ( const auto& switchBlockStatementGroup : ctx->switchBlockStatementGroup() )
  {
    visitSwitchBlockStatementGroup( switchBlockStatementGroup );
  }
  --_currident;
  _line_parts.emplace_back( "endcase", r.end, TokenPart::SPACE | TokenPart::BREAKPOINT );
  buildLine();
  return new_node( ctx, "case-statement",        //
                   "label", std::move( label ),  //
                   "test", std::move( test ),    //
                   "cases", std::move( cases )   //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitContinueStatement(
    EscriptGrammar::EscriptParser::ContinueStatementContext* ctx )
{
  antlrcpp::Any label;

  if ( auto identifier = ctx->IDENTIFIER() )
  {
    label = make_identifier( identifier );
  }

  _line_parts.emplace_back( "continue", Range( *ctx ).end, TokenPart::NONE );
  _line_parts.emplace_back( ";", Range( *ctx ).end, TokenPart::SPACE );
  buildLine();
  return new_node( ctx, "continue-statement",   //
                   "label", std::move( label )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitBasicForStatement(
    EscriptGrammar::EscriptParser::BasicForStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "for", r.start, TokenPart::SPACE );
  auto identifier = make_identifier( ctx->IDENTIFIER() );
  _line_parts.emplace_back( ":=", Range( *ctx->IDENTIFIER() ).end, TokenPart::SPACE );
  auto first = visitExpression( ctx->expression( 0 ) );
  _line_parts.emplace_back( "to", Range( *ctx->expression( 0 ) ).end, TokenPart::SPACE );
  auto last = visitExpression( ctx->expression( 1 ) );
  buildLine();

  ++_currident;
  auto body = visitBlock( ctx->block() );
  --_currident;

  _line_parts.emplace_back( "endfor", r.end, TokenPart::SPACE );
  buildLine();

  return new_node( ctx, "basic-for-statement",             //
                   "identifier", std::move( identifier ),  //
                   "first", std::move( first ),            //
                   "last", std::move( last )               //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitCstyleForStatement(
    EscriptGrammar::EscriptParser::CstyleForStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "for", r.start, TokenPart::SPACE );
  _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );
  auto initializer = visitExpression( ctx->expression( 0 ) );
  _line_parts.emplace_back( ";", r.start,
                            TokenPart::ATTACHED | TokenPart::SPACE | TokenPart::BREAKPOINT );
  auto test = visitExpression( ctx->expression( 1 ) );
  _line_parts.emplace_back( ";", r.start,
                            TokenPart::ATTACHED | TokenPart::SPACE | TokenPart::BREAKPOINT );
  auto advancer = visitExpression( ctx->expression( 2 ) );
  _line_parts.emplace_back( ")", r.start, TokenPart::SPACE | TokenPart::BREAKPOINT );

  buildLine();
  ++_currident;
  auto body = visitBlock( ctx->block() );
  --_currident;
  _line_parts.emplace_back( "endfor", r.end, TokenPart::SPACE );
  buildLine();
  return new_node( ctx, "cstyle-for-statement",              //
                   "initializer", std::move( initializer ),  //
                   "test", std::move( test ),                //
                   "advancer", std::move( advancer ),        //
                   "body", std::move( body )                 //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitRepeatStatement(
    EscriptGrammar::EscriptParser::RepeatStatementContext* ctx )
{
  auto label = make_statement_label( ctx->statementLabel() );
  Range r( *ctx );
  _line_parts.emplace_back( "repeat", r.start, TokenPart::SPACE );
  buildLine();

  ++_currident;
  auto body = visitBlock( ctx->block() );
  --_currident;

  _line_parts.emplace_back( "until", Range( *ctx->expression() ).start, TokenPart::SPACE );
  auto test = visitExpression( ctx->expression() );
  _line_parts.emplace_back( ";", Range( *ctx->expression() ).start,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();

  return new_node( ctx, "repeat-statement",      //
                   "label", std::move( label ),  //
                   "body", std::move( body ),    //
                   "test", std::move( test )     //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitReturnStatement(
    EscriptGrammar::EscriptParser::ReturnStatementContext* ctx )
{
  antlrcpp::Any value;
  Range r( *ctx );
  _line_parts.emplace_back( "return", r.start, TokenPart::SPACE );

  if ( auto expression = ctx->expression() )
  {
    value = visitExpression( expression );
  }
  _line_parts.emplace_back( ";", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();

  return new_node( ctx, "return-statement",     //
                   "value", std::move( value )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitScopedFunctionCall(
    EscriptGrammar::EscriptParser::ScopedFunctionCallContext* ctx )
{
  return new_node( ctx, "function-call-expression",                                 //
                   "callee", make_identifier( ctx->functionCall()->IDENTIFIER() ),  //
                   "arguments", visitChildren( ctx->functionCall() ),               //
                   "scope", make_identifier( ctx->IDENTIFIER() )                    //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitFunctionReference(
    EscriptGrammar::EscriptParser::FunctionReferenceContext* ctx )
{
  _line_parts.emplace_back( "@", Range( *ctx ).start, TokenPart::NONE );
  return new_node( ctx, "function-reference-expression",         //
                   "name", make_identifier( ctx->IDENTIFIER() )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitGotoStatement(
    EscriptGrammar::EscriptParser::GotoStatementContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "goto", r.start, TokenPart::SPACE );
  auto label = make_identifier( ctx->IDENTIFIER() );
  _line_parts.emplace_back( ";", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  buildLine();
  return new_node( ctx, "goto-statement",  //
                   "label", label          //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitIfStatement(
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

antlrcpp::Any JsonAstFileProcessor::visitIncludeDeclaration(
    EscriptGrammar::EscriptParser::IncludeDeclarationContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "include", r.start, TokenPart::SPACE );
  auto id = visitStringIdentifier( ctx->stringIdentifier() );
  _line_parts.emplace_back( ";", r.end, TokenPart::SPACE | TokenPart::ATTACHED );
  buildLine();
  return new_node( ctx, "include-declaration",  //
                   "specifier", id              //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitInterpolatedString(
    EscriptGrammar::EscriptParser::InterpolatedStringContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "$", r.start, TokenPart::SPACE );
  _line_parts.emplace_back( "\"", r.start, TokenPart::ATTACHED );
  INFO_PRINTLN( "RAW '{}'", ctx->getText() );
  auto childs = visitChildren( ctx );
  _line_parts.emplace_back( "\"", r.end, TokenPart::ATTACHED | TokenPart::SPACE );
  return new_node( ctx, "interpolated-string-expression",  //
                   "parts", childs                         //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitInterpolatedStringPart(
    EscriptGrammar::EscriptParser::InterpolatedStringPartContext* ctx )
{
  antlrcpp::Any format;
  antlrcpp::Any expression;
  if ( auto expression_ctx = ctx->expression() )
  {
    Range r( *expression_ctx );
    _line_parts.emplace_back( "{", r.start, TokenPart::ATTACHED );
    expression = visitExpression( expression_ctx );

    if ( auto format_string = ctx->FORMAT_STRING() )
    {
      INFO_PRINTLN( "FORMAT {}", format_string->getText() );
      _line_parts.emplace_back( ":", Range( *format_string ).start, TokenPart::ATTACHED );
      _line_parts.emplace_back( format_string->getText(), Range( *format_string ).start,
                                TokenPart::ATTACHED );
      //      format = new_node( format_string, "format-string",  //
      //                       "id", format_string->getText()   //
      //  );
    }
    _line_parts.emplace_back( "}", r.end, TokenPart::ATTACHED );
  }

  else if ( auto string_literal = ctx->STRING_LITERAL_INSIDE() )
  {
    _line_parts.emplace_back( string_literal->getText(), Range( *string_literal ).start,
                              TokenPart::ATTACHED );
    //    expression = make_string_literal( string_literal, string_literal->getText() );
  }
  else if ( auto lbrace = ctx->DOUBLE_LBRACE_INSIDE() )
  {
    _line_parts.emplace_back( "{{", Range( *lbrace ).start, TokenPart::ATTACHED );
    // expression = make_string_literal( lbrace, "{" );
  }
  else if ( auto rbrace = ctx->DOUBLE_RBRACE() )
  {
    _line_parts.emplace_back( "}}", Range( *rbrace ).start, TokenPart::ATTACHED );
    //    expression = make_string_literal( rbrace, "}" );
  }
  else if ( auto escaped = ctx->REGULAR_CHAR_INSIDE() )
  {
    _line_parts.emplace_back( escaped->getText(), Range( *escaped ).start, TokenPart::ATTACHED );
    //    expression = make_string_literal( escaped, escaped->getText() );
  }

  return new_node( ctx, "interpolated-string-part",        //
                   "expression", std::move( expression ),  //
                   "format", std::move( format )           //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitBoolLiteral(
    EscriptGrammar::EscriptParser::BoolLiteralContext* ctx )
{
  if ( auto bool_false = ctx->BOOL_FALSE() )
  {
    return make_bool_literal( bool_false );
  }
  else if ( auto bool_true = ctx->BOOL_TRUE() )
  {
    return make_bool_literal( bool_true );
  }

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitIntegerLiteral(
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

antlrcpp::Any JsonAstFileProcessor::visitLiteral(
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

antlrcpp::Any JsonAstFileProcessor::visitModuleFunctionDeclaration(
    EscriptGrammar::EscriptParser::ModuleFunctionDeclarationContext* ctx )
{
  auto name = make_identifier( ctx->IDENTIFIER() );
  antlrcpp::Any parameters;

  if ( auto moduleFunctionParameterList = ctx->moduleFunctionParameterList() )
  {
    parameters = visitModuleFunctionParameterList( moduleFunctionParameterList );
  }
  else
  {
    parameters = defaultResult();
  }

  return new_node( ctx, "module-function-declaration",    //
                   "name", std::move( name ),             //
                   "parameters", std::move( parameters )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitModuleFunctionParameter(
    EscriptGrammar::EscriptParser::ModuleFunctionParameterContext* ctx )
{
  antlrcpp::Any value;
  auto name = make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    value = visitExpression( expression );
  }

  return new_node( ctx, "module-function-parameter",  //
                   "name", std::move( name ),         //
                   "value", std::move( value )        //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitModuleUnit(
    EscriptGrammar::EscriptParser::ModuleUnitContext* ctx )
{
  return new_node( ctx, "file",                   //
                   "body", visitChildren( ctx ),  //
                   "module", true                 //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitPrimary(
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
    auto e = visitExpression( parExpression->expression() );

    _line_parts.emplace_back( ")", r.end, TokenPart::SPACE );
    return e;
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
    return new_node( uninit, "uninitialized-value" );
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

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitProgramDeclaration(
    EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "program", r.start, TokenPart::SPACE );
  auto ident = make_identifier( ctx->IDENTIFIER() );
  _line_parts.emplace_back( "(", r.start, TokenPart::SPACE | TokenPart::ATTACHED );
  auto cursize = _line_parts.size();
  auto args = visitProgramParameters( ctx->programParameters() );
  if ( _line_parts.back().text == "," )  // arguments always add , remove the last one
    _line_parts.pop_back();
  _line_parts.emplace_back( ")", r.start, TokenPart::SPACE );
  if ( _line_parts.size() - 1 == cursize )
    _line_parts.back().style |= TokenPart::ATTACHED;
  buildLine();
  ++_currident;
  auto block = visitBlock( ctx->block() );
  --_currident;
  _line_parts.emplace_back( "endprogram", r.end, TokenPart::SPACE );
  buildLine();

  return new_node( ctx, "program",      //
                   "name", ident,       //
                   "parameters", args,  //
                   "body", block        //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitProgramParameter(
    EscriptGrammar::EscriptParser::ProgramParameterContext* ctx )
{
  antlrcpp::Any init;
  auto unused = ctx->UNUSED();
  if ( unused )
  {
    Range r( *unused );
    _line_parts.emplace_back( "unused", r.start, TokenPart::SPACE );
  }

  auto id = make_identifier( ctx->IDENTIFIER() );
  if ( auto expression = ctx->expression() )
  {
    Range r( *expression );
    _line_parts.emplace_back( ":=", r.start, TokenPart::SPACE );
    init = visitExpression( expression );
  }
  Range r( *ctx );
  _line_parts.emplace_back( ",", r.end, TokenPart::SPACE | TokenPart::ATTACHED );
  return new_node( ctx, "program-parameter",  //
                   "name", id,                //
                   "unused", (bool)unused,    //
                   "init", std::move( init )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitStatement(
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
    auto a = new_node( ctx, "expression-statement",                 //
                       "expression", visitExpression( expression )  //
    );

    Range r( *ctx );
    _line_parts.emplace_back( ";", r.end,
                              TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
    buildLine();
    return a;
  }

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitStringIdentifier(
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
  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitStructInitializerExpression(
    EscriptGrammar::EscriptParser::StructInitializerExpressionContext* ctx )
{
  antlrcpp::Any name;
  antlrcpp::Any init;

  if ( auto identifier = ctx->IDENTIFIER() )
  {
    name = make_identifier( identifier );
  }
  else if ( auto string_literal = ctx->STRING_LITERAL() )
  {
    name = make_string_literal( string_literal );
  }

  if ( auto expression = ctx->expression() )
  {
    Range r( *expression );
    _line_parts.emplace_back( ":=", r.start, TokenPart::SPACE );
    init = visitExpression( expression );
  }
  Range r( *ctx );
  _line_parts.emplace_back( ",", r.end,
                            TokenPart::SPACE | TokenPart::ATTACHED | TokenPart::BREAKPOINT );
  return new_node( ctx, "struct-initializer",  //
                   "name", std::move( name ),  //
                   "init", std::move( init )   //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitUseDeclaration( EscriptParser::UseDeclarationContext* ctx )
{
  Range r( *ctx );
  _line_parts.emplace_back( "use", r.start, TokenPart::SPACE );
  auto a = visitStringIdentifier( ctx->stringIdentifier() );
  _line_parts.emplace_back( ";", r.end, TokenPart::SPACE | TokenPart::ATTACHED );

  buildLine();
  return new_node( ctx, "use-declaration",  //
                   "specifier", a           //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_statement_label(
    EscriptGrammar::EscriptParser::StatementLabelContext* ctx )
{
  antlrcpp::Any label;

  if ( ctx )
  {
    label = make_identifier( ctx->IDENTIFIER() );
    Range r( *ctx->IDENTIFIER() );
    _line_parts.emplace_back( ":", r.end, TokenPart::SPACE | TokenPart::ATTACHED );
  }


  return label;
}

antlrcpp::Any JsonAstFileProcessor::make_identifier( antlr4::tree::TerminalNode* terminal )
{
  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return new_node( terminal, "identifier",    //
                   "id", terminal->getText()  //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_string_literal( antlr4::tree::TerminalNode* terminal,
                                                         const std::string& text )
{
  Range r( *terminal );
  _line_parts.emplace_back( fmt::format( "\"{}\"", text ), r.start, TokenPart::SPACE );
  return new_node( terminal, "string-value",  //
                   "value", text,             //
                   "raw", text                //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_string_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();
  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );

  return new_node( terminal, "string-value",                      //
                   "value", text.substr( 1, text.length() - 2 ),  //
                   "raw", text                                    //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_integer_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();

  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return new_node( terminal, "integer-literal",  //
                   "value", std::stoi( text ),   //
                   "raw", text                   //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_float_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();

  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return new_node( terminal, "float-literal",   //
                   "value", std::stod( text ),  //
                   "raw", text                  //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_bool_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();
  bool value = terminal->getSymbol()->getType() == EscriptGrammar::EscriptLexer::BOOL_TRUE;
  Range r( *terminal );
  _line_parts.emplace_back( terminal->getText(), r.start, TokenPart::SPACE );
  return new_node( terminal, "boolean-literal",  //
                   "value", value,               //
                   "raw", text                   //
  );
}

}  // namespace Pol::Bscript::Compiler
