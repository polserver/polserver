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

// antlrcpp::Any JsonAstFileProcessor::visitExpression(
//     EscriptGrammar::EscriptParser::ExpressionContext* ctx )
// {
//   return antlrcpp::Any();
// }

antlrcpp::Any JsonAstFileProcessor::visitIncludeDeclaration(
    EscriptGrammar::EscriptParser::IncludeDeclarationContext* ctx )
{
  return new_node( ctx, "include-declaration",                                    //
                   "specifier", visitStringIdentifier( ctx->stringIdentifier() )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitPrimary(
    EscriptGrammar::EscriptParser::PrimaryContext* ctx )
{
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

  return visitChildren( ctx );
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
  return new_node( ctx, "program-parameter",                                                   //
                   "name", make_identifier( ctx->IDENTIFIER() ),                               //
                   "unused", ctx->UNUSED() ? true : false,                                     //
                   "init",                                                                     //
                   ctx->expression() ? visitExpression( ctx->expression() ) : antlrcpp::Any()  //
  );
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

antlrcpp::Any JsonAstFileProcessor::visitUseDeclaration( EscriptParser::UseDeclarationContext* ctx )
{
  return new_node( ctx, "use-declaration",                                        //
                   "specifier", visitStringIdentifier( ctx->stringIdentifier() )  //
  );
}


antlrcpp::Any JsonAstFileProcessor::make_identifier( antlr4::tree::TerminalNode* terminal )
{
  return new_node( terminal, "identifier",    //
                   "id", terminal->getText()  //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_string_literal( antlr4::tree::TerminalNode* terminal )
{
  auto text = terminal->getText();

  return new_node( terminal, "string-literal",                   //
                   "value", text.substr( 1, text.length() - 2 )  //
  );
}

}  // namespace Pol::Bscript::Compiler
