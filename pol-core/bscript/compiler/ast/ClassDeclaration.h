#pragma once

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/astbuilder/AvailableParseTree.h"
namespace Pol::Bscript::Compiler
{
class ClassParameterList;
class ClassBody;
class DefaultConstructorFunction;
class Identifier;
class NodeVisitor;
class VarStatement;
class UserFunction;

class ClassDeclaration : public Node
{
public:
  ClassDeclaration( const SourceLocation& source_location, std::string name,
                    std::unique_ptr<ClassParameterList> parameters,
                    std::vector<std::string> function_names,
                    std::unique_ptr<DefaultConstructorFunction> default_constructor );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::vector<std::reference_wrapper<Identifier>> parameters();

  const std::string name;
  std::vector<std::string> function_names;
};

}  // namespace Pol::Bscript::Compiler
