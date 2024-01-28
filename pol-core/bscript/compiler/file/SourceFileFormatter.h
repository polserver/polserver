#ifndef VSCODEESCRIPT_SOURCEFILEFORMATTER_H
#define VSCODEESCRIPT_SOURCEFILEFORMATTER_H

#include <EscriptGrammar/EscriptParserBaseVisitor.h>

#include <memory>
#include <stack>

namespace Pol::Bscript::Compiler
{
class Profile;
class SourceFile;
class Report;
class SourceFileIdentifier;

class Terminal
{
public:
  Terminal( antlr4::tree::TerminalNode* node, const std::string& value )
      : node( node ), value( value )
  {
  }
  Terminal( antlr4::tree::TerminalNode* node ) : node( node ), value( "" ) {}
  static Terminal from( antlr4::tree::ParseTree* tree );

  // needed for "get comments left/right of node"
  antlr4::tree::TerminalNode* node;

  // what's printed if not empty, otherwise print `node`'s text value (eg. identifiers)
  std::string value;
};

struct hardline_t
{
};
struct space_t
{
};
struct indent_t
{
};
struct unindent_t
{
};

class SourceFileFormatter : public EscriptGrammar::EscriptParserBaseVisitor
{
public:
  SourceFileFormatter( const Pol::Bscript::Compiler::SourceFileIdentifier&,
                       std::shared_ptr<Pol::Bscript::Compiler::SourceFile>, Report& );

private:
  const Pol::Bscript::Compiler::SourceFileIdentifier& ident;
  std::shared_ptr<Pol::Bscript::Compiler::SourceFile> sf;
  Report& report;

  std::string formatted = "";
  std::string current_line = "";
  bool extra_lines_allowed = false;
  size_t last_printed_line_number = 0;

  antlr4::tree::TerminalNode* last_printed_node = nullptr;
  antlr4::tree::TerminalNode* last_node = nullptr;

  std::stack<size_t> indent_levels = {};
  int current_indent_level() const;

  template <typename T, typename... Types>
  void print( T var1, Types... var2 );
  void print();

  void out();
  void out( const hardline_t&, bool include_right_comments = true );
  void out( const space_t& );
  void out( const indent_t& );
  void out( const unindent_t& );
  void out( const Terminal& );
  void out( const std::string& a );

public:
  std::string format();

  //   antlrcpp::Any visitCompilationUnit(
  //       EscriptGrammar::EscriptParser::CompilationUnitContext* ctx );

  //   antlrcpp::Any visitModuleUnit( EscriptGrammar::EscriptParser::ModuleUnitContext* ctx );

  //   antlrcpp::Any visitModuleDeclarationStatement(
  //       EscriptGrammar::EscriptParser::ModuleDeclarationStatementContext* ctx );

  //   antlrcpp::Any visitModuleFunctionDeclaration(
  //       EscriptGrammar::EscriptParser::ModuleFunctionDeclarationContext* ctx );

  //   antlrcpp::Any visitModuleFunctionParameterList(
  //       EscriptGrammar::EscriptParser::ModuleFunctionParameterListContext* ctx );

  //   antlrcpp::Any visitModuleFunctionParameter(
  //       EscriptGrammar::EscriptParser::ModuleFunctionParameterContext* ctx );

  //   antlrcpp::Any visitTopLevelDeclaration(
  //       EscriptGrammar::EscriptParser::TopLevelDeclarationContext* ctx );

  //   antlrcpp::Any visitFunctionDeclaration(
  //       EscriptGrammar::EscriptParser::FunctionDeclarationContext* ctx );

  //   antlrcpp::Any visitStringIdentifier(
  //       EscriptGrammar::EscriptParser::StringIdentifierContext* ctx );

  antlrcpp::Any visitUseDeclaration( EscriptGrammar::EscriptParser::UseDeclarationContext* ctx );

  //   antlrcpp::Any visitIncludeDeclaration(
  //       EscriptGrammar::EscriptParser::IncludeDeclarationContext* ctx );

  antlrcpp::Any visitProgramDeclaration(
      EscriptGrammar::EscriptParser::ProgramDeclarationContext* ctx );

  antlrcpp::Any visitStatement( EscriptGrammar::EscriptParser::StatementContext* ctx );

  //   antlrcpp::Any visitStatementLabel(
  //       EscriptGrammar::EscriptParser::StatementLabelContext* ctx );

  antlrcpp::Any visitIfStatement( EscriptGrammar::EscriptParser::IfStatementContext* ctx );

  //   antlrcpp::Any visitGotoStatement( EscriptGrammar::EscriptParser::GotoStatementContext* ctx );

  //   antlrcpp::Any visitReturnStatement(
  //       EscriptGrammar::EscriptParser::ReturnStatementContext* ctx );

  //   antlrcpp::Any visitConstStatement(
  //       EscriptGrammar::EscriptParser::ConstStatementContext* ctx );

  //   antlrcpp::Any visitVarStatement( EscriptGrammar::EscriptParser::VarStatementContext* ctx );

  //   antlrcpp::Any visitDoStatement( EscriptGrammar::EscriptParser::DoStatementContext* ctx );

  //   antlrcpp::Any visitWhileStatement(
  //       EscriptGrammar::EscriptParser::WhileStatementContext* ctx );

  //   antlrcpp::Any visitExitStatement( EscriptGrammar::EscriptParser::ExitStatementContext* ctx );

  //   antlrcpp::Any visitBreakStatement(
  //       EscriptGrammar::EscriptParser::BreakStatementContext* ctx );

  //   antlrcpp::Any visitContinueStatement(
  //       EscriptGrammar::EscriptParser::ContinueStatementContext* ctx );

  //   antlrcpp::Any visitForStatement( EscriptGrammar::EscriptParser::ForStatementContext* ctx );

  //   antlrcpp::Any visitForeachIterableExpression(
  //       EscriptGrammar::EscriptParser::ForeachIterableExpressionContext* ctx );

  //   antlrcpp::Any visitForeachStatement(
  //       EscriptGrammar::EscriptParser::ForeachStatementContext* ctx );

  //   antlrcpp::Any visitRepeatStatement(
  //       EscriptGrammar::EscriptParser::RepeatStatementContext* ctx );

  //   antlrcpp::Any visitCaseStatement( EscriptGrammar::EscriptParser::CaseStatementContext* ctx );

  //   antlrcpp::Any visitEnumStatement( EscriptGrammar::EscriptParser::EnumStatementContext* ctx );

  //   antlrcpp::Any visitBlock( EscriptGrammar::EscriptParser::BlockContext* ctx );

  //   antlrcpp::Any visitVariableDeclarationInitializer(
  //       EscriptGrammar::EscriptParser::VariableDeclarationInitializerContext* ctx );

  //   antlrcpp::Any visitEnumList( EscriptGrammar::EscriptParser::EnumListContext* ctx );

  //   antlrcpp::Any visitEnumListEntry( EscriptGrammar::EscriptParser::EnumListEntryContext* ctx );

  //   antlrcpp::Any visitSwitchBlockStatementGroup(
  //       EscriptGrammar::EscriptParser::SwitchBlockStatementGroupContext* ctx );

  //   antlrcpp::Any visitSwitchLabel( EscriptGrammar::EscriptParser::SwitchLabelContext* ctx );

  //   antlrcpp::Any visitForGroup( EscriptGrammar::EscriptParser::ForGroupContext* ctx );

  //   antlrcpp::Any visitBasicForStatement(
  //       EscriptGrammar::EscriptParser::BasicForStatementContext* ctx );

  //   antlrcpp::Any visitCstyleForStatement(
  //       EscriptGrammar::EscriptParser::CstyleForStatementContext* ctx );

  //   antlrcpp::Any visitIdentifierList(
  //       EscriptGrammar::EscriptParser::IdentifierListContext* ctx );

  //   antlrcpp::Any visitVariableDeclarationList(
  //       EscriptGrammar::EscriptParser::VariableDeclarationListContext* ctx );

  //   antlrcpp::Any visitConstantDeclaration(
  //       EscriptGrammar::EscriptParser::ConstantDeclarationContext* ctx );

  //   antlrcpp::Any visitVariableDeclaration(
  //       EscriptGrammar::EscriptParser::VariableDeclarationContext* ctx );

  antlrcpp::Any visitProgramParameters(
      EscriptGrammar::EscriptParser::ProgramParametersContext* ctx );

  antlrcpp::Any visitProgramParameterList(
      EscriptGrammar::EscriptParser::ProgramParameterListContext* ctx );

  antlrcpp::Any visitProgramParameter(
      EscriptGrammar::EscriptParser::ProgramParameterContext* ctx );

  //   antlrcpp::Any visitFunctionParameters(
  //       EscriptGrammar::EscriptParser::FunctionParametersContext* ctx );

  //   antlrcpp::Any visitFunctionParameterList(
  //       EscriptGrammar::EscriptParser::FunctionParameterListContext* ctx );

  //   antlrcpp::Any visitFunctionParameter(
  //       EscriptGrammar::EscriptParser::FunctionParameterContext* ctx );

  antlrcpp::Any visitScopedFunctionCall(
      EscriptGrammar::EscriptParser::ScopedFunctionCallContext* ctx );

  //   antlrcpp::Any visitFunctionReference(
  //       EscriptGrammar::EscriptParser::FunctionReferenceContext* ctx );

  antlrcpp::Any visitExpression( EscriptGrammar::EscriptParser::ExpressionContext* ctx );

  antlrcpp::Any visitPrimary( EscriptGrammar::EscriptParser::PrimaryContext* ctx );

  //   antlrcpp::Any visitExplicitArrayInitializer(
  //       EscriptGrammar::EscriptParser::ExplicitArrayInitializerContext* ctx );

  //   antlrcpp::Any visitExplicitStructInitializer(
  //       EscriptGrammar::EscriptParser::ExplicitStructInitializerContext* ctx );

  antlrcpp::Any visitExplicitDictInitializer(
      EscriptGrammar::EscriptParser::ExplicitDictInitializerContext* ctx );

  //   antlrcpp::Any visitExplicitErrorInitializer(
  //       EscriptGrammar::EscriptParser::ExplicitErrorInitializerContext* ctx );

  //   antlrcpp::Any visitBareArrayInitializer(
  //       EscriptGrammar::EscriptParser::BareArrayInitializerContext* ctx );

  antlrcpp::Any visitParExpression( EscriptGrammar::EscriptParser::ParExpressionContext* ctx );

  antlrcpp::Any visitExpressionList( EscriptGrammar::EscriptParser::ExpressionListContext* ctx );

  //   antlrcpp::Any visitExpressionSuffix(
  //       EscriptGrammar::EscriptParser::ExpressionSuffixContext* ctx );

  //   antlrcpp::Any visitIndexingSuffix(
  //       EscriptGrammar::EscriptParser::IndexingSuffixContext* ctx );

  //   antlrcpp::Any visitNavigationSuffix(
  //       EscriptGrammar::EscriptParser::NavigationSuffixContext* ctx );

  //   antlrcpp::Any visitMethodCallSuffix(
  //       EscriptGrammar::EscriptParser::MethodCallSuffixContext* ctx );

  antlrcpp::Any visitFunctionCall( EscriptGrammar::EscriptParser::FunctionCallContext* ctx );

  //   antlrcpp::Any visitStructInitializerExpression(
  //       EscriptGrammar::EscriptParser::StructInitializerExpressionContext* ctx );

  //   antlrcpp::Any visitStructInitializerExpressionList(
  //       EscriptGrammar::EscriptParser::StructInitializerExpressionListContext* ctx );

  //   antlrcpp::Any visitStructInitializer(
  //       EscriptGrammar::EscriptParser::StructInitializerContext* ctx );

  antlrcpp::Any visitDictInitializerExpression(
      EscriptGrammar::EscriptParser::DictInitializerExpressionContext* ctx );

  antlrcpp::Any visitDictInitializerExpressionList(
      EscriptGrammar::EscriptParser::DictInitializerExpressionListContext* ctx );

  antlrcpp::Any visitDictInitializer( EscriptGrammar::EscriptParser::DictInitializerContext* ctx );

  //   antlrcpp::Any visitArrayInitializer(
  //       EscriptGrammar::EscriptParser::ArrayInitializerContext* ctx );

  // antlrcpp::Any visitLiteral( EscriptGrammar::EscriptParser::LiteralContext* ctx );

  //   antlrcpp::Any visitInterpolatedString(
  //       EscriptGrammar::EscriptParser::InterpolatedStringContext* ctx );

  //   antlrcpp::Any visitInterpolatedStringPart(
  //       EscriptGrammar::EscriptParser::InterpolatedStringPartContext* ctx );

  // antlrcpp::Any visitIntegerLiteral(
  //     EscriptGrammar::EscriptParser::IntegerLiteralContext* ctx );

  //   antlrcpp::Any visitFloatLiteral( EscriptGrammar::EscriptParser::FloatLiteralContext* ctx );

  //   antlrcpp::Any visitBoolLiteral( EscriptGrammar::EscriptParser::BoolLiteralContext* ctx );

  ///////////

  // void expression( EscriptGrammar::EscriptParser::ExpressionContext* );
  // void function_call( EscriptGrammar::EscriptParser::FunctionCallContext* );
};
}  // namespace Pol::Bscript::Compiler
#endif