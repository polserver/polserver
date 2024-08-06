#include "FunctionResolver.h"

#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Function.h"
#include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/astbuilder/AvailableUserFunction.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/file/SourceLocation.h"
#include "bscript/compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
FunctionResolver::FunctionResolver( Report& report ) : report( report ) {}
FunctionResolver::~FunctionResolver() = default;

const Function* FunctionResolver::find( const std::string& scoped_name )
{
  auto itr = resolved_functions_by_name.find( scoped_name );
  if ( itr != resolved_functions_by_name.end() )
    return ( *itr ).second;
  else
    return nullptr;
}

void FunctionResolver::force_reference( const std::string& function_name,
                                        const SourceLocation& loc )
{
  register_function_link( function_name, std::make_shared<FunctionLink>( loc ) );
}

void FunctionResolver::register_available_user_function(
    const SourceLocation& source_location,
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  register_available_user_function_parse_tree( source_location, ctx, ctx->IDENTIFIER(),
                                               ctx->EXPORTED() );
}

void FunctionResolver::register_available_class_decl(
    const SourceLocation& loc, EscriptGrammar::EscriptParser::ClassDeclarationContext* ctx )
{
  register_available_class_decl_parse_tree( loc, ctx, ctx->IDENTIFIER() );
}

void FunctionResolver::register_function_link( const std::string& name,
                                               std::shared_ptr<FunctionLink> function_link )
{
  auto already_resolved_itr = resolved_functions_by_name.find( name );
  if ( already_resolved_itr != resolved_functions_by_name.end() )
  {
    function_link->link_to( ( *already_resolved_itr ).second );
  }
  else
  {
    unresolved_function_links_by_name[name].push_back( std::move( function_link ) );
  }
}

std::string FunctionResolver::register_function_expression(
    const SourceLocation& source_location,
    EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx )
{
  auto name = function_expression_name( source_location );
  auto apt = AvailableParseTree{ source_location, ctx, false };
  available_parse_trees.insert( { name, apt } );
  return name;
}

void FunctionResolver::register_module_function( ModuleFunctionDeclaration* mf )
{
  const auto& name = mf->name;
  auto scoped_name = mf->module_name + "::" + name;

  auto itr = available_parse_trees.find( name );
  if ( itr != available_parse_trees.end() )
  {
    const auto& previous = ( *itr ).second;

    report.error( previous.source_location,
                  "User Function '{}' conflicts with Module Function of the same name.\n"
                  "  Module Function declaration: {}",
                  name, mf->source_location );
  }

  resolved_functions_by_name[name] = mf;
  resolved_functions_by_name[scoped_name] = mf;
}

void FunctionResolver::register_user_function( UserFunction* uf )
{
  resolved_functions_by_name[uf->name] = uf;
}

bool FunctionResolver::resolve( std::vector<AvailableParseTree>& to_build_ast )
{
  for ( auto unresolved_itr = unresolved_function_links_by_name.begin();
        unresolved_itr != unresolved_function_links_by_name.end(); )
  {
    const std::string& name = ( *unresolved_itr ).first;
    const std::vector<std::shared_ptr<FunctionLink>>& function_links = ( *unresolved_itr ).second;

    auto previously_resolved_itr = resolved_functions_by_name.find( name );
    if ( previously_resolved_itr != resolved_functions_by_name.end() )
    {
      Function* resolved_function = ( *previously_resolved_itr ).second;
      for ( auto& function_link : function_links )
      {
        function_link->link_to( resolved_function );
      }

      unresolved_itr = unresolved_function_links_by_name.erase( unresolved_itr );
    }
    else
    {
      auto available_itr = available_parse_trees.find( name );
      if ( available_itr != available_parse_trees.end() )
      {
        to_build_ast.push_back( ( *available_itr ).second );
        available_parse_trees.erase( available_itr );
        ++unresolved_itr;
      }
      else
      {
        // Do not complain here, as the identifier in `name` may be a variable.
        // Error handled in SemanticAnalyzer.
        unresolved_itr = unresolved_function_links_by_name.erase( unresolved_itr );
      }
    }
  }

  return !to_build_ast.empty();
}

std::string FunctionResolver::function_expression_name( const SourceLocation& source_location )
{
  return fmt::format( "funcexpr@{}:{}:{}", source_location.source_file_identifier->index,
                      source_location.range.start.line_number,
                      source_location.range.start.character_column );
}

void FunctionResolver::register_available_user_function_parse_tree(
    const SourceLocation& source_location, antlr4::ParserRuleContext* ctx,
    antlr4::tree::TerminalNode* identifier, antlr4::tree::TerminalNode* exported )
{
  std::string name = identifier->getSymbol()->getText();
  auto itr = available_parse_trees.find( name );
  if ( itr != available_parse_trees.end() )
  {
    AvailableParseTree& previous = ( *itr ).second;

    if ( previous.class_declaration )
    {
      report.error( source_location,
                    "Class '{}' conflicts with User Function of the same name.\n"
                    "  Previous declaration: {}",
                    name, previous.source_location );
    }
    else
    {
      report.error( source_location,
                    "Function '{}' defined more than once.\n"
                    "  Previous declaration: {}",
                    name, previous.source_location );
    }
  }

  auto itr2 = resolved_functions_by_name.find( name );
  if ( itr2 != resolved_functions_by_name.end() )
  {
    auto* previous = ( *itr2 ).second;

    report.error( source_location,
                  "User Function '{}' conflicts with Module Function of the same name.\n"
                  "  Module Function declaration: {}",
                  name, previous->source_location );
  }

  auto apt = AvailableParseTree{ source_location, ctx, false };
  available_parse_trees.insert( { name, apt } );

  if ( exported )
  {
    auto function_name = identifier->getSymbol()->getText();

    // just make sure there is an entry, so that we build an AST for it
    unresolved_function_links_by_name[function_name];
  }
}

void FunctionResolver::register_available_class_decl_parse_tree(
    const SourceLocation& source_location, antlr4::ParserRuleContext* ctx,
    antlr4::tree::TerminalNode* identifier )
{
  std::string name = identifier->getSymbol()->getText();
  auto itr = available_parse_trees.find( name );
  if ( itr != available_parse_trees.end() )
  {
    AvailableParseTree& previous = ( *itr ).second;
    if ( !previous.class_declaration )
    {
      report.error( source_location,
                    "Class '{}' conflicts with User Function of the same name.\n"
                    "  Previous declaration: {}",
                    name, previous.source_location );
    }
    else
    {
      report.error( source_location,
                    "Class '{}' defined more than once.\n"
                    "  Previous declaration: {}",
                    name, previous.source_location );
    }
  }

  auto itr2 = resolved_functions_by_name.find( name );
  if ( itr2 != resolved_functions_by_name.end() )
  {
    auto* previous = ( *itr2 ).second;

    report.error( source_location,
                  "Class '{}' conflicts with Module Function of the same name.\n"
                  "  Module Function declaration: {}",
                  name, previous->source_location );
  }


  auto apt = AvailableParseTree{ source_location, ctx, true };
  available_parse_trees.insert( { name, apt } );
}

}  // namespace Pol::Bscript::Compiler
