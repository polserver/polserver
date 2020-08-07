#include "FunctionResolver.h"

#include "AvailableUserFunction.h"
#include "SourceFileProcessor.h"
#include "compiler/Profile.h"
#include "compiler/Report.h"
#include "compiler/ast/Function.h"
#include "compiler/ast/ModuleFunctionDeclaration.h"
#include "compiler/ast/UserFunction.h"
#include "compiler/file/SourceLocation.h"
#include "compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
FunctionResolver::FunctionResolver( Report& report ) : report( report ) {}

const Function* FunctionResolver::find( const std::string& scoped_name )
{
  auto itr = resolved_functions_by_name.find( scoped_name );
  if ( itr != resolved_functions_by_name.end() )
    return ( *itr ).second;
  else
    return nullptr;
}

void FunctionResolver::register_available_user_function(
    const SourceLocation& source_location,
    EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx )
{
  register_available_user_function_parse_tree( source_location, ctx, ctx->IDENTIFIER(),
                                               ctx->EXPORTED() );
}

void FunctionResolver::register_available_user_function(
    const SourceLocation& source_location,
    EscriptGrammar::EscriptParser::UnambiguousFunctionDeclarationContext* ctx )
{
  register_available_user_function_parse_tree( source_location, ctx, ctx->IDENTIFIER(),
                                               ctx->EXPORTED() );
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

void FunctionResolver::register_module_function( ModuleFunctionDeclaration* mf )
{
  resolved_functions_by_name[mf->name] = mf;
  auto scoped_name = mf->module_name + "::" + mf->name;
  resolved_functions_by_name[scoped_name] = mf;
}

void FunctionResolver::register_user_function( UserFunction* uf )
{
  resolved_functions_by_name[uf->name] = uf;
}

bool FunctionResolver::resolve( std::vector<AvailableUserFunction>& to_build_ast )
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
      auto available_itr = available_user_function_parse_trees.find( name );
      if ( available_itr != available_user_function_parse_trees.end() )
      {
        to_build_ast.push_back( ( *available_itr ).second );
        available_user_function_parse_trees.erase( available_itr );
        ++unresolved_itr;
      }
      else
      {
        // complain, super complain
        const std::shared_ptr<FunctionLink>& function_link = ( function_links )[0];
        report.error( function_link->source_location, "User Function '", name, "' not found.\n" );
        unresolved_itr = unresolved_function_links_by_name.erase( unresolved_itr );
      }
    }
  }

  return !to_build_ast.empty();
}

void FunctionResolver::register_available_user_function_parse_tree(
    const SourceLocation& source_location, antlr4::ParserRuleContext* ctx,
    antlr4::tree::TerminalNode* identifier, antlr4::tree::TerminalNode* exported )
{
  std::string name = identifier->getSymbol()->getText();
  auto itr = available_user_function_parse_trees.find( name );
  if ( itr != available_user_function_parse_trees.end() )
  {
    AvailableUserFunction& previous = ( *itr ).second;

    report.error( source_location, "Function '", name, "' defined more than once.\n",
                  "  Previous declaration: ", previous.source_location, "\n" );
  }

  auto auf = AvailableUserFunction{ source_location, ctx };
  available_user_function_parse_trees.insert( { name, auf } );

  if ( exported )
  {
    auto function_name = identifier->getSymbol()->getText();

    // just make sure there is an entry, so that we build an AST for it
    unresolved_function_links_by_name[function_name];
  }
}

}  // namespace Pol::Bscript::Compiler
