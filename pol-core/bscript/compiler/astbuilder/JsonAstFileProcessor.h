#ifndef POLSERVER_JSONASTFILEPROCESSOR_H
#define POLSERVER_JSONASTFILEPROCESSOR_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <memory>
#include <optional>
#include <string>

#include <picojson/picojson.h>


namespace Pol::Bscript::Compiler
{
class Profile;
class Report;
class SourceFileIdentifier;
class SourceFile;

class JsonAstFileProcessor : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  JsonAstFileProcessor( const SourceFileIdentifier&, Profile&, Report& );

public:

  // antlrcpp::Any visitChildren( antlr4::tree::ParseTree* node ) override;
  antlrcpp::Any defaultResult() override;
  antlrcpp::Any aggregateResult( antlrcpp::Any aggregate, antlrcpp::Any nextResult ) override;

  // antlrcpp::Any visitArrayInitializer(EscriptGrammar::EscriptParser::ArrayInitializerContext *ctx) override;
  antlrcpp::Any visitBareArrayInitializer(EscriptGrammar::EscriptParser::BareArrayInitializerContext *ctx) override;
  antlrcpp::Any visitBasicForStatement(EscriptGrammar::EscriptParser::BasicForStatementContext *ctx) override;
  // antlrcpp::Any visitBlock(EscriptGrammar::EscriptParser::BlockContext *ctx) override;
  antlrcpp::Any visitBoolLiteral(EscriptGrammar::EscriptParser::BoolLiteralContext *ctx) override;
  antlrcpp::Any visitBreakStatement(EscriptGrammar::EscriptParser::BreakStatementContext *ctx) override;
  antlrcpp::Any visitCaseStatement(EscriptGrammar::EscriptParser::CaseStatementContext *ctx) override;
  antlrcpp::Any visitCompilationUnit(EscriptGrammar::EscriptParser::CompilationUnitContext *ctx) override;
  antlrcpp::Any visitConstStatement(EscriptGrammar::EscriptParser::ConstStatementContext *ctx) override;
  antlrcpp::Any visitContinueStatement(EscriptGrammar::EscriptParser::ContinueStatementContext *ctx) override;
  antlrcpp::Any visitCstyleForStatement(EscriptGrammar::EscriptParser::CstyleForStatementContext *ctx) override;
  // antlrcpp::Any visitDictInitializer(EscriptGrammar::EscriptParser::DictInitializerContext *ctx) override;
  antlrcpp::Any visitDictInitializerExpression(EscriptGrammar::EscriptParser::DictInitializerExpressionContext *ctx) override;
  // antlrcpp::Any visitDictInitializerExpressionList(EscriptGrammar::EscriptParser::DictInitializerExpressionListContext *ctx) override;
  antlrcpp::Any visitDoStatement(EscriptGrammar::EscriptParser::DoStatementContext *ctx) override;
  // antlrcpp::Any visitEnumList(EscriptGrammar::EscriptParser::EnumListContext *ctx) override;
  antlrcpp::Any visitEnumListEntry(EscriptGrammar::EscriptParser::EnumListEntryContext *ctx) override;
  antlrcpp::Any visitEnumStatement(EscriptGrammar::EscriptParser::EnumStatementContext *ctx) override;
  // antlrcpp::Any visitEvaluateUnit(EscriptGrammar::EscriptParser::EvaluateUnitContext *ctx) override;
  antlrcpp::Any visitExitStatement(EscriptGrammar::EscriptParser::ExitStatementContext *ctx) override;
  antlrcpp::Any visitExplicitArrayInitializer(EscriptGrammar::EscriptParser::ExplicitArrayInitializerContext *ctx) override;
  antlrcpp::Any visitExplicitDictInitializer(EscriptGrammar::EscriptParser::ExplicitDictInitializerContext *ctx) override;
  antlrcpp::Any visitExplicitErrorInitializer(EscriptGrammar::EscriptParser::ExplicitErrorInitializerContext *ctx) override;
  antlrcpp::Any visitExplicitStructInitializer(EscriptGrammar::EscriptParser::ExplicitStructInitializerContext *ctx) override;
  antlrcpp::Any visitExpression(EscriptGrammar::EscriptParser::ExpressionContext *ctx) override;
  // antlrcpp::Any visitExpressionList(EscriptGrammar::EscriptParser::ExpressionListContext *ctx) override;
  // antlrcpp::Any visitExpressionSuffix(EscriptGrammar::EscriptParser::ExpressionSuffixContext *ctx) override;
  antlrcpp::Any visitFloatLiteral(EscriptGrammar::EscriptParser::FloatLiteralContext *ctx) override;
  antlrcpp::Any visitForeachIterableExpression(EscriptGrammar::EscriptParser::ForeachIterableExpressionContext *ctx) override;
  antlrcpp::Any visitForeachStatement(EscriptGrammar::EscriptParser::ForeachStatementContext *ctx) override;
  // antlrcpp::Any visitForGroup(EscriptGrammar::EscriptParser::ForGroupContext *ctx) override;
  antlrcpp::Any visitForStatement(EscriptGrammar::EscriptParser::ForStatementContext *ctx) override;
  antlrcpp::Any visitFunctionCall(EscriptGrammar::EscriptParser::FunctionCallContext *ctx) override;
  antlrcpp::Any visitFunctionDeclaration(EscriptGrammar::EscriptParser::FunctionDeclarationContext *ctx) override;
  antlrcpp::Any visitFunctionParameter(EscriptGrammar::EscriptParser::FunctionParameterContext *ctx) override;
  // antlrcpp::Any visitFunctionParameterList(EscriptGrammar::EscriptParser::FunctionParameterListContext *ctx) override;
  // antlrcpp::Any visitFunctionParameters(EscriptGrammar::EscriptParser::FunctionParametersContext *ctx) override;
  antlrcpp::Any visitFunctionReference(EscriptGrammar::EscriptParser::FunctionReferenceContext *ctx) override;
  antlrcpp::Any visitGotoStatement(EscriptGrammar::EscriptParser::GotoStatementContext *ctx) override;
  // antlrcpp::Any visitIdentifierList(EscriptGrammar::EscriptParser::IdentifierListContext *ctx) override;
  antlrcpp::Any visitIfStatement(EscriptGrammar::EscriptParser::IfStatementContext *ctx) override;
  antlrcpp::Any visitIncludeDeclaration(EscriptGrammar::EscriptParser::IncludeDeclarationContext *ctx) override;
  // antlrcpp::Any visitIndexingSuffix(EscriptGrammar::EscriptParser::IndexingSuffixContext *ctx) override;
  antlrcpp::Any visitIntegerLiteral(EscriptGrammar::EscriptParser::IntegerLiteralContext *ctx) override;
  antlrcpp::Any visitInterpolatedString(EscriptGrammar::EscriptParser::InterpolatedStringContext *ctx) override;
  antlrcpp::Any visitInterpolatedStringPart(EscriptGrammar::EscriptParser::InterpolatedStringPartContext *ctx) override;
  antlrcpp::Any visitLiteral(EscriptGrammar::EscriptParser::LiteralContext *ctx) override;
  // antlrcpp::Any visitMethodCallSuffix(EscriptGrammar::EscriptParser::MethodCallSuffixContext *ctx) override;
  // antlrcpp::Any visitModuleDeclarationStatement(EscriptGrammar::EscriptParser::ModuleDeclarationStatementContext *ctx) override;
  antlrcpp::Any visitModuleFunctionDeclaration(EscriptGrammar::EscriptParser::ModuleFunctionDeclarationContext *ctx) override;
  antlrcpp::Any visitModuleFunctionParameter(EscriptGrammar::EscriptParser::ModuleFunctionParameterContext *ctx) override;
  // antlrcpp::Any visitModuleFunctionParameterList(EscriptGrammar::EscriptParser::ModuleFunctionParameterListContext *ctx) override;
  antlrcpp::Any visitModuleUnit(EscriptGrammar::EscriptParser::ModuleUnitContext *ctx) override;
  // antlrcpp::Any visitNavigationSuffix(EscriptGrammar::EscriptParser::NavigationSuffixContext *ctx) override;
  // antlrcpp::Any visitParExpression(EscriptGrammar::EscriptParser::ParExpressionContext *ctx) override;
  antlrcpp::Any visitPrimary(EscriptGrammar::EscriptParser::PrimaryContext *ctx) override;
  antlrcpp::Any visitProgramDeclaration(EscriptGrammar::EscriptParser::ProgramDeclarationContext *ctx) override;
  antlrcpp::Any visitProgramParameter(EscriptGrammar::EscriptParser::ProgramParameterContext *ctx) override;
  // antlrcpp::Any visitProgramParameterList(EscriptGrammar::EscriptParser::ProgramParameterListContext *ctx) override;
  // antlrcpp::Any visitProgramParameters(EscriptGrammar::EscriptParser::ProgramParametersContext *ctx) override;
  antlrcpp::Any visitRepeatStatement(EscriptGrammar::EscriptParser::RepeatStatementContext *ctx) override;
  antlrcpp::Any visitReturnStatement(EscriptGrammar::EscriptParser::ReturnStatementContext *ctx) override;
  antlrcpp::Any visitScopedFunctionCall(EscriptGrammar::EscriptParser::ScopedFunctionCallContext *ctx) override;
  antlrcpp::Any visitStatement(EscriptGrammar::EscriptParser::StatementContext *ctx) override;
  // antlrcpp::Any visitStatementLabel(EscriptGrammar::EscriptParser::StatementLabelContext *ctx) override;
  antlrcpp::Any visitStringIdentifier(EscriptGrammar::EscriptParser::StringIdentifierContext *ctx) override;
  // antlrcpp::Any visitStructInitializer(EscriptGrammar::EscriptParser::StructInitializerContext *ctx) override;
  antlrcpp::Any visitStructInitializerExpression(EscriptGrammar::EscriptParser::StructInitializerExpressionContext *ctx) override;
  // antlrcpp::Any visitStructInitializerExpressionList(EscriptGrammar::EscriptParser::StructInitializerExpressionListContext *ctx) override;
  antlrcpp::Any visitSwitchBlockStatementGroup(EscriptGrammar::EscriptParser::SwitchBlockStatementGroupContext *ctx) override;
  antlrcpp::Any visitSwitchLabel(EscriptGrammar::EscriptParser::SwitchLabelContext *ctx) override;
  // antlrcpp::Any visitTopLevelDeclaration(EscriptGrammar::EscriptParser::TopLevelDeclarationContext *ctx) override;
  antlrcpp::Any visitUseDeclaration(EscriptGrammar::EscriptParser::UseDeclarationContext *ctx) override;
  antlrcpp::Any visitVariableDeclaration(EscriptGrammar::EscriptParser::VariableDeclarationContext *ctx) override;
  // antlrcpp::Any visitVariableDeclarationInitializer(EscriptGrammar::EscriptParser::VariableDeclarationInitializerContext *ctx) override;
  // antlrcpp::Any visitVariableDeclarationList(EscriptGrammar::EscriptParser::VariableDeclarationListContext *ctx) override;
  antlrcpp::Any visitVarStatement(EscriptGrammar::EscriptParser::VarStatementContext *ctx) override;
  antlrcpp::Any visitWhileStatement(EscriptGrammar::EscriptParser::WhileStatementContext *ctx) override;

  picojson::value process_compilation_unit( SourceFile& );
  picojson::value process_module_unit( SourceFile& );
  // SourceLocation location_for( antlr4::ParserRuleContext& ) const;
private:
  const SourceFileIdentifier& source_file_identifier;
  Profile& profile;
  Report& report;

  antlrcpp::Any expression_suffix( EscriptGrammar::EscriptParser::ExpressionContext*, EscriptGrammar::EscriptParser::ExpressionSuffixContext* );

  antlrcpp::Any make_statement_label( EscriptGrammar::EscriptParser::StatementLabelContext* );
  antlrcpp::Any make_identifier(antlr4::tree::TerminalNode *);
  antlrcpp::Any make_string_literal(antlr4::tree::TerminalNode *);
  antlrcpp::Any make_string_literal(antlr4::tree::TerminalNode *, const std::string& text);
  antlrcpp::Any make_bool_literal(antlr4::tree::TerminalNode *);
  antlrcpp::Any make_integer_literal(antlr4::tree::TerminalNode *);
  antlrcpp::Any make_float_literal(antlr4::tree::TerminalNode *);

};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_JSONASTFILEPROCESSOR_H
