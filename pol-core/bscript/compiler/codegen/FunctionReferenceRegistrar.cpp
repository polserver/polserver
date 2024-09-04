#include "FunctionReferenceRegistrar.h"

#include <boost/range/adaptor/reversed.hpp>
#include <limits.h>

#include "bscript/compiler/ast/ClassDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterDeclaration.h"
#include "bscript/compiler/ast/FunctionParameterList.h"
#include "bscript/compiler/ast/FunctionReference.h"
#include "bscript/compiler/ast/UserFunction.h"
#include "bscript/compiler/model/ClassLink.h"
#include "bscript/compiler/model/FlowControlLabel.h"
#include "bscript/compiler/model/FunctionLink.h"
#include "bscript/compiler/representation/FunctionReferenceDescriptor.h"

namespace Pol::Bscript::Compiler
{
FunctionReferenceRegistrar::FunctionReferenceRegistrar() = default;

void FunctionReferenceRegistrar::register_function_reference( const UserFunction& node,
                                                              FlowControlLabel& label )
{
  auto function_name = node.scoped_name();
  auto reference_itr = registered_references.find( function_name );
  if ( reference_itr == registered_references.end() )
  {
    auto reference_index = static_cast<unsigned>( registered_references.size() );

    registered_references[function_name] =
        std::make_unique<EmittedReference>( reference_index, node, label );
    reference_names_in_order.push_back( function_name );
  }
}

std::vector<FunctionReferenceDescriptor> FunctionReferenceRegistrar::take_descriptors(
    const std::map<std::string, size_t>& class_declaration_indexes,
    const std::map<std::string, FlowControlLabel>& user_function_labels )
{
  std::vector<FunctionReferenceDescriptor> descriptors;

  for ( const auto& k : reference_names_in_order )
  {
    const auto& v = registered_references[k];
    if ( !v->label.has_address() )
    {
      v->user_function.internal_error(
          "cannot take funcref descriptors as funcref has no address" );
    }

    unsigned class_index = std::numeric_limits<unsigned>::max();

    if ( v->user_function.class_link )
    {
      if ( auto cd = v->user_function.class_link->class_declaration() )
      {
        auto cd_itr = class_declaration_indexes.find( cd->name );

        if ( cd_itr == class_declaration_indexes.end() )
        {
          v->user_function.internal_error(
              "cannot take funcref descriptors as funcref has no class declaration"
              "index" );
        }

        class_index = static_cast<unsigned>( cd_itr->second );
      }
    }

    std::vector<unsigned> default_params_addresses;
    std::vector<std::reference_wrapper<FunctionParameterDeclaration>> params;
    v->user_function.child<FunctionParameterList>( 0 ).get_children( params );

    // Emit the default arguments for parameters in reverse order. The idea
    // is that on a method call, if the executor does not have enough args on
    // the ValueStack, jumping to the instruction of the default parameter will
    // push its value to the ValueStack, and then chaining through to the
    // remaining default arguments.
    for ( const auto& param_ref : params | boost::adaptors::reversed )
    {
      const auto& param = param_ref.get();
      auto label_name =
          FlowControlLabel::label_for_user_function_default_argument( v->user_function, param );

      auto itr = user_function_labels.find( label_name );
      if ( itr != user_function_labels.end() )
      {
        if ( !itr->second.has_address() )
        {
          v->user_function.internal_error(
              "cannot take funcref descriptor for default param as funcref has no default "
              "parameter address" );
        }

        default_params_addresses.push_back( itr->second.address() );
        continue;
      }
      break;
    }

    descriptors.emplace_back( v->label.address(), v->user_function.parameter_count(),
                              v->user_function.capture_count(), v->user_function.is_variadic(),
                              class_index, v->user_function.type == UserFunctionType::Constructor,
                              std::move( default_params_addresses ) );
  }

  registered_references.clear();
  reference_names_in_order.clear();

  return descriptors;
}

void FunctionReferenceRegistrar::lookup_or_register_reference( const UserFunction& node,
                                                               FlowControlLabel& label,
                                                               unsigned& reference_index )
{
  if ( lookup_reference( node, reference_index ) )
  {
    return;
  }

  register_function_reference( node, label );

  reference_index = registered_references[node.scoped_name()]->index;
}

bool FunctionReferenceRegistrar::lookup_reference( const UserFunction& node, unsigned& index )
{
  auto reference_itr = registered_references.find( node.scoped_name() );
  if ( reference_itr != registered_references.end() )
  {
    index = ( *reference_itr ).second->index;
    return true;
  }

  return false;
}

FunctionReferenceRegistrar::EmittedReference::EmittedReference( unsigned index,
                                                                const UserFunction& user_function,
                                                                FlowControlLabel& label )
    : index( index ), user_function( user_function ), label( label )
{
}

FunctionReferenceRegistrar::EmittedReference::~EmittedReference() = default;

}  // namespace Pol::Bscript::Compiler
