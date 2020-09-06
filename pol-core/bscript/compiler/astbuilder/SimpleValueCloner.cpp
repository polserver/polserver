#include "SimpleValueCloner.h"

#include "compiler/Report.h"
#include "compiler/ast/Argument.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/IntegerValue.h"
#include "compiler/ast/StringValue.h"
#include "compiler/model/FunctionLink.h"

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

void SimpleValueCloner::visit_float_value( FloatValue& fv )
{
  cloned_value = std::make_unique<FloatValue>( use_source_location, fv.value );
}

void SimpleValueCloner::visit_function_call( FunctionCall& fc )
{
  if ( fc.function_link->module_function_declaration() && fc.children.empty() )
  {
    std::vector<std::unique_ptr<Argument>> no_args;
    auto new_fc = std::make_unique<FunctionCall>( use_source_location, fc.scope, fc.method_name,
                                                    std::move( no_args ) );
    new_fc->function_link->link_to( fc.function_link->function() );
    cloned_value = std::move( new_fc );
  }
}

void SimpleValueCloner::visit_identifier( Identifier& ident )
{
  report.error( ident, "Cannot clone '", ident.name, "' because it is not a constant.\n" );
}

void SimpleValueCloner::visit_integer_value( IntegerValue& iv )
{
  cloned_value = std::make_unique<IntegerValue>( use_source_location, iv.value );
}

void SimpleValueCloner::visit_string_value( StringValue& sv )
{
  cloned_value = std::make_unique<StringValue>( use_source_location, sv.value );
}

void SimpleValueCloner::visit_children( Node& parent )
{
  report.error( parent, "Cannot optimize this expression:\n", parent, "\n" );
}

}  // namespace Pol::Bscript::Compiler
