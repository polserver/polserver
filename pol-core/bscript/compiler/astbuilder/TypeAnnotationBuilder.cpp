#include "TypeAnnotationBuilder.h"

// #include "bscript/compiler/Report.h"
// #include "bscript/compiler/ast/Argument.h"
// #include "bscript/compiler/ast/ArrayInitializer.h"
// #include "bscript/compiler/ast/BinaryOperator.h"
// #include "bscript/compiler/ast/BooleanValue.h"
// #include "bscript/compiler/ast/ConditionalOperator.h"
// #include "bscript/compiler/ast/DictionaryEntry.h"
// #include "bscript/compiler/ast/DictionaryInitializer.h"
// #include "bscript/compiler/ast/ElementAccess.h"
// #include "bscript/compiler/ast/ElementAssignment.h"
// #include "bscript/compiler/ast/ElementIndexes.h"
// #include "bscript/compiler/ast/ElvisOperator.h"
// #include "bscript/compiler/ast/ErrorInitializer.h"
// #include "bscript/compiler/ast/FormatExpression.h"
// #include "bscript/compiler/ast/FunctionCall.h"
// #include "bscript/compiler/ast/FunctionExpression.h"
// #include "bscript/compiler/ast/FunctionParameterDeclaration.h"
// #include "bscript/compiler/ast/FunctionParameterList.h"
// #include "bscript/compiler/ast/FunctionReference.h"
// #include "bscript/compiler/ast/Identifier.h"
// #include "bscript/compiler/ast/InterpolateString.h"
// #include "bscript/compiler/ast/MemberAccess.h"
// #include "bscript/compiler/ast/MemberAssignment.h"
// #include "bscript/compiler/ast/MethodCall.h"
// #include "bscript/compiler/ast/MethodCallArgumentList.h"
// #include "bscript/compiler/ast/ModuleFunctionDeclaration.h"
// #include "bscript/compiler/ast/SpreadElement.h"
// #include "bscript/compiler/ast/StringValue.h"
// #include "bscript/compiler/ast/StructInitializer.h"
// #include "bscript/compiler/ast/StructMemberInitializer.h"
// #include "bscript/compiler/ast/UnaryOperator.h"
// #include "bscript/compiler/ast/UninitializedValue.h"
#include "bscript/compiler/ast/types/AnyKeyword.h"
#include "bscript/compiler/ast/types/ArrayType.h"
#include "bscript/compiler/ast/types/CallSignature.h"
#include "bscript/compiler/ast/types/DictionaryType.h"
#include "bscript/compiler/ast/types/DoubleKeyword.h"
#include "bscript/compiler/ast/types/FunctionType.h"
#include "bscript/compiler/ast/types/IndexSignature.h"
#include "bscript/compiler/ast/types/IntegerKeyword.h"
#include "bscript/compiler/ast/types/IntersectionType.h"
#include "bscript/compiler/ast/types/MemberSignature.h"
#include "bscript/compiler/ast/types/MethodSignature.h"
#include "bscript/compiler/ast/types/Parameter.h"
#include "bscript/compiler/ast/types/ParameterList.h"
#include "bscript/compiler/ast/types/PropertySignature.h"
#include "bscript/compiler/ast/types/StringKeyword.h"
#include "bscript/compiler/ast/types/StructType.h"
#include "bscript/compiler/ast/types/TupleType.h"
#include "bscript/compiler/ast/types/TypeArgumentList.h"
#include "bscript/compiler/ast/types/TypeConstraint.h"
#include "bscript/compiler/ast/types/TypeParameter.h"
#include "bscript/compiler/ast/types/TypeParameterList.h"
#include "bscript/compiler/ast/types/TypeReference.h"
#include "bscript/compiler/ast/types/UninitKeyword.h"
#include "bscript/compiler/ast/types/UnionType.h"
// #include "bscript/compiler/astbuilder/BuilderWorkspace.h"
// #include "bscript/compiler/model/CompilerWorkspace.h"
// #include "bscript/compiler/model/ScopeName.h"

using EscriptGrammar::EscriptParser;

namespace Pol::Bscript::Compiler
{
TypeAnnotationBuilder::TypeAnnotationBuilder( const SourceFileIdentifier& source_file_identifier,
                                      BuilderWorkspace& workspace )
    : ValueBuilder( source_file_identifier, workspace )
{
}

std::unique_ptr<TypeNode> TypeAnnotationBuilder::type_node(
    EscriptGrammar::EscriptParser::TypeAnnotationContext* ctx )
{
  if ( !ctx )
    return nullptr;

  return type_node( ctx->type() );
}

std::unique_ptr<TypeNode> TypeAnnotationBuilder::type_node(
    EscriptGrammar::EscriptParser::BinaryOrPrimaryTypeContext* ctx )
{
  if ( !ctx )
    return nullptr;

  if ( auto primary_type_ctx = ctx->primaryType() )
  {
    return type_node( primary_type_ctx );
  }
  else if ( ctx->binaryOrPrimaryType().size() == 2 )
  {
    if ( ctx->BITAND() )
    {
      return std::make_unique<IntersectionType>( location_for( *ctx ),
                                                 type_node( ctx->binaryOrPrimaryType( 0 ) ),
                                                 type_node( ctx->binaryOrPrimaryType( 1 ) ) );
    }
    else if ( ctx->BITOR() )
    {
      return std::make_unique<UnionType>( location_for( *ctx ),
                                          type_node( ctx->binaryOrPrimaryType( 0 ) ),
                                          type_node( ctx->binaryOrPrimaryType( 1 ) ) );
    }
  }

  return std::make_unique<AnyKeyword>( location_for( *ctx ) );
}

std::unique_ptr<TypeNode> TypeAnnotationBuilder::type_node(
    EscriptGrammar::EscriptParser::TypeContext* ctx )
{
  if ( !ctx )
    return nullptr;

  if ( auto inner_type = ctx->binaryOrPrimaryType() )
  {
    return type_node( inner_type );
  }
  else if ( auto function_type = ctx->functionType() )
  {
    std::unique_ptr<TypeNode> type_annotation;
    auto params = parameter_list( location_for( *function_type ), function_type->parameterList() );
    auto type_params =
        type_parameter_list( location_for( *function_type ), function_type->typeParameters() );

    if ( auto type_ctx = function_type->type() )
    {
      type_annotation = type_node( type_ctx );
    }

    return std::make_unique<FunctionType>( location_for( *ctx ), std::move( type_params ),
                                           std::move( params ), std::move( type_annotation ) );
  }
  return std::make_unique<AnyKeyword>( location_for( *ctx ) );
}

std::unique_ptr<TypeNode> TypeAnnotationBuilder::type_node(
    EscriptGrammar::EscriptParser::PrimaryTypeContext* ctx )
{
  if ( !ctx )
    return nullptr;
  if ( auto type = ctx->type() )
  {
    return type_node( type );
  }
  else if ( auto predefined_type = ctx->predefinedType() )
  {
    if ( auto integer_ctx = predefined_type->INTEGER() )
    {
      return std::make_unique<IntegerKeyword>( location_for( *integer_ctx ) );
    }
    else if ( auto double_ctx = predefined_type->DOUBLE() )
    {
      return std::make_unique<DoubleKeyword>( location_for( *double_ctx ) );
    }
    else if ( auto string_ctx = predefined_type->STRING() )
    {
      return std::make_unique<StringKeyword>( location_for( *string_ctx ) );
    }
    else if ( auto long_ctx = predefined_type->TOK_LONG() )
    {
      return std::make_unique<IntegerKeyword>( location_for( *long_ctx ) );
    }
    else if ( auto uninit_ctx = predefined_type->UNINIT() )
    {
      return std::make_unique<UninitKeyword>( location_for( *uninit_ctx ) );
    }
    else if ( auto array_ctx = predefined_type->ARRAY() )
    {
      return std::make_unique<ArrayType>( location_for( *array_ctx ) );
    }
  }
  else if ( auto object_type_ctx = ctx->objectType() )
  {
    NodeVector members;
    if ( auto type_body_ctx = object_type_ctx->typeBody() )
    {
      if ( auto type_member_list_ctx = type_body_ctx->typeMemberList() )
      {
        for ( auto type_member : type_member_list_ctx->typeMember() )
        {
          if ( auto property = type_member->propertySignature() )
          {
            if ( auto property_name = property->propertyName() )
            {
              std::string name;
              if ( auto identifier_ctx = property_name->IDENTIFIER() )
              {
                name = text( identifier_ctx );
              }
              else if ( auto reservedWord = property_name->reservedWord() )
              {
                name = reservedWord->getText();
              }

              if ( !name.empty() )
              {
                bool question = property->QUESTION() != nullptr;
                if ( auto type_annotation = property->typeAnnotation() )
                {
                  auto type = type_node( type_annotation );
                  members.push_back( std::make_unique<PropertySignature>(
                      location_for( *property ), std::move( name ), question, std::move( type ) ) );
                }
                else
                {
                  members.push_back( std::make_unique<PropertySignature>(
                      location_for( *property ), std::move( name ), question ) );
                }
              }
            }
          }

          else if ( auto call_signature_ctx = type_member->callSignature() )
          {
            std::unique_ptr<TypeNode> type_annotation;
            auto params = parameter_list( location_for( *call_signature_ctx ),
                                          call_signature_ctx->parameterList() );
            auto type_params = type_parameter_list( location_for( *call_signature_ctx ),
                                                    call_signature_ctx->typeParameters() );


            if ( auto type_annotation_ctx = call_signature_ctx->typeAnnotation() )
            {
              type_annotation = type_node( type_annotation_ctx );
            }

            members.push_back( std::make_unique<CallSignature>(
                location_for( *call_signature_ctx ), std::move( type_params ), std::move( params ),
                std::move( type_annotation ) ) );
          }

          else if ( auto index_signature = type_member->indexSignature() )
          {
            if ( auto type_annotation = index_signature->typeAnnotation() )
            {
              auto params = parameter_list( location_for( *index_signature ),
                                            index_signature->parameterList() );

              members.push_back( std::make_unique<IndexSignature>( location_for( *index_signature ),
                                                                   std::move( params ),
                                                                   type_node( type_annotation ) ) );
            }
          }

          else if ( auto method_signature = type_member->methodSignature() )
          {
            if ( auto property_name = method_signature->propertyName() )
            {
              std::string name;
              if ( auto identifier_ctx = property_name->IDENTIFIER() )
              {
                name = text( identifier_ctx );
              }
              else if ( auto reservedWord = property_name->reservedWord() )
              {
                name = reservedWord->getText();
              }

              if ( auto call_signature_ctx = method_signature->callSignature() )
              {
                auto params = parameter_list( location_for( *method_signature ),
                                              call_signature_ctx->parameterList() );


                auto question = method_signature->QUESTION() != nullptr;
                if ( auto type_annotation = call_signature_ctx->typeAnnotation() )
                {
                  members.push_back( std::make_unique<MethodSignature>(
                      location_for( *method_signature ), std::move( name ), question,
                      std::move( params ), type_node( type_annotation ) ) );
                }
                else
                {
                  members.push_back( std::make_unique<MethodSignature>(
                      location_for( *method_signature ), std::move( name ), question,
                      std::move( params ) ) );
                }
              }
            }
          }
        }
      }
    }

    if ( auto dictionary_terminal = object_type_ctx->DICTIONARY() )
    {
      return std::make_unique<DictionaryType>( location_for( *dictionary_terminal ),
                                               std::move( members ) );
    }
    else if ( auto struct_terminal = object_type_ctx->STRUCT() )
    {
      return std::make_unique<StructType>( location_for( *struct_terminal ), std::move( members ) );
    }
  }
  else if ( auto type_reference_ctx = ctx->typeReference() )
  {
    std::unique_ptr<TypeArgumentList> type_arguments;

    if ( auto generic_ctx = type_reference_ctx->typeGeneric() )
    {
      if ( auto type_argument_list_ctx = generic_ctx->typeArgumentList() )
      {
        type_arguments = type_argument_list( location_for( *generic_ctx ), type_argument_list_ctx );
      }
    }

    if ( auto identifier_name_ctx = type_reference_ctx->identifierName() )
    {
      if ( auto identifier_ctx = identifier_name_ctx->IDENTIFIER() )
      {
        auto type_name = text( identifier_ctx );

        if ( Clib::caseInsensitiveEqual( type_name, "any" ) )
        {
          return std::make_unique<AnyKeyword>( location_for( *identifier_ctx ) );
        }

        if ( type_arguments )
          return std::make_unique<TypeReference>( location_for( *identifier_ctx ), type_name,
                                                  std::move( type_arguments ) );

        return std::make_unique<TypeReference>( location_for( *identifier_ctx ), type_name );
      }
      else if ( auto reservedWord = identifier_name_ctx->reservedWord() )
      {
        if ( type_arguments )
          return std::make_unique<TypeReference>(
              location_for( *reservedWord ), reservedWord->getText(), std::move( type_arguments ) );

        return std::make_unique<TypeReference>( location_for( *reservedWord ),
                                                reservedWord->getText() );
      }
    }
  }
  else if ( auto primary_type_ctx = ctx->primaryType() )
  {
    if ( ctx->LBRACK() && ctx->RBRACK() )
    {
      return std::make_unique<ArrayType>( location_for( *primary_type_ctx ),
                                          type_node( primary_type_ctx ) );
    }
  }
  else if ( auto tuple_ctx = ctx->tupleElementTypes() )
  {
    if ( ctx->LBRACK() && ctx->RBRACK() )
    {
      NodeVector elements;
      for ( auto& type : tuple_ctx->type() )
      {
        elements.push_back( type_node( type ) );
      }

      return std::make_unique<TupleType>( location_for( *tuple_ctx ), std::move( elements ) );
    }
  }
  return std::make_unique<AnyKeyword>( location_for( *ctx ) );
}

std::unique_ptr<ParameterList> TypeAnnotationBuilder::parameter_list(
    const SourceLocation& source_location_if_empty,
    EscriptGrammar::EscriptParser::ParameterListContext* ctx )
{
  std::vector<std::unique_ptr<Parameter>> parameters;

  if ( !ctx )
  {
    return std::make_unique<ParameterList>( source_location_if_empty, std::move( parameters ) );
  }

  for ( auto& parameter : ctx->parameter() )
  {
    auto name = text( parameter->IDENTIFIER() );
    bool question = parameter->QUESTION() != nullptr || parameter->ELVIS() != nullptr;
    if ( auto type_ctx = parameter->type() )
    {
      parameters.push_back( std::make_unique<Parameter>(
          location_for( *parameter ), std::move( name ), question, false, type_node( type_ctx ) ) );
    }
    else
    {
      parameters.push_back( std::make_unique<Parameter>( location_for( *parameter ),
                                                         std::move( name ), question, false ) );
    }
  }
  if ( auto rest = ctx->restParameter() )
  {
    auto name = text( rest->IDENTIFIER() );
    if ( auto type_ctx = rest->typeAnnotation() )
    {
      parameters.push_back( std::make_unique<Parameter>( location_for( *rest ), std::move( name ),
                                                         false, true, type_node( type_ctx ) ) );
    }
    else
    {
      parameters.push_back(
          std::make_unique<Parameter>( location_for( *rest ), std::move( name ), false, true ) );
    }
  }

  return std::make_unique<ParameterList>( location_for( *ctx ), std::move( parameters ) );
}

std::unique_ptr<TypeArgumentList> TypeAnnotationBuilder::type_argument_list(
    const SourceLocation& source_location_if_empty,
    EscriptGrammar::EscriptParser::TypeArgumentListContext* ctx )
{
  std::vector<std::unique_ptr<TypeNode>> type_arguments;

  if ( !ctx )
  {
    return std::make_unique<TypeArgumentList>( source_location_if_empty,
                                               std::move( type_arguments ) );
  }

  for ( auto& type_argument_ctx : ctx->typeArgument() )
  {
    type_arguments.push_back( type_node( type_argument_ctx->type() ) );
  }
  return std::make_unique<TypeArgumentList>( location_for( *ctx ), std::move( type_arguments ) );
}

std::unique_ptr<TypeArgumentList> TypeAnnotationBuilder::type_argument_list(
    const SourceLocation& source_location_if_empty,
    EscriptGrammar::EscriptParser::TypeGenericContext* ctx )
{
  if ( !ctx )
  {
    return std::make_unique<TypeArgumentList>( source_location_if_empty );
  }

  return type_argument_list( location_for( *ctx ), ctx->typeArgumentList() );
}

std::unique_ptr<TypeParameterList> TypeAnnotationBuilder::type_parameter_list(
    const SourceLocation& source_location_if_empty,
    EscriptGrammar::EscriptParser::TypeParametersContext* ctx )
{
  std::vector<std::unique_ptr<TypeParameter>> type_parameters;

  if ( !ctx )
  {
    return std::make_unique<TypeParameterList>( source_location_if_empty,
                                                std::move( type_parameters ) );
  }

  if ( auto type_parameter_list_ctx = ctx->typeParameterList() )
  {
    for ( auto& type_parameter_ctx : type_parameter_list_ctx->typeParameter() )
    {
      auto name = text( type_parameter_ctx->IDENTIFIER() );
      auto constraint_ctx = type_parameter_ctx->constraint();
      auto default_value_ctx = type_parameter_ctx->typeArgument();
      auto has_constraint = constraint_ctx && constraint_ctx->type();
      auto has_default_value = default_value_ctx && default_value_ctx->type();
      if ( has_constraint && has_default_value )
      {
        type_parameters.push_back( std::make_unique<TypeParameter>(
            location_for( *type_parameter_ctx ), std::move( name ),
            type_node( default_value_ctx->type() ),
            std::make_unique<TypeConstraint>( location_for( *type_parameter_ctx ),
                                              type_node( constraint_ctx->type() ) ) ) );
      }
      else if ( has_constraint )
      {
        type_parameters.push_back( std::make_unique<TypeParameter>(
            location_for( *type_parameter_ctx ), std::move( name ),
            std::make_unique<TypeConstraint>( location_for( *type_parameter_ctx ),
                                              type_node( constraint_ctx->type() ) ) ) );
      }
      else if ( has_default_value )
      {
        type_parameters.push_back(
            std::make_unique<TypeParameter>( location_for( *type_parameter_ctx ), std::move( name ),
                                             type_node( default_value_ctx->type() ) ) );
      }
      else
      {
        type_parameters.push_back( std::make_unique<TypeParameter>(
            location_for( *type_parameter_ctx ), std::move( name ) ) );
      }
    }
  }

  return std::make_unique<TypeParameterList>( location_for( *ctx ), std::move( type_parameters ) );
}
}  // namespace Pol::Bscript::Compiler
