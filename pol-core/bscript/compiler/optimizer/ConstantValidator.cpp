#include "ConstantValidator.h"

#include "compiler/ast/ArrayInitializer.h"
#include "compiler/ast/ConstDeclaration.h"
#include "compiler/ast/DictionaryInitializer.h"
#include "compiler/ast/ErrorInitializer.h"
#include "compiler/ast/FloatValue.h"
#include "compiler/ast/FunctionCall.h"
#include "compiler/ast/Identifier.h"
#include "compiler/ast/IntegerValue.h"
#include "compiler/ast/StringValue.h"
#include "compiler/ast/StructInitializer.h"
#include "compiler/model/FunctionLink.h"

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

void ConstantValidator::visit_children( Node& )
{
}

}  // namespace Pol::Bscript::Compiler
