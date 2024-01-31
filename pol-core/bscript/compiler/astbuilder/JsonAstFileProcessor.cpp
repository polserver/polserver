#include "JsonAstFileProcessor.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
// #include "bscript/compiler/ast/ConstDeclaration.h"
// #include "bscript/compiler/ast/Program.h"
#include "bscript/compiler/ast/Statement.h"
// #include "bscript/compiler/ast/TopLevelStatements.h"
// #include "bscript/compiler/astbuilder/AvailableUserFunction.h"
// #include "bscript/compiler/astbuilder/BuilderWorkspace.h"
// #include "bscript/compiler/astbuilder/ModuleProcessor.h"
#include "bscript/compiler/file/SourceFile.h"
// #include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/JsonAst.h"
// #include "clib/fileutil.h"
// #include "clib/logfacility.h"
// #include "clib/timer.h"
// #include "compilercfg.h"
// #include "plib/pkg.h"

#include <picojson/picojson.h>

#include <iostream>

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
JsonAstFileProcessor::JsonAstFileProcessor( const SourceFileIdentifier& source_file_identifier,
                                            Profile& profile, Report& report )
    : source_file_identifier( source_file_identifier ), profile( profile ), report( report )
{
}


picojson::value JsonAstFileProcessor::process_compilation_unit( SourceFile& sf )
{
  if ( auto compilation_unit = sf.get_compilation_unit( report, source_file_identifier ) )
  {
    return std::any_cast<picojson::value>( compilation_unit->accept( this ) );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}


picojson::value JsonAstFileProcessor::process_module_unit( SourceFile& sf )
{
  if ( auto module_unit = sf.get_module_unit( report, source_file_identifier ) )
  {
    return std::any_cast<picojson::value>( module_unit->accept( this ) );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}


antlrcpp::Any JsonAstFileProcessor::defaultResult()
{
  return picojson::value( picojson::array() );
}

antlrcpp::Any JsonAstFileProcessor::aggregateResult( antlrcpp::Any /*picojson::array*/ aggregate,
                                                     antlrcpp::Any /*picojson::array*/ nextResult )
{
  // std::any_cast<picojson::array>( aggregate )
  auto accum = std::any_cast<picojson::value>( aggregate );
  auto next_res = std::any_cast<picojson::value>( nextResult );
  // .push_back( std::any_cast<picojson::array>( nextResult ) );

  if ( accum.is<picojson::array>() )
  {
    if ( next_res.is<picojson::array>() )
    {
      for ( auto const& v : next_res.get<picojson::array>() )
        accum.get<picojson::array>().push_back( v );
    }
    else
    {
      accum.get<picojson::array>().push_back( next_res );
    }
  }

  return accum;
}

picojson::value add( picojson::value v )
{
  return v;
}

template <typename T>
picojson::value to_value( T arg )
{
  return picojson::value( arg );
}

template <>
picojson::value to_value( int arg )
{
  return picojson::value( static_cast<double>( arg ) );
}

template <>
picojson::value to_value( antlrcpp::Any arg )
{
  if ( arg.has_value() )
    return picojson::value( std::any_cast<picojson::value>( arg ) );
  return picojson::value();
}

template <typename T1, typename... Types>
picojson::value add( picojson::value v, const std::string& var1, T1 var2, Types... var3 )
{
  if ( v.is<picojson::object>() )
  {
    v.get<picojson::object>().insert(
        std::pair<std::string, picojson::value>( { var1, to_value( var2 ) } ) );
  }
  return add( v, var3... );
}


template <typename Rangeable, typename... Types>
picojson::value new_node( Rangeable* ctx, const std::string& type, Types... var3 )
{
  picojson::object w;
  Range range( *ctx );

  w["type"] = picojson::value( type );
  w["start"] = picojson::value( picojson::object( {
      { "line_number", picojson::value( static_cast<double>( range.start.line_number ) ) },
      { "character_column",
        picojson::value( static_cast<double>( range.start.character_column ) ) },
      { "token_index", picojson::value( static_cast<double>( range.start.token_index ) ) },
  } ) );
  w["end"] = picojson::value( picojson::object( {
      { "line_number", picojson::value( static_cast<double>( range.end.line_number ) ) },
      { "character_column", picojson::value( static_cast<double>( range.end.character_column ) ) },
      { "token_index", picojson::value( static_cast<double>( range.end.token_index ) ) },
  } ) );

  return add( picojson::value( w ), var3... );
};


antlrcpp::Any JsonAstFileProcessor::visitCompilationUnit(
    EscriptGrammar::EscriptParser::CompilationUnitContext* ctx )
{
  return new_node( ctx, "file",                   //
                   "body", visitChildren( ctx ),  //
                   "module", false                //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitDictInitializerExpression(
    EscriptGrammar::EscriptParser::DictInitializerExpressionContext* ctx )
{
  antlrcpp::Any init;

  if ( ctx->expression().size() > 1 )
  {
    init = visitExpression( ctx->expression( 1 ) );
  }

  return new_node( ctx, "dictionary-initializer",                   //
                   "key", visitExpression( ctx->expression( 0 ) ),  //
                   "value", init                                    //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExplicitArrayInitializer(
    EscriptGrammar::EscriptParser::ExplicitArrayInitializerContext* ctx )
{
  return new_node( ctx, "array-expression",          //
                   "elements", visitChildren( ctx )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExplicitDictInitializer(
    EscriptGrammar::EscriptParser::ExplicitDictInitializerContext* ctx )
{
  return new_node( ctx, "dictionary-expression",    //
                   "entries", visitChildren( ctx )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExplicitErrorInitializer(
    EscriptGrammar::EscriptParser::ExplicitErrorInitializerContext* ctx )
{
  return new_node( ctx, "error-expression",         //
                   "members", visitChildren( ctx )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitBareArrayInitializer(
    EscriptGrammar::EscriptParser::BareArrayInitializerContext* ctx )
{
  return new_node( ctx, "array-expression",          //
                   "elements", visitChildren( ctx )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExplicitStructInitializer(
    EscriptGrammar::EscriptParser::ExplicitStructInitializerContext* ctx )
{
  return new_node( ctx, "struct-expression",        //
                   "members", visitChildren( ctx )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExpression(
    EscriptGrammar::EscriptParser::ExpressionContext* ctx )
{
  if ( auto prim = ctx->primary() )
    return visitPrimary( prim );
  else if ( ctx->prefix )
    return new_node( ctx, "unary-expression",                             //
                     "prefix", true,                                      //
                     "operator", ctx->prefix->getText(),                  //
                     "argument", visitExpression( ctx->expression( 0 ) )  //
    );

  else if ( ctx->postfix )
    return new_node( ctx, "unary-expression",                             //
                     "prefix", false,                                     //
                     "operator", ctx->postfix->getText(),                 //
                     "argument", visitExpression( ctx->expression( 0 ) )  //
    );

  else if ( ctx->bop && ctx->expression().size() == 2 )
  {
    return new_node( ctx, "binary-expression",                         //
                     "left", visitExpression( ctx->expression( 0 ) ),  //
                     "operator", ctx->bop->getText(),                  //
                     "right", visitExpression( ctx->expression( 1 ) )  //
    );
  }
  else if ( auto suffix = ctx->expressionSuffix() )
  {
    return expression_suffix( ctx->expression( 0 ), suffix );
    // auto suffix_node = visitExpressionSuffix( suffix );
    // add( suffix_node, "target", visitExpression( ctx->expression( 0 ) ) );
    // result = expression_suffix( expression( ctx->expression()[0] ), suffix );
  }
  else if ( ctx->QUESTION() )
  {
    return new_node( ctx, "conditional-expression",                           //
                     "conditional", visitExpression( ctx->expression( 0 ) ),  //
                     "consequent", visitExpression( ctx->expression( 1 ) ),   //
                     "alternate", visitExpression( ctx->expression( 2 ) )     //
    );
  }
  return visitChildren( ctx );
  // return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::expression_suffix(
    EscriptGrammar::EscriptParser::ExpressionContext* expr_ctx,
    EscriptGrammar::EscriptParser::ExpressionSuffixContext* expr_suffix_ctx )
{
  if ( auto indexing = expr_suffix_ctx->indexingSuffix() )
  {
    return new_node( expr_ctx, "element-access-expression",                         //
                     "indexes", visitExpressionList( indexing->expressionList() ),  //
                     "entity", visitExpression( expr_ctx )                          //
    );
  }
  else if ( auto member = expr_suffix_ctx->navigationSuffix() )
  {
    antlrcpp::Any accessor;

    if ( auto string_literal = member->STRING_LITERAL() )
    {
      accessor = make_string_literal( string_literal );
    }
    else if ( auto identifier = member->IDENTIFIER() )
    {
      accessor = make_identifier( identifier );
    }

    return new_node( expr_ctx, "member-access-expression",  //
                     "name", accessor,                      //
                     "entity", visitExpression( expr_ctx )  //
    );
  }
  else if ( auto method = expr_suffix_ctx->methodCallSuffix() )
  {
    return new_node( expr_ctx, "method-call-expression",               //
                     "name", make_identifier( method->IDENTIFIER() ),  //
                     "arguments", visitChildren( method ),             //
                     "entity", visitExpression( expr_ctx )             //
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

antlrcpp::Any JsonAstFileProcessor::visitFunctionCall(
    EscriptGrammar::EscriptParser::FunctionCallContext* ctx )
{
  return new_node( ctx, "function-call-expression",                 //
                   "callee", make_identifier( ctx->IDENTIFIER() ),  //
                   "arguments", visitChildren( ctx ),               //
                   "scope", picojson::value()                       //
  );
}
antlrcpp::Any JsonAstFileProcessor::visitFunctionDeclaration(
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  auto exported = ctx->EXPORTED() ? true : false;
  auto parameters = visitFunctionParameters( ctx->functionParameters() );
  auto body = visitBlock( ctx->block() );

  return new_node( ctx, "function-declaration",                   //
                   "name", make_identifier( ctx->IDENTIFIER() ),  //
                   "parameters", parameters,                      //
                   "exported", exported,                          //
                   "body", body                                   //
  );
}
antlrcpp::Any JsonAstFileProcessor::visitFunctionParameter(
    EscriptGrammar::EscriptParser::FunctionParameterContext* ctx )
{
  antlrcpp::Any init;
  auto byref = ctx->BYREF() ? true : false;
  auto unused = ctx->UNUSED() ? true : false;

  if ( auto expression = ctx->expression() )
  {
    init = visitExpression( expression );
  }

  return new_node( ctx, "function-parameter",                     //
                   "name", make_identifier( ctx->IDENTIFIER() ),  //
                   "init", init,                                  //
                   "byref", byref,                                //
                   "unused", unused                               //
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
  return new_node( ctx, "function-reference-expression",         //
                   "name", make_identifier( ctx->IDENTIFIER() )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitIncludeDeclaration(
    EscriptGrammar::EscriptParser::IncludeDeclarationContext* ctx )
{
  return new_node( ctx, "include-declaration",                                    //
                   "specifier", visitStringIdentifier( ctx->stringIdentifier() )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitInterpolatedString(
    EscriptGrammar::EscriptParser::InterpolatedStringContext* ctx )
{
  return new_node( ctx, "interpolated-string-expression",  //
                   "parts", visitChildren( ctx )           //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitInterpolatedStringPart(
    EscriptGrammar::EscriptParser::InterpolatedStringPartContext* ctx )
{
  antlrcpp::Any format;
  antlrcpp::Any expression;
  if ( auto expression_ctx = ctx->expression() )
  {
    expression = visitExpression( expression_ctx );

    if ( auto format_string = ctx->FORMAT_STRING() )
    {
      format = new_node( format_string, "format-string",  //
                         "id", format_string->getText()   //
      );
    }
  }

  else if ( auto string_literal = ctx->STRING_LITERAL_INSIDE() )
  {
    expression = make_string_literal( string_literal, string_literal->getText() );
  }
  else if ( auto lbrace = ctx->DOUBLE_LBRACE_INSIDE() )
  {
    expression = make_string_literal( lbrace, "{" );
  }
  else if ( auto rbrace = ctx->DOUBLE_RBRACE() )
  {
    expression = make_string_literal( rbrace, "}" );
  }
  else if ( auto escaped = ctx->REGULAR_CHAR_INSIDE() )
  {
    expression = make_string_literal( escaped, escaped->getText() );
  }

  return new_node( ctx, "interpolated-string-part",  //
                   "expression", expression,         //
                   "format", format                  //
  );
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

antlrcpp::Any JsonAstFileProcessor::visitPrimary(
    EscriptGrammar::EscriptParser::PrimaryContext* ctx )
{
  /*
  if ( auto identifier = ctx->IDENTIFIER() )
  {
    return make_identifier( identifier );
  }
  else if ( auto uninit = ctx->UNINIT() )
  {
    return new_node( uninit, "uninitialized-value" );
  }
  else if ( auto uninit = ctx->BOOL_TRUE() )
  {
    return new_node( uninit, "boolean-value",  //
                     "value", true             //
    );
  }
  else if ( auto uninit = ctx->BOOL_FALSE() )
  {
    return new_node( uninit, "uninitialized-value",  //
                     "value", false                  //
    );
  }
*/
  if ( auto literal = ctx->literal() )
  {
    return visitLiteral( literal );
  }
  else if ( auto parExpression = ctx->parExpression() )
  {
    return visitExpression( parExpression->expression() );
  }
  // else if (auto functionCall = ctx->functionCall()) { return make_function_call(functionCall,
  // nullptr); } else if (auto scopedFunctionCall = ctx->scopedFunctionCall()) { return
  // make_function_call(scopedFunctionCall->functionCall(), scopedFunctionCall->IDENTIFIER()); }
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

  return ctx->children.at( 0 )->accept( this );
}

antlrcpp::Any JsonAstFileProcessor::visitProgramDeclaration(
    EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx )
{
  return new_node( ctx, "program",                                                    //
                   "name", make_identifier( ctx->IDENTIFIER() ),                      //
                   "parameters", visitProgramParameters( ctx->programParameters() ),  //
                   "body", visitBlock( ctx->block() )                                 //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitProgramParameter(
    EscriptGrammar::EscriptParser::ProgramParameterContext* ctx )
{
  return new_node( ctx, "program-parameter",                      //
                   "name", make_identifier( ctx->IDENTIFIER() ),  //
                   "unused", ctx->UNUSED() ? true : false,        //
                   "init",
                   ctx->expression() ? visitExpression( ctx->expression() ) : antlrcpp::Any()  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitStatement(
    EscriptGrammar::EscriptParser::StatementContext* ctx )
{
  if ( auto expression = ctx->statementExpression )
  {
    return new_node( ctx, "expression-statement",                 //
                     "expression", visitExpression( expression )  //
    );
  }
  return visitChildren( ctx );
  // return antlrcpp::Any();
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
    init = visitExpression( expression );
  }
  return new_node( ctx, "struct-initializer",  //
                   "name", name,               //
                   "init", init                //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitUseDeclaration( EscriptParser::UseDeclarationContext* ctx )
{
  return new_node( ctx, "use-declaration",                                        //
                   "specifier", visitStringIdentifier( ctx->stringIdentifier() )  //
  );
}


antlrcpp::Any JsonAstFileProcessor::make_function_call(
    EscriptGrammar::EscriptParser::FunctionCallContext* ctx, antlr4::tree::TerminalNode* scope )
{
  return new_node( ctx, "function-call-expression",                               //
                   "callee", make_identifier( ctx->IDENTIFIER() ),                //
                   "arguments", visitChildren( ctx ),                             //
                   "scope", scope ? make_identifier( scope ) : picojson::value()  //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_identifier( antlr4::tree::TerminalNode* terminal )
{
  return new_node( terminal, "identifier",    //
                   "id", terminal->getText()  //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_string_literal( antlr4::tree::TerminalNode* terminal,
                                                         const std::string& text )
{
  return new_node( terminal, "string-value",  //
                   "value", text,             //
                   "raw", text                //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_string_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();

  return new_node( terminal, "string-value",                      //
                   "value", text.substr( 1, text.length() - 2 ),  //
                   "raw", text                                    //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_integer_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();

  return new_node( terminal, "integer-literal",  //
                   "value", std::stoi( text ),   //
                   "raw", text                   //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_float_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();

  return new_node( terminal, "float-literal",   //
                   "value", std::stod( text ),  //
                   "raw", text                  //
  );
}
antlrcpp::Any JsonAstFileProcessor::make_bool_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();
  bool value = terminal->getSymbol()->getType() == EscriptGrammar::EscriptLexer::BOOL_TRUE;
  return new_node( terminal, "float-literal",  //
                   "value", value,             //
                   "raw", text                 //
  );
}

}  // namespace Pol::Bscript::Compiler
