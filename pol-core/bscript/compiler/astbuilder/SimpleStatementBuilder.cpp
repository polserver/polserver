#include "SimpleStatementBuilder.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/BinaryOperator.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/DebugStatementMarker.h"
#include "bscript/compiler/ast/EnumDeclaration.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IndexUnpacking.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/JumpStatement.h"
#include "bscript/compiler/ast/MemberUnpacking.h"
#include "bscript/compiler/ast/ReturnStatement.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/UnpackingList.h"
#include "bscript/compiler/ast/UnpackingStatement.h"
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
      else if ( auto unpacking_decl = decl->unpackingDeclaration() )
      {
        auto unpackings = unpacking_list( class_name, unpacking_decl );
        auto initializer = unpacking_initializer( decl->unpackingDeclarationInitializer() );
        auto unpacking_ast = std::make_unique<UnpackingStatement>(
            location_for( *decl ), std::move( unpackings ), std::move( initializer ) );

        statements.push_back( std::move( unpacking_ast ) );
      }
    }
  }
}

std::unique_ptr<Expression> SimpleStatementBuilder::unpacking_initializer(
    EscriptGrammar::EscriptParser::UnpackingDeclarationInitializerContext* ctx )
{
  return expression( ctx->expression() );
}

std::unique_ptr<Node> SimpleStatementBuilder::unpacking_list(
    const std::string& class_name, EscriptGrammar::EscriptParser::UnpackingDeclarationContext* ctx )
{
  std::vector<std::unique_ptr<Node>> unpackings;
  bool index_unpacking = false;

  if ( auto indexed_unpacking_list = ctx->indexUnpackingList() )
  {
    index_unpacking = true;
    for ( auto* indexed_unpacking : indexed_unpacking_list->indexUnpacking() )
    {
      if ( auto identifier = indexed_unpacking->IDENTIFIER() )
      {
        bool rest = indexed_unpacking->ELLIPSIS();
        unpackings.push_back( std::make_unique<IndexUnpacking>(
            location_for( *indexed_unpacking ), class_name, text( identifier ), rest ) );
      }
      else if ( auto unpacking_decl = indexed_unpacking->unpackingDeclaration() )
      {
        unpackings.push_back( unpacking_list( class_name, unpacking_decl ) );
      }
      else
      {
        report.error( location_for( *indexed_unpacking ), "Unsupported indexed unpacking" );
        break;
      }
    }
  }
  else if ( auto named_unpacking_list = ctx->memberUnpackingList() )
  {
    for ( auto* named_unpacking : named_unpacking_list->memberUnpacking() )
    {
      std::unique_ptr<Expression> member;
      std::unique_ptr<Node> child_unpacking;
      std::string member_text;

      if ( auto expr = named_unpacking->expression() )
      {
        member = this->expression( expr );
      }
      else if ( auto identifier = named_unpacking->IDENTIFIER() )
      {
        member_text = text( identifier );
        member = std::make_unique<StringValue>( location_for( *named_unpacking ), member_text );
      }
      else
      {
        report.error( location_for( *named_unpacking ), "Unsupported unpacking" );
        break;
      }

      if ( auto unpacking = named_unpacking->unpacking() )
      {
        if ( auto identifier = unpacking->IDENTIFIER() )
        {
          unpackings.push_back( std::make_unique<MemberUnpacking>(
              location_for( *unpacking ), class_name, text( identifier ), std::move( member ) ) );
        }
        else if ( auto unpacking_decl = unpacking->unpackingDeclaration() )
        {
          unpackings.push_back(
              std::make_unique<MemberUnpacking>( location_for( *unpacking ), std::move( member ),
                                                 unpacking_list( class_name, unpacking_decl ) ) );
        }
        else
        {
          report.error( location_for( *named_unpacking ), "Unsupported unpacking" );
          break;
        }
      }
      else if ( !member_text.empty() )
      {
        if ( named_unpacking->ELLIPSIS() )
        {
          unpackings.push_back( std::make_unique<MemberUnpacking>( location_for( *named_unpacking ),
                                                                   class_name, member_text ) );
        }
        else
        {
          unpackings.push_back( std::make_unique<MemberUnpacking>(
              location_for( *named_unpacking ), class_name, member_text, std::move( member ) ) );
        }
      }
      else
      {
        report.error( location_for( *named_unpacking ), "Unsupported unpacking" );
        break;
      }
    }
  }
  else
  {
    report.error( location_for( *ctx ), "Unsupported unpacking list" );
  }

  return std::make_unique<UnpackingList>( location_for( *ctx ), std::move( unpackings ),
                                          index_unpacking );
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
