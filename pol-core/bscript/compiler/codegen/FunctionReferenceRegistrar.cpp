#include "FunctionReferenceRegistrar.h"

#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/representation/FunctionReferenceDescriptor.h"

namespace Pol::Bscript::Compiler
{
FunctionReferenceRegistrar::FunctionReferenceRegistrar() = default;

void FunctionReferenceRegistrar::register_function_reference( const UserFunction& node )
{
  auto& function_name = node.name;
  auto reference_itr = registered_references.find( function_name );
  if ( reference_itr == registered_references.end() )
  {
    auto reference_index = static_cast<unsigned>( registered_references.size() );

    registered_references[function_name] =
        std::make_unique<EmittedReference>( reference_index, node );
    reference_names_in_order.push_back( function_name );
  }
}

std::vector<FunctionReferenceDescriptor> FunctionReferenceRegistrar::take_descriptors()
{
  std::vector<FunctionReferenceDescriptor> descriptors;

  for ( const auto& k : reference_names_in_order )
  {
    const auto& v = registered_references[k];
    descriptors.emplace_back( v->user_function.parameter_count(), v->user_function.capture_count(),
                              v->user_function.is_variadic() );
  }

  registered_references.clear();
  reference_names_in_order.clear();

  return descriptors;
}

void FunctionReferenceRegistrar::lookup_or_register_reference( const UserFunction& node,
                                                               unsigned& reference_index )
{
  auto& function_name = node.name;

  auto reference_itr = registered_references.find( function_name );
  if ( reference_itr != registered_references.end() )
  {
    reference_index = ( *reference_itr ).second->index;
    return;
  }

  register_function_reference( node );

  reference_index = registered_references[function_name]->index;
}

FunctionReferenceRegistrar::EmittedReference::EmittedReference( unsigned index,
                                                                const UserFunction& user_function )
    : index( index ), user_function( user_function )
{
}

FunctionReferenceRegistrar::EmittedReference::~EmittedReference() = default;

}  // namespace Pol::Bscript::Compiler
