#pragma once

#include "bscript/compiler/astbuilder/ValueBuilder.h"
#include "bscript/compiler/model/ScopeName.h"

#ifndef __TOKENS_H
#include "bscript/tokens.h"
#endif

namespace Pol::Bscript::Compiler
{
class Argument;
class ArrayInitializer;
class BinaryOperator;
class ConditionalOperator;
class DictionaryInitializer;
class ElementAccess;
class ElvisOperator;
class ErrorInitializer;
class Expression;
class FormatExpression;
class FunctionCall;
class Identifier;
class InterpolateString;
class MemberAccess;
class MethodCall;
class Parameter;
class ParameterList;
class TypeArgumentList;
class TypeParameterList;
class TypeNode;
class UnaryOperator;

class TypeAnnotationBuilder : public ValueBuilder
{
public:
  TypeAnnotationBuilder( const SourceFileIdentifier&, BuilderWorkspace& );

  std::unique_ptr<TypeNode> type_node( EscriptGrammar::EscriptParser::TypeAnnotationContext* );
  std::unique_ptr<TypeNode> type_node( EscriptGrammar::EscriptParser::BinaryOrPrimaryTypeContext* );
  std::unique_ptr<TypeNode> type_node( EscriptGrammar::EscriptParser::TypeContext* );
  std::unique_ptr<TypeNode> type_node( EscriptGrammar::EscriptParser::PrimaryTypeContext* );
  std::unique_ptr<ParameterList> parameter_list(
      const SourceLocation& source_location_if_empty,
      EscriptGrammar::EscriptParser::ParameterListContext* );
  std::unique_ptr<TypeArgumentList> type_argument_list(
      const SourceLocation& source_location_if_empty,
      EscriptGrammar::EscriptParser::TypeArgumentListContext* );
  std::unique_ptr<TypeArgumentList> type_argument_list(
      const SourceLocation& source_location_if_empty,
      EscriptGrammar::EscriptParser::TypeGenericContext* );
  std::unique_ptr<TypeParameterList> type_parameter_list(
      const SourceLocation& source_location_if_empty,
      EscriptGrammar::EscriptParser::TypeParametersContext* );
};

}  // namespace Pol::Bscript::Compiler
