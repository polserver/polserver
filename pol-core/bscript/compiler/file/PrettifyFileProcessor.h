#pragma once

#include "bscript/compiler/file/PrettifyLineBuilder.h"
#include "bscript/compiler/file/SourceLocation.h"
#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include "PrettifyLineBuilder.h"

#include <memory>
#include <optional>
#include <string>

namespace Pol::Bscript::Compiler
{
class Profile;
class Report;
class SourceFileIdentifier;
class SourceFile;
class SourceFileLoader;

class PrettifyFileProcessor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  PrettifyFileProcessor( const SourceFileIdentifier&, SourceFileLoader&, Profile&, Report& );

  std::string prettify() const;

public:
  antlrcpp::Any visitArrayInitializer(
      EscriptGrammar::EscriptParser::ArrayInitializerContext* ctx ) override;
  antlrcpp::Any visitBareArrayInitializer(
      EscriptGrammar::EscriptParser::BareArrayInitializerContext* ctx ) override;
  antlrcpp::Any visitBasicForStatement(
      EscriptGrammar::EscriptParser::BasicForStatementContext* ctx ) override;
  antlrcpp::Any visitBinding( EscriptGrammar::EscriptParser::BindingContext* ctx ) override;
  antlrcpp::Any visitBindingDeclaration(
      EscriptGrammar::EscriptParser::BindingDeclarationContext* ctx ) override;
  antlrcpp::Any visitBindingDeclarationInitializer(
      EscriptGrammar::EscriptParser::BindingDeclarationInitializerContext* ctx ) override;
  antlrcpp::Any visitBlock( EscriptGrammar::EscriptParser::BlockContext* ctx ) override;
  antlrcpp::Any visitBoolLiteral( EscriptGrammar::EscriptParser::BoolLiteralContext* ctx ) override;
  antlrcpp::Any visitBreakStatement(
      EscriptGrammar::EscriptParser::BreakStatementContext* ctx ) override;
  antlrcpp::Any visitCaseStatement(
      EscriptGrammar::EscriptParser::CaseStatementContext* ctx ) override;
  antlrcpp::Any visitClassBody( EscriptGrammar::EscriptParser::ClassBodyContext* ctx ) override;
  antlrcpp::Any visitClassDeclaration(
      EscriptGrammar::EscriptParser::ClassDeclarationContext* ctx ) override;
  antlrcpp::Any visitClassParameters(
      EscriptGrammar::EscriptParser::ClassParametersContext* ctx ) override;
  antlrcpp::Any visitClassParameterList(
      EscriptGrammar::EscriptParser::ClassParameterListContext* ctx ) override;
  antlrcpp::Any visitCompilationUnit(
      EscriptGrammar::EscriptParser::CompilationUnitContext* ctx ) override;
  antlrcpp::Any visitConstStatement(
      EscriptGrammar::EscriptParser::ConstStatementContext* ctx ) override;
  antlrcpp::Any visitContinueStatement(
      EscriptGrammar::EscriptParser::ContinueStatementContext* ctx ) override;
  antlrcpp::Any visitCstyleForStatement(
      EscriptGrammar::EscriptParser::CstyleForStatementContext* ctx ) override;
  antlrcpp::Any visitDictInitializer(
      EscriptGrammar::EscriptParser::DictInitializerContext* ctx ) override;
  antlrcpp::Any visitDictInitializerExpression(
      EscriptGrammar::EscriptParser::DictInitializerExpressionContext* ctx ) override;
  antlrcpp::Any visitDictInitializerExpressionList(
      EscriptGrammar::EscriptParser::DictInitializerExpressionListContext* ctx ) override;
  antlrcpp::Any visitDoStatement( EscriptGrammar::EscriptParser::DoStatementContext* ctx ) override;
  antlrcpp::Any visitEnumList( EscriptGrammar::EscriptParser::EnumListContext* ctx ) override;
  antlrcpp::Any visitEnumListEntry(
      EscriptGrammar::EscriptParser::EnumListEntryContext* ctx ) override;
  antlrcpp::Any visitEnumStatement(
      EscriptGrammar::EscriptParser::EnumStatementContext* ctx ) override;
  antlrcpp::Any visitExitStatement(
      EscriptGrammar::EscriptParser::ExitStatementContext* ctx ) override;
  antlrcpp::Any visitExplicitArrayInitializer(
      EscriptGrammar::EscriptParser::ExplicitArrayInitializerContext* ctx ) override;
  antlrcpp::Any visitExplicitDictInitializer(
      EscriptGrammar::EscriptParser::ExplicitDictInitializerContext* ctx ) override;
  antlrcpp::Any visitExplicitErrorInitializer(
      EscriptGrammar::EscriptParser::ExplicitErrorInitializerContext* ctx ) override;
  antlrcpp::Any visitExplicitStructInitializer(
      EscriptGrammar::EscriptParser::ExplicitStructInitializerContext* ctx ) override;
  antlrcpp::Any visitExpression( EscriptGrammar::EscriptParser::ExpressionContext* ctx ) override;
  antlrcpp::Any visitExpressionList(
      EscriptGrammar::EscriptParser::ExpressionListContext* ctx ) override;
  antlrcpp::Any visitFloatLiteral(
      EscriptGrammar::EscriptParser::FloatLiteralContext* ctx ) override;
  antlrcpp::Any visitForeachIterableExpression(
      EscriptGrammar::EscriptParser::ForeachIterableExpressionContext* ctx ) override;
  antlrcpp::Any visitForeachStatement(
      EscriptGrammar::EscriptParser::ForeachStatementContext* ctx ) override;
  antlrcpp::Any visitForStatement(
      EscriptGrammar::EscriptParser::ForStatementContext* ctx ) override;
  antlrcpp::Any visitFunctionCall(
      EscriptGrammar::EscriptParser::FunctionCallContext* ctx ) override;
  antlrcpp::Any visitFunctionDeclaration(
      EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx ) override;
  antlrcpp::Any visitFunctionParameter(
      EscriptGrammar::EscriptParser::FunctionParameterContext* ctx ) override;
  antlrcpp::Any visitFunctionParameterList(
      EscriptGrammar::EscriptParser::FunctionParameterListContext* ctx ) override;
  antlrcpp::Any visitFunctionParameters(
      EscriptGrammar::EscriptParser::FunctionParametersContext* ctx ) override;
  antlrcpp::Any visitFunctionReference(
      EscriptGrammar::EscriptParser::FunctionReferenceContext* ctx ) override;
  antlrcpp::Any visitFunctionExpression(
      EscriptGrammar::EscriptParser::FunctionExpressionContext* ctx ) override;
  antlrcpp::Any visitGotoStatement(
      EscriptGrammar::EscriptParser::GotoStatementContext* ctx ) override;
  antlrcpp::Any visitIfStatement( EscriptGrammar::EscriptParser::IfStatementContext* ctx ) override;
  antlrcpp::Any visitIncludeDeclaration(
      EscriptGrammar::EscriptParser::IncludeDeclarationContext* ctx ) override;
  antlrcpp::Any visitIndexBinding(
      EscriptGrammar::EscriptParser::IndexBindingContext* ctx ) override;
  antlrcpp::Any visitIndexBindingList(
      EscriptGrammar::EscriptParser::IndexBindingListContext* ctx ) override;
  antlrcpp::Any visitIndexList( EscriptGrammar::EscriptParser::IndexListContext* ctx ) override;
  antlrcpp::Any visitIntegerLiteral(
      EscriptGrammar::EscriptParser::IntegerLiteralContext* ctx ) override;
  antlrcpp::Any visitInterpolatedString(
      EscriptGrammar::EscriptParser::InterpolatedStringContext* ctx ) override;
  antlrcpp::Any visitInterpolatedStringPart(
      EscriptGrammar::EscriptParser::InterpolatedStringPartContext* ctx ) override;
  antlrcpp::Any visitLiteral( EscriptGrammar::EscriptParser::LiteralContext* ctx ) override;
  antlrcpp::Any visitModuleFunctionDeclaration(
      EscriptGrammar::EscriptParser::ModuleFunctionDeclarationContext* ctx ) override;
  antlrcpp::Any visitModuleFunctionParameter(
      EscriptGrammar::EscriptParser::ModuleFunctionParameterContext* ctx ) override;
  antlrcpp::Any visitModuleFunctionParameterList(
      EscriptGrammar::EscriptParser::ModuleFunctionParameterListContext* ctx ) override;
  antlrcpp::Any visitModuleUnit( EscriptGrammar::EscriptParser::ModuleUnitContext* ctx ) override;
  antlrcpp::Any visitParExpression(
      EscriptGrammar::EscriptParser::ParExpressionContext* ctx ) override;
  antlrcpp::Any visitPrimary( EscriptGrammar::EscriptParser::PrimaryContext* ctx ) override;
  antlrcpp::Any visitProgramDeclaration(
      EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx ) override;
  antlrcpp::Any visitProgramParameter(
      EscriptGrammar::EscriptParser::ProgramParameterContext* ctx ) override;
  antlrcpp::Any visitProgramParameterList(
      EscriptGrammar::EscriptParser::ProgramParameterListContext* ctx ) override;
  antlrcpp::Any visitProgramParameters(
      EscriptGrammar::EscriptParser::ProgramParametersContext* ctx ) override;
  antlrcpp::Any visitRepeatStatement(
      EscriptGrammar::EscriptParser::RepeatStatementContext* ctx ) override;
  antlrcpp::Any visitReturnStatement(
      EscriptGrammar::EscriptParser::ReturnStatementContext* ctx ) override;
  antlrcpp::Any visitScopedFunctionCall(
      EscriptGrammar::EscriptParser::ScopedFunctionCallContext* ctx ) override;
  antlrcpp::Any visitScopedIdentifier(
      EscriptGrammar::EscriptParser::ScopedIdentifierContext* ctx ) override;
  antlrcpp::Any visitSequenceBinding(
      EscriptGrammar::EscriptParser::SequenceBindingContext* ctx ) override;
  antlrcpp::Any visitSequenceBindingList(
      EscriptGrammar::EscriptParser::SequenceBindingListContext* ctx ) override;
  antlrcpp::Any visitStatement( EscriptGrammar::EscriptParser::StatementContext* ctx ) override;
  antlrcpp::Any visitStringIdentifier(
      EscriptGrammar::EscriptParser::StringIdentifierContext* ctx ) override;
  antlrcpp::Any visitStructInitializer(
      EscriptGrammar::EscriptParser::StructInitializerContext* ctx ) override;
  antlrcpp::Any visitStructInitializerExpression(
      EscriptGrammar::EscriptParser::StructInitializerExpressionContext* ctx ) override;
  antlrcpp::Any visitStructInitializerExpressionList(
      EscriptGrammar::EscriptParser::StructInitializerExpressionListContext* ctx ) override;
  antlrcpp::Any visitSwitchBlockStatementGroup(
      EscriptGrammar::EscriptParser::SwitchBlockStatementGroupContext* ctx ) override;
  antlrcpp::Any visitSwitchLabel( EscriptGrammar::EscriptParser::SwitchLabelContext* ctx ) override;
  antlrcpp::Any visitUseDeclaration(
      EscriptGrammar::EscriptParser::UseDeclarationContext* ctx ) override;
  antlrcpp::Any visitVariableDeclaration(
      EscriptGrammar::EscriptParser::VariableDeclarationContext* ctx ) override;
  antlrcpp::Any visitVariableDeclarationList(
      EscriptGrammar::EscriptParser::VariableDeclarationListContext* ctx ) override;
  antlrcpp::Any visitVarStatement(
      EscriptGrammar::EscriptParser::VarStatementContext* ctx ) override;
  antlrcpp::Any visitWhileStatement(
      EscriptGrammar::EscriptParser::WhileStatementContext* ctx ) override;

  antlrcpp::Any process_compilation_unit( SourceFile&, std::optional<Range> format_range );
  antlrcpp::Any process_module_unit( SourceFile&, std::optional<Range> format_range );

private:
  const SourceFileIdentifier& source_file_identifier;
  PrettifyLineBuilder linebuilder;
  size_t _currindent = 0;
  size_t _currentgroup = 0;
  bool _suppressnewline = false;
  FmtToken::Scope _currentscope = FmtToken::Scope::NONE;
  // Profile& profile;
  SourceFileLoader& source_loader;
  Report& report;
  void addToken( std::string&& text, const Position& pos, int style, size_t token_type,
                 FmtContext context = FmtContext::NONE );
  void addToken( std::string&& text, antlr4::tree::TerminalNode* terminal, int style,
                 FmtContext context = FmtContext::NONE );
  void addToken( std::string&& text, antlr4::Token* token, int style,
                 FmtContext context = FmtContext::NONE );
  void preprocess( SourceFile& sf, std::optional<Range> format_range );
  std::vector<FmtToken> collectComments( SourceFile& sf );
  std::vector<std::string> load_raw_file();

  antlrcpp::Any expression_suffix( EscriptGrammar::EscriptParser::ExpressionContext*,
                                   EscriptGrammar::EscriptParser::ExpressionSuffixContext* );

  antlrcpp::Any make_statement_label( EscriptGrammar::EscriptParser::StatementLabelContext* );
  antlrcpp::Any make_identifier( antlr4::tree::TerminalNode* );
  antlrcpp::Any make_string_literal( antlr4::tree::TerminalNode* );
  antlrcpp::Any make_bool_literal( antlr4::tree::TerminalNode* );
  antlrcpp::Any make_integer_literal( antlr4::tree::TerminalNode* );
  antlrcpp::Any make_float_literal( antlr4::tree::TerminalNode* );
};
}  // namespace Pol::Bscript::Compiler
