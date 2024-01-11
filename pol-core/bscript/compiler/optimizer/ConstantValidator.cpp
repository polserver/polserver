#include "ConstantValidator.h"

#include "bscript/compiler/ast/ArrayInitializer.h"
#include "bscript/compiler/ast/ConstDeclaration.h"
#include "bscript/compiler/ast/DictionaryInitializer.h"
#include "bscript/compiler/ast/ErrorInitializer.h"
#include "bscript/compiler/ast/FloatValue.h"
#include "bscript/compiler/ast/FunctionCall.h"
#include "bscript/compiler/ast/Identifier.h"
#include "bscript/compiler/ast/IntegerValue.h"
#include "bscript/compiler/ast/StringValue.h"
#include "bscript/compiler/ast/StructInitializer.h"
#include "bscript/compiler/model/FunctionLink.h"

namespace Pol::Bscript::Compiler
{
ConstantValidator::ConstantValidator() : valid( false ) {}

bool ConstantValidator::validate( Node& node )
{
  valid = false;
  node.accept( *this );
  return valid;
}

void ConstantValidator::visit_array_initializer( ArrayInitializer& initializer )
{
  valid = initializer.children.empty();
}

void ConstantValidator::visit_dictionary_initializer( DictionaryInitializer& initializer )
{
  valid = initializer.children.empty();
}

void ConstantValidator::visit_error_initializer( ErrorInitializer& initializer )
{
  valid = initializer.children.empty();
}

void ConstantValidator::visit_float_value( FloatValue& )
{
  valid = true;
}

void ConstantValidator::visit_function_call( FunctionCall& fc )
{
  // this was only allowed by accident.
  valid = ( fc.function_link->module_function_declaration() && fc.children.empty() );
}

void ConstantValidator::visit_integer_value( IntegerValue& )
{
  valid = true;
}

void ConstantValidator::visit_string_value( StringValue& )
{
  valid = true;
}

void ConstantValidator::visit_struct_initializer( StructInitializer& initializer )
{
  valid = initializer.children.empty();
}

void ConstantValidator::visit_uninitialized_value( UninitializedValue& )
{
  valid = true;
}

void ConstantValidator::visit_children( Node& )
{
}

}  // namespace Pol::Bscript::Compiler
