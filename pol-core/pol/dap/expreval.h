#ifndef DAP_EXPREVAL_H
#define DAP_EXPREVAL_H

#include "../../bscript/compiler/Profile.h"
#include "../../bscript/compiler/Report.h"
#include "../../bscript/compiler/ast/NodeVisitor.h"
#include "../../bscript/compiler/astbuilder/BuilderWorkspace.h"
#include "../../bscript/compiler/astbuilder/ExpressionBuilder.h"
#include "../../bscript/compiler/file/SourceFileCache.h"
#include "../../bscript/compiler/file/SourceFileIdentifier.h"
#include "../../bscript/compiler/model/CompilerWorkspace.h"

#include <memory>
#include <stack>

namespace Pol
{
namespace Bscript
{
class BObjectRef;
class EScriptProgram;
namespace Compiler
{
class Identifier;
}
};  // namespace Bscript
namespace Core
{
class UOExecutor;
}  // namespace Core
namespace Network
{
namespace DAP
{

class EvaluationVisitor : public Bscript::Compiler::NodeVisitor
{
public:
  EvaluationVisitor( Core::UOExecutor* uoexec, Bscript::EScriptProgram* script )
      : _uoexec( uoexec ), _script( script )
  {
  }

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

  Bscript::BObjectRef& result();

private:
  Core::UOExecutor* _uoexec;
  Bscript::EScriptProgram* _script;
  std::stack<Bscript::BObjectRef> stack;

  void throw_invalid_expression() const;
};

class ExpressionEvaluator
{
public:
  ExpressionEvaluator()
      : _profile(),
        _report( false, false ),
        _ident( 0, "<eval>" ),
        _compiler_workspace( _report ),
        _cache( _profile ),
        _builder_workspace( _compiler_workspace, _cache, _cache, _profile, _report ),
        _expression_builder( _ident, _builder_workspace )
  {
  }
  // Throws on errors (parsing, evaluation, ...)
  Bscript::BObjectRef evaluate( Core::UOExecutor* uoexec, Bscript::EScriptProgram* script,
                                std::string expression );

private:
  Bscript::Compiler::Profile _profile;
  Bscript::Compiler::Report _report;

  Bscript::Compiler::SourceFileIdentifier _ident;
  Bscript::Compiler::CompilerWorkspace _compiler_workspace;
  Bscript::Compiler::SourceFileCache _cache;
  Bscript::Compiler::BuilderWorkspace _builder_workspace;
  Bscript::Compiler::ExpressionBuilder _expression_builder;
};

}  // namespace DAP
}  // namespace Network
}  // namespace Pol
#endif
