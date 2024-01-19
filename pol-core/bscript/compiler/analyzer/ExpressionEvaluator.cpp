#include "ExpressionEvaluator.h"

#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/ElementAccess.h"
#include "bscript/compiler/ast/ElementIndexes.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/MemberAccess.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/file/SourceFile.h"
#include "bscript/executor.h"
#include "bscript/impstr.h"

namespace Pol::Bscript::Compiler
{

ExpressionEvaluator::ExpressionEvaluator()
    : _profile(),
      _consoleReporter( false, false ),
      _report( _consoleReporter ),
      _ident( 0, "<eval>" ),
      _compiler_workspace( _report, _cache, _cache, true, _profile ),
      _loader(),
      _cache( _loader, _profile ),
      _builder_workspace( _compiler_workspace, _cache, _cache, true, _profile, _report ),
      _expression_builder( _ident, _builder_workspace )
{
}

BObjectRef ExpressionEvaluator::evaluate( Executor* exec, EScriptProgram* script,
                                          std::string expression )
{
  SourceFile source_file( "<eval>", expression, _profile );

  _report.reset();
  auto unit = source_file.get_evaluate_unit( _report );
  if ( unit == nullptr || _report.error_count() || !unit->expression() )
  {
    throw std::runtime_error( "Invalid expression" );
  }

  EvaluationVisitor visitor( exec, script );

  auto expression_node = _expression_builder.expression( unit->expression() );
  expression_node->accept( visitor );

  return visitor.result();
}

EvaluationVisitor::EvaluationVisitor( Executor* exec, EScriptProgram* script )
    : _exec( exec ), _script( script )
{
}

void EvaluationVisitor::visit_identifier( Identifier& identifier )
{
  visit_children( identifier );
  BObjectRefVec::const_iterator itr = _exec->Globals2->begin(), end = _exec->Globals2->end();
  BObjectRef result;

  if ( !identifier.scoped_name.scope.empty() )
    throw_invalid_expression();

  auto name = identifier.name();

  unsigned block = _script->dbg_ins_blocks[_exec->PC];
  size_t left = _exec->Locals2->size();

  while ( left )
  {
    while ( left <= _script->blocks[block].parentvariables )
    {
      block = _script->blocks[block].parentblockidx;
    }
    size_t varidx = left - 1 - _script->blocks[block].parentvariables;
    if ( _script->blocks[block].localvarnames[varidx] == name )
    {
      stack.push( ( *_exec->Locals2 )[left - 1] );
      return;
    }
    --left;
  }

  // Then check globals
  for ( unsigned idx = 0; itr != end; ++itr, ++idx )
  {
    if ( _script->globalvarnames.size() > idx && _script->globalvarnames[idx] == name )
    {
      stack.push( ( *_exec->Globals2 )[idx] );
      return;
    }
  }

  throw std::runtime_error( "Unknown variable " + name );
}

void EvaluationVisitor::visit_float_value( FloatValue& node )
{
  stack.push( BObjectRef( new Double( node.value ) ) );
}

void EvaluationVisitor::visit_string_value( StringValue& node )
{
  stack.push( BObjectRef( new String( node.value ) ) );
}

void EvaluationVisitor::visit_integer_value( IntegerValue& node )
{
  stack.push( BObjectRef( new BLong( node.value ) ) );
}

void EvaluationVisitor::visit_boolean_value( BooleanValue& node )
{
  stack.push( BObjectRef( new BBoolean( node.value ) ) );
}

void EvaluationVisitor::visit_uninitialized_value( UninitializedValue& )
{
  stack.push( BObjectRef( UninitObject::create() ) );
}

// Operators
void EvaluationVisitor::visit_member_access( MemberAccess& member_access )
{
  visit_children( member_access );

  if ( member_access.known_member )
  {
    BObjectRef& leftref = stack.top();

    BObject& left = *leftref;
    leftref = left->get_member_id( member_access.known_member->id );
  }
  else
  {
    BObjectRef& leftref = stack.top();

    BObject& left = *leftref;
    leftref = left->get_member( member_access.name.c_str() );
  }
}

void EvaluationVisitor::visit_element_access( ElementAccess& acc )
{
  visit_children( acc );
  auto indices_count = static_cast<unsigned>( acc.indexes().children.size() );
  if ( indices_count == 1 )
  {
    BObjectRef rightref = stack.top();
    stack.pop();
    BObjectRef& leftref = stack.top();

    leftref = ( *leftref )->OperSubscript( *rightref );
  }
  else
  {
    std::stack<BObjectRef> indices;
    for ( size_t i = 0; i < indices_count; ++i )
    {
      indices.push( stack.top() );
      stack.pop();
    }

    BObjectRef& leftref = stack.top();
    leftref = ( *leftref )->OperMultiSubscript( indices );
  }
}

void EvaluationVisitor::visit_argument( Argument& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_array_initializer( ArrayInitializer& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_binary_operator( BinaryOperator& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_dictionary_entry( DictionaryEntry& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_element_assignment( ElementAssignment& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_elvis_operator( ElvisOperator& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_error_initializer( ErrorInitializer& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_member_assignment( MemberAssignment& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_member_assignment_by_operator( MemberAssignmentByOperator& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_method_call( MethodCall& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_method_call_argument_list( MethodCallArgumentList& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_interpolate_string( InterpolateString& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_format_expression( FormatExpression& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_struct_initializer( StructInitializer& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_struct_member_initializer( StructMemberInitializer& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_unary_operator( UnaryOperator& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_function_call( FunctionCall& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_function_reference( FunctionReference& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_variable_assignment_statement( VariableAssignmentStatement& )
{
  throw_invalid_expression();
}

void EvaluationVisitor::visit_conditional_operator( ConditionalOperator& )
{
  throw_invalid_expression();
}

BObjectRef& EvaluationVisitor::result()
{
  if ( stack.empty() )
  {
    throw std::runtime_error( "Error evaluating expression (empty result?)" );
  }

  return stack.top();
}

void EvaluationVisitor::throw_invalid_expression() const
{
  throw std::runtime_error( "Unsupported expression" );
}

}  // namespace Pol::Bscript::Compiler
