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

antlrcpp::Any JsonAstFileProcessor::process_compilation_unit( SourceFile& sf )
{
  if ( auto compilation_unit = sf.get_compilation_unit( report, source_file_identifier ) )
  {
    return compilation_unit->accept( this );
  }
  throw std::runtime_error( "No compilation unit in source file" );
}

antlrcpp::Any JsonAstFileProcessor::process_module_unit( SourceFile& sf )
{
  if ( auto module_unit = sf.get_module_unit( report, source_file_identifier ) )
  {
    return module_unit->accept( this );
  }
  throw std::runtime_error( "No compilation unit in source file" );
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

antlrcpp::Any JsonAstFileProcessor::visitCompilationUnit(
    EscriptGrammar::EscriptParser::CompilationUnitContext* ctx )
{
  return new_node( ctx, "file",                   //
                   "body", visitChildren( ctx ),  //
                   "module", false                //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitVarStatement(
    EscriptGrammar::EscriptParser::VarStatementContext* ctx )
{
  auto declarations = visitVariableDeclarationList( ctx->variableDeclarationList() );

  return new_node( ctx, "var-statement",         //
                   "declarations", declarations  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitWhileStatement(
    EscriptGrammar::EscriptParser::WhileStatementContext* ctx )
{
  auto label = make_statement_label( ctx->statementLabel() );
  auto body = visitBlock( ctx->block() );
  auto test = visitExpression( ctx->parExpression()->expression() );

  return new_node( ctx, "while-statement",       //
                   "label", std::move( label ),  //
                   "test", std::move( test ),    //
                   "body", std::move( body )     //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitVariableDeclaration(
    EscriptGrammar::EscriptParser::VariableDeclarationContext* ctx )
{
  // auto const_declaration_ctx = ctx->constantDeclaration();
  antlrcpp::Any init;

  if ( auto variable_declaration_initializer = ctx->variableDeclarationInitializer() )
  {
    if ( auto expression = variable_declaration_initializer->expression() )
    {
      init = visitExpression( expression );
    }

    else if ( variable_declaration_initializer->ARRAY() )
    {
      init = new_node( ctx, "array-expression",     //
                       "elements", defaultResult()  //
      );
    }
  }

  return new_node( ctx, "variable-declaration",                   //
                   "name", make_identifier( ctx->IDENTIFIER() ),  //
                   "init", std::move( init )                      //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitConstStatement(
    EscriptGrammar::EscriptParser::ConstStatementContext* ctx )
{
  auto const_declaration_ctx = ctx->constantDeclaration();
  antlrcpp::Any init;

  if ( auto variable_declaration_initializer =
           const_declaration_ctx->variableDeclarationInitializer() )
  {
    if ( auto expression = variable_declaration_initializer->expression() )
    {
      init = visitExpression( expression );
    }

    else if ( variable_declaration_initializer->ARRAY() )
    {
      init = new_node( ctx, "array-expression",     //
                       "elements", defaultResult()  //
      );
    }
  }

  return new_node( ctx, "const-statement",                                          //
                   "name", make_identifier( const_declaration_ctx->IDENTIFIER() ),  //
                   "init", std::move( init )                                        //
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
                   "value", std::move( init )                       //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitDoStatement(
    EscriptGrammar::EscriptParser::DoStatementContext* ctx )
{
  auto label = make_statement_label( ctx->statementLabel() );
  auto body = visitBlock( ctx->block() );
  auto test = visitExpression( ctx->parExpression()->expression() );

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
    value = visitExpression( expression );
  }

  return new_node( ctx, "enum-entry",                      //
                   "identifier", std::move( identifier ),  //
                   "value", std::move( value )             //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitEnumStatement(
    EscriptGrammar::EscriptParser::EnumStatementContext* ctx )
{
  auto identifier = make_identifier( ctx->IDENTIFIER() );
  auto enums = visitEnumList( ctx->enumList() );

  return new_node( ctx, "enum-statement",                  //
                   "identifier", std::move( identifier ),  //
                   "enums", std::move( enums )             //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitExitStatement(
    EscriptGrammar::EscriptParser::ExitStatementContext* ctx )
{
  return new_node( ctx, "exit-statement" );
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
  }
  else if ( ctx->QUESTION() )
  {
    return new_node( ctx, "conditional-expression",                           //
                     "conditional", visitExpression( ctx->expression( 0 ) ),  //
                     "consequent", visitExpression( ctx->expression( 1 ) ),   //
                     "alternate", visitExpression( ctx->expression( 2 ) )     //
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
                     "name", std::move( accessor ),         //
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
  auto identifier = make_identifier( ctx->IDENTIFIER() );
  auto label = make_statement_label( ctx->statementLabel() );
  auto expression = visitForeachIterableExpression( ctx->foreachIterableExpression() );
  auto body = visitBlock( ctx->block() );

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
  return new_node( ctx, "function-call-expression",                 //
                   "callee", make_identifier( ctx->IDENTIFIER() ),  //
                   "arguments", visitChildren( ctx ),               //
                   "scope", picojson::value()                       //
  );
}
antlrcpp::Any JsonAstFileProcessor::visitFunctionDeclaration(
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  bool exported = ctx->EXPORTED();
  auto name = make_identifier( ctx->IDENTIFIER() );
  auto parameters = visitFunctionParameters( ctx->functionParameters() );
  auto body = visitBlock( ctx->block() );

  return new_node( ctx, "function-declaration",            //
                   "name", std::move( name ),              //
                   "parameters", std::move( parameters ),  //
                   "exported", std::move( exported ),      //
                   "body", std::move( body )               //
  );
}
antlrcpp::Any JsonAstFileProcessor::visitFunctionParameter(
    EscriptGrammar::EscriptParser::FunctionParameterContext* ctx )
{
  antlrcpp::Any init;
  bool byref = ctx->BYREF();
  bool unused = ctx->UNUSED();
  auto name = make_identifier( ctx->IDENTIFIER() );

  if ( auto expression = ctx->expression() )
  {
    init = visitExpression( expression );
  }

  return new_node( ctx, "function-parameter",  //
                   "name", std::move( name ),  //
                   "init", std::move( init ),  //
                   "byref", byref,             //
                   "unused", unused            //
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
    labels = aggregateResult( std::move( labels ), visitSwitchLabel( switchLabel ) );
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
    return visitIntegerLiteral( integerLiteral );
  }
  else if ( auto boolLiteral = ctx->boolLiteral() )
  {
    return visitBoolLiteral( boolLiteral );
  }
  else if ( auto uninit = ctx->UNINIT() )
  {
    return new_node( uninit, "uninitialized-value" );
  }
  else if ( auto identifier = ctx->IDENTIFIER() )
  {
    return make_identifier( identifier );
  }
  else if ( auto string_literal = ctx->STRING_LITERAL() )
  {
    return make_string_literal( string_literal );
  }
  else if ( ctx->DEFAULT() )
  {
    return new_node( ctx->DEFAULT(), "default-case-label" );
  }

  return antlrcpp::Any();
}

antlrcpp::Any JsonAstFileProcessor::visitCaseStatement(
    EscriptGrammar::EscriptParser::CaseStatementContext* ctx )
{
  auto label = make_statement_label( ctx->statementLabel() );
  auto test = visitExpression( ctx->expression() );

  auto cases = defaultResult();
  for ( const auto& switchBlockStatementGroup : ctx->switchBlockStatementGroup() )
  {
    cases = aggregateResult( std::move( cases ),
                             visitSwitchBlockStatementGroup( switchBlockStatementGroup ) );
  }

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

  return new_node( ctx, "continue-statement",   //
                   "label", std::move( label )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitBasicForStatement(
    EscriptGrammar::EscriptParser::BasicForStatementContext* ctx )
{
  auto identifier = make_identifier( ctx->IDENTIFIER() );
  auto first = visitExpression( ctx->expression( 0 ) );
  auto last = visitExpression( ctx->expression( 1 ) );

  return new_node( ctx, "basic-for-statement",             //
                   "identifier", std::move( identifier ),  //
                   "first", std::move( first ),            //
                   "last", std::move( last )               //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitCstyleForStatement(
    EscriptGrammar::EscriptParser::CstyleForStatementContext* ctx )
{
  auto body = visitBlock( ctx->block() );
  auto initializer = visitExpression( ctx->expression( 0 ) );
  auto test = visitExpression( ctx->expression( 1 ) );
  auto advancer = visitExpression( ctx->expression( 2 ) );

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
  auto body = visitBlock( ctx->block() );
  auto test = visitExpression( ctx->expression() );

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

  if ( auto expression = ctx->expression() )
  {
    value = visitExpression( expression );
  }

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
  return new_node( ctx, "function-reference-expression",         //
                   "name", make_identifier( ctx->IDENTIFIER() )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitGotoStatement(
    EscriptGrammar::EscriptParser::GotoStatementContext* ctx )
{
  return new_node( ctx, "goto-statement",                         //
                   "label", make_identifier( ctx->IDENTIFIER() )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitIfStatement(
    EscriptGrammar::EscriptParser::IfStatementContext* ctx )
{
  auto blocks = ctx->block();
  auto par_expression = ctx->parExpression();

  antlrcpp::Any else_clause;

  if ( ctx->ELSE() )
  {
    else_clause = visitBlock( blocks.at( blocks.size() - 1 ) );
  }

  picojson::value if_statement_ast;

  size_t num_expressions = par_expression.size();
  for ( auto clause_index = num_expressions; clause_index != 0; )
  {
    --clause_index;
    auto expression_ctx = par_expression.at( clause_index );
    auto expression_ast = visitExpression( expression_ctx->expression() );
    antlrcpp::Any consequent_ast;

    if ( blocks.size() > clause_index )
    {
      consequent_ast = visitBlock( blocks.at( clause_index ) );
    }

    auto alternative_ast = if_statement_ast.is<picojson::null>() ? std::move( else_clause )
                                                                 : std::move( if_statement_ast );

    bool elseif = clause_index != 0;

    if_statement_ast = new_node( ctx, "if-statement",                          //
                                 "test", std::move( expression_ast ),          //
                                 "consequent", std::move( consequent_ast ),    //
                                 "alternative", std::move( alternative_ast ),  //
                                 "elseif", elseif                              //
    );
  }

  return if_statement_ast;
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

  return antlrcpp::Any();
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
  antlrcpp::Any init;
  bool unused = ctx->UNUSED();
  if ( auto expression = ctx->expression() )
  {
    init = visitExpression( expression );
  }
  return new_node( ctx, "program-parameter",                      //
                   "name", make_identifier( ctx->IDENTIFIER() ),  //
                   "unused", unused,                              //
                   "init", std::move( init )                      //
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
    return new_node( ctx, "expression-statement",                 //
                     "expression", visitExpression( expression )  //
    );
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
    init = visitExpression( expression );
  }
  return new_node( ctx, "struct-initializer",  //
                   "name", std::move( name ),  //
                   "init", std::move( init )   //
  );
}

antlrcpp::Any JsonAstFileProcessor::visitUseDeclaration( EscriptParser::UseDeclarationContext* ctx )
{
  return new_node( ctx, "use-declaration",                                        //
                   "specifier", visitStringIdentifier( ctx->stringIdentifier() )  //
  );
}

antlrcpp::Any JsonAstFileProcessor::make_statement_label(
    EscriptGrammar::EscriptParser::StatementLabelContext* ctx )
{
  antlrcpp::Any label;

  if ( ctx )
  {
    label = make_identifier( ctx->IDENTIFIER() );
  }

  return label;
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
  return new_node( terminal, "boolean-literal",  //
                   "value", value,               //
                   "raw", text                   //
  );
}

}  // namespace Pol::Bscript::Compiler
