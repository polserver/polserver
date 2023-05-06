#ifndef POLSERVER_EXPREVAL_H
#define POLSERVER_EXPREVAL_H

#include "bscript/bobject.h"
#include "bscript/compiler/Profile.h"
#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/NodeVisitor.h"
#include "bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "bscript/compiler/astbuilder/ExpressionBuilder.h"
#include "bscript/compiler/file/SourceFileCache.h"
#include "bscript/compiler/file/SourceFileIdentifier.h"
#include "bscript/compiler/model/CompilerWorkspace.h"

#include <memory>
#include <stack>


namespace Pol::Bscript
{
class Executor;
class EScriptProgram;
}  // namespace Pol::Bscript

namespace Pol::Bscript::Compiler
{

class EvaluationVisitor : public Bscript::Compiler::NodeVisitor
{
public:
  EvaluationVisitor( Executor* exec, EScriptProgram* script );

  // Values
  void visit_element_access( Bscript::Compiler::ElementAccess& acc ) override;
  void visit_float_value( Bscript::Compiler::FloatValue& node ) override;
  void visit_identifier( Bscript::Compiler::Identifier& identifier ) override;
  void visit_integer_value( Bscript::Compiler::IntegerValue& node ) override;
  void visit_member_access( Bscript::Compiler::MemberAccess& member_access ) override;
  void visit_string_value( Bscript::Compiler::StringValue& node ) override;

  // Unsupported expression operations
  void visit_argument( Bscript::Compiler::Argument& ) override;
  void visit_array_initializer( Bscript::Compiler::ArrayInitializer& ) override;
  void visit_binary_operator( Bscript::Compiler::BinaryOperator& ) override;
  void visit_dictionary_entry( Bscript::Compiler::DictionaryEntry& ) override;
  void visit_element_assignment( Bscript::Compiler::ElementAssignment& ) override;
  void visit_elvis_operator( Bscript::Compiler::ElvisOperator& ) override;
  void visit_error_initializer( Bscript::Compiler::ErrorInitializer& ) override;
  void visit_member_assignment( Bscript::Compiler::MemberAssignment& ) override;
  void visit_member_assignment_by_operator(
      Bscript::Compiler::MemberAssignmentByOperator& ) override;
  void visit_method_call( Bscript::Compiler::MethodCall& ) override;
  void visit_method_call_argument_list( Bscript::Compiler::MethodCallArgumentList& ) override;
  void visit_interpolate_string( Bscript::Compiler::InterpolateString& ) override;
  void visit_format_expression( Bscript::Compiler::FormatExpression& ) override;
  void visit_struct_initializer( Bscript::Compiler::StructInitializer& ) override;
  void visit_struct_member_initializer( Bscript::Compiler::StructMemberInitializer& ) override;
  void visit_unary_operator( Bscript::Compiler::UnaryOperator& ) override;
  void visit_uninitialized_value( Bscript::Compiler::UninitializedValue& ) override;
  void visit_function_call( Bscript::Compiler::FunctionCall& ) override;
  void visit_function_reference( Bscript::Compiler::FunctionReference& ) override;
  void visit_variable_assignment_statement(
      Bscript::Compiler::VariableAssignmentStatement& ) override;
  void visit_conditional_operator( Bscript::Compiler::ConditionalOperator& ) override;

  BObjectRef& result();

private:
  Executor* _exec;
  Bscript::EScriptProgram* _script;
  std::stack<BObjectRef> stack;

  void throw_invalid_expression() const;
};

class ExpressionEvaluator
{
public:
  ExpressionEvaluator();

  // Throws on errors (parsing, evaluation, ...)
  Bscript::BObjectRef evaluate( Executor* uoexec, EScriptProgram* script,
                                std::string expression );

private:
  Profile _profile;
  Report _report;

  SourceFileIdentifier _ident;
  CompilerWorkspace _compiler_workspace;
  SourceFileCache _cache;
  BuilderWorkspace _builder_workspace;
  ExpressionBuilder _expression_builder;
};
}  // namespace Pol::Bscript::Compiler
#endif
