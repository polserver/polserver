#include "SimpleStatementBuilder.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/BindingList.h"
#include "bscript/compiler/ast/BindingStatement.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/DebugStatementMarker.h"
#include "bscript/compiler/ast/EnumDeclaration.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IndexBinding.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/JumpStatement.h"
#include "bscript/compiler/ast/MemberBinding.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/VarStatement.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
SimpleStatementBuilder::SimpleStatementBuilder( const SourceFileIdentifier& source_file_identifier,
                                                BuilderWorkspace& workspace )
    : ExpressionBuilder( source_file_identifier, workspace )
{
}

void SimpleStatementBuilder::add_intrusive_debug_marker(
    antlr4::ParserRuleContext* ctx, std::vector<std::unique_ptr<Statement>>& statements )
{
  statements.push_back( std::make_unique<DebugStatementMarker>(
      location_for( *ctx ), ctx->getText(),
      static_cast<unsigned>( ctx->start->getStartIndex() ) ) );
}

void SimpleStatementBuilder::add_var_statements(
    EscriptParser::VarStatementContext* ctx, const std::string& class_name,
    std::vector<std::unique_ptr<Statement>>& statements )
{
  if ( auto variable_declaration_list = ctx->variableDeclarationList() )
  {
    for ( auto decl : variable_declaration_list->variableDeclaration() )
    {
      if ( auto identifier = decl->IDENTIFIER() )
      {
        auto loc = location_for( *decl );
        std::string name = text( identifier );
        std::unique_ptr<VarStatement> var_ast;

        if ( auto initializer_context = decl->variableDeclarationInitializer() )
        {
          if ( initializer_context->ARRAY() )
          {
            var_ast = std::make_unique<VarStatement>( loc, class_name, std::move( name ), true );
          }
          else
          {
            auto initializer = variable_initializer( initializer_context );
            var_ast = std::make_unique<VarStatement>( loc, class_name, std::move( name ),
                                                      std::move( initializer ) );
          }
        }
        else
        {
          var_ast = std::make_unique<VarStatement>( loc, class_name, std::move( name ) );
        }
        statements.push_back( std::move( var_ast ) );
      }
      else if ( auto binding_decl = decl->bindingDeclaration() )
      {
        auto bindings = binding_list( binding_decl );
        auto initializer = binding_initializer( decl->bindingDeclarationInitializer() );
        auto binding_ast = std::make_unique<BindingStatement>(
            location_for( *decl ), std::move( bindings ), std::move( initializer ) );

        statements.push_back( std::move( binding_ast ) );
      }
    }
  }
}

std::unique_ptr<Expression> SimpleStatementBuilder::binding_initializer(
    EscriptGrammar::EscriptParser::BindingDeclarationInitializerContext* ctx )
{
  return expression( ctx->expression() );
}

std::unique_ptr<Node> SimpleStatementBuilder::binding_list(
    EscriptGrammar::EscriptParser::BindingDeclarationContext* ctx )
{
  std::vector<std::unique_ptr<Node>> bindings;
  bool index_binding = false;

  if ( auto indexed_binding_list = ctx->indexBindingList() )
  {
    index_binding = true;
    for ( auto* indexed_binding : indexed_binding_list->indexBinding() )
    {
      if ( auto identifier = indexed_binding->IDENTIFIER() )
      {
        bool rest = indexed_binding->ELLIPSIS();
        bindings.push_back( std::make_unique<IndexBinding>( location_for( *indexed_binding ),
                                                            text( identifier ), rest ) );
      }
      else if ( auto binding_decl = indexed_binding->bindingDeclaration() )
      {
        bindings.push_back( binding_list( binding_decl ) );
      }
      else
      {
        report.error( location_for( *indexed_binding ), "Unsupported indexed binding" );
        break;
      }
    }
  }
  else if ( auto named_binding_list = ctx->memberBindingList() )
  {
    for ( auto* named_binding : named_binding_list->memberBinding() )
    {
      std::unique_ptr<Expression> member;
      std::unique_ptr<Node> child_binding;
      std::string member_text;

      if ( auto expr = named_binding->expression() )
      {
        member = this->expression( expr );
      }
      else if ( auto identifier = named_binding->IDENTIFIER() )
      {
        member_text = text( identifier );
        member = std::make_unique<StringValue>( location_for( *named_binding ), member_text );
      }
      else
      {
        report.error( location_for( *named_binding ), "Unsupported binding" );
        break;
      }

      if ( auto binding = named_binding->binding() )
      {
        if ( auto identifier = binding->IDENTIFIER() )
        {
          bindings.push_back( std::make_unique<MemberBinding>(
              location_for( *binding ), text( identifier ), std::move( member ) ) );
        }
        else if ( auto binding_decl = binding->bindingDeclaration() )
        {
          bindings.push_back( std::make_unique<MemberBinding>(
              location_for( *binding ), std::move( member ), binding_list( binding_decl ) ) );
        }
        else
        {
          report.error( location_for( *named_binding ), "Unsupported binding" );
          break;
        }
      }
      else if ( !member_text.empty() )
      {
        if ( named_binding->ELLIPSIS() )
        {
          bindings.push_back(
              std::make_unique<MemberBinding>( location_for( *named_binding ), member_text ) );
        }
        else
        {
          bindings.push_back( std::make_unique<MemberBinding>( location_for( *named_binding ),
                                                               member_text, std::move( member ) ) );
        }
      }
      else
      {
        report.error( location_for( *named_binding ), "Unsupported binding" );
        break;
      }
    }
  }
  else
  {
    report.error( location_for( *ctx ), "Unsupported binding list" );
  }

  return std::make_unique<BindingList>( location_for( *ctx ), std::move( bindings ),
                                        index_binding );
}

std::unique_ptr<JumpStatement> SimpleStatementBuilder::break_statement(
    EscriptParser::BreakStatementContext* ctx )
{
  auto source_location = location_for( *ctx );
  std::string label = ctx->IDENTIFIER() ? text( ctx->IDENTIFIER() ) : "";

  return std::make_unique<JumpStatement>( source_location, JumpStatement::Break,
                                          std::move( label ) );
}

std::unique_ptr<ConstDeclaration> SimpleStatementBuilder::const_declaration(
    EscriptParser::ConstStatementContext* ctx )
{
  auto variable_declaration = ctx->constantDeclaration();
  auto identifier = text( variable_declaration->IDENTIFIER() );
  auto expression_context = variable_declaration->variableDeclarationInitializer()->expression();
  auto value = expression( expression_context );

  return std::make_unique<ConstDeclaration>( location_for( *ctx ), std::move( identifier ),
                                             std::move( value ) );
}

std::unique_ptr<JumpStatement> SimpleStatementBuilder::continue_statement(
    EscriptParser::ContinueStatementContext* ctx )
{
  auto source_location = location_for( *ctx );
  std::string label = ctx->IDENTIFIER() ? text( ctx->IDENTIFIER() ) : "";

  return std::make_unique<JumpStatement>( source_location, JumpStatement::Continue,
                                          std::move( label ) );
}

std::unique_ptr<EnumDeclaration> SimpleStatementBuilder::enum_declaration(
    EscriptParser::EnumStatementContext* ctx )
{
  std::vector<std::string> names;
  std::vector<std::unique_ptr<Expression>> expressions;
  if ( auto enum_list = ctx->enumList() )
  {
    std::string last_identifier;
    for ( auto entry : enum_list->enumListEntry() )
    {
      auto source_location = location_for( *entry );
      std::string identifier = text( entry->IDENTIFIER() );
      std::unique_ptr<Expression> value;
      if ( auto expr_ctx = entry->expression() )
      {
        value = expression( expr_ctx );
      }
      else if ( !last_identifier.empty() )
      {
        // The optimizer runs later, so we don't necessarily know the value of
        // the previous enum value.  The optimizer will sort it out.
        auto lhs = std::make_unique<Identifier>( source_location, last_identifier );
        auto one = std::make_unique<IntegerValue>( source_location, 1 );
        value = std::make_unique<BinaryOperator>( source_location, std::move( lhs ), "+", TOK_ADD,
                                                  std::move( one ) );
      }
      else
      {
        value = std::make_unique<IntegerValue>( source_location, 0 );
      }
      bool allow_overwrite = true;
      auto constant = std::make_unique<ConstDeclaration>( location_for( *entry ), identifier,
                                                          std::move( value ), allow_overwrite );
      workspace.compiler_workspace.const_declarations.push_back( std::move( constant ) );

      last_identifier = identifier;
    }
  }

  auto source_location = location_for( *ctx );
  std::string identifier = text( ctx->IDENTIFIER() );
  return std::make_unique<EnumDeclaration>( source_location, std::move( identifier ),
                                            std::move( names ), std::move( expressions ) );
}

std::unique_ptr<Expression> SimpleStatementBuilder::variable_initializer(
    EscriptParser::VariableDeclarationInitializerContext* ctx )
{
  if ( auto expr = ctx->expression() )
    return expression( expr );
  else
    return std::unique_ptr<Expression>( new StringValue( location_for( *ctx ), "" ) );
}

std::unique_ptr<ReturnStatement> SimpleStatementBuilder::return_statement(
    EscriptParser::ReturnStatementContext* ctx )
{
  auto source_location = location_for( *ctx );

  std::unique_ptr<Expression> result;

  // Always emit the expression if it exists, so we can get semantic analysis
  // errors.
  if ( auto expression_ctx = ctx->expression() )
  {
    result = expression( expression_ctx );
  }
  else
  {
    // Only emit the empty string if we're not in a constructor function.
    if ( !in_constructor_function.top() )
      result = std::unique_ptr<Expression>( new StringValue( source_location, "" ) );
  }

  return std::make_unique<ReturnStatement>( source_location, std::move( result ) );
}

}  // namespace Pol::Bscript::Compiler
