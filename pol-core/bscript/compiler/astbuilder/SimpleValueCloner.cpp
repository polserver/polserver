#include "SimpleValueCloner.h"

#include "bscript/compiler/Report.h"
#include "bscript/compiler/ast/Argument.h"
#include "bscript/compiler/ast/ArrayInitializer.h"
#include "bscript/compiler/ast/BooleanValue.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/DictionaryEntry.h"
#include "bscript/compiler/ast/DictionaryInitializer.h"
#include "bscript/compiler/ast/ErrorInitializer.h"
#include "bscript/compiler/ast/Expression.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/StructInitializer.h"
#include "bscript/compiler/ast/StructMemberInitializer.h"
#include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
SimpleValueCloner::SimpleValueCloner( Report& report, const SourceLocation& use_source_location )
    : report( report ), use_source_location( use_source_location )
{
}

std::unique_ptr<Node> SimpleValueCloner::clone( Node& node )
{
  node.accept( *this );
  return std::move( cloned_value );
}

void SimpleValueCloner::visit_array_initializer( ArrayInitializer& initializer )
{
  if ( initializer.children.empty() )
  {
    std::vector<std::unique_ptr<Expression>> empty;
    cloned_value = std::make_unique<ArrayInitializer>( use_source_location, std::move( empty ) );
  }
  else
  {
    report.error( initializer,
                  "A const array must be empty.\n"
                  "{}",
                  initializer );
  }
}

void SimpleValueCloner::visit_boolean_value( BooleanValue& bv )
{
  cloned_value = std::make_unique<BooleanValue>( use_source_location, bv.value );
}

void SimpleValueCloner::visit_dictionary_initializer( DictionaryInitializer& initializer )
{
  if ( initializer.children.empty() )
  {
    std::vector<std::unique_ptr<DictionaryEntry>> empty;
    cloned_value =
        std::make_unique<DictionaryInitializer>( use_source_location, std::move( empty ) );
  }
  else
  {
    report.error( initializer,
                  "A const dictionary must be empty.\n"
                  "{}",
                  initializer );
  }
}

void SimpleValueCloner::visit_error_initializer( ErrorInitializer& initializer )
{
  if ( initializer.children.empty() )
  {
    std::vector<std::string> no_names;
    std::vector<std::unique_ptr<Expression>> empty;
    cloned_value =
        std::make_unique<ErrorInitializer>( use_source_location, no_names, std::move( empty ) );
  }
  else
  {
    report.error( initializer,
                  "A const error must be empty.\n"
                  "{}",
                  initializer );
  }
}

void SimpleValueCloner::visit_float_value( FloatValue& fv )
{
  cloned_value = std::make_unique<FloatValue>( use_source_location, fv.value );
}

void SimpleValueCloner::visit_function_call( FunctionCall& fc )
{
  if ( fc.function_link->module_function_declaration() && fc.children.empty() )
  {
    std::vector<std::unique_ptr<Argument>> no_args;

    if ( fc.scoped_name )
    {
      auto new_fc = std::make_unique<FunctionCall>( use_source_location, fc.calling_scope,
                                                    *fc.scoped_name, std::move( no_args ) );
      new_fc->function_link->link_to( fc.function_link->function() );
      cloned_value = std::move( new_fc );
    }
    else
    {
      report.error( fc,
                    "Cannot clone function call because it does not have method name as callee." );
    }
  }
}

void SimpleValueCloner::visit_identifier( Identifier& ident )
{
  report.error( ident, "Cannot clone '{}' because it is not a constant.", ident.name );
}

void SimpleValueCloner::visit_integer_value( IntegerValue& iv )
{
  cloned_value = std::make_unique<IntegerValue>( use_source_location, iv.value );
}

void SimpleValueCloner::visit_string_value( StringValue& sv )
{
  cloned_value = std::make_unique<StringValue>( use_source_location, sv.value );
}

void SimpleValueCloner::visit_struct_initializer( StructInitializer& node )
{
  if ( node.children.empty() )
  {
    std::vector<std::unique_ptr<StructMemberInitializer>> empty;
    cloned_value = std::make_unique<StructInitializer>( use_source_location, std::move( empty ) );
  }
  else
  {
    report.error( node,
                  "A const struct must be empty.\n"
                  "{}",
                  node );
  }
}

void SimpleValueCloner::visit_uninitialized_value( UninitializedValue& )
{
  cloned_value = std::make_unique<UninitializedValue>( use_source_location );
}


void SimpleValueCloner::visit_children( Node& parent )
{
  report.error( parent,
                "Cannot optimize this expression:\n"
                "{}",
                parent );
}

}  // namespace Pol::Bscript::Compiler
