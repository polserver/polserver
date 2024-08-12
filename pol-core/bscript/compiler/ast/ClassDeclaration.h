#pragma once

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/astbuilder/AvailableParseTree.h"
namespace Pol::Bscript::Compiler
{
class ClassParameterList;
class ClassBody;
class ClassParameterDeclaration;
class NodeVisitor;
class VarStatement;
class UserFunction;
class FunctionLink;

class ClassDeclaration : public Node
{
public:
  ClassDeclaration( const SourceLocation& source_location, std::string name,
                    std::unique_ptr<ClassParameterList> parameters,
                    std::vector<std::string> function_names, Node* body,
                    std::shared_ptr<FunctionLink> constructor );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::vector<std::reference_wrapper<ClassParameterDeclaration>> parameters();

  const std::string name;
  std::vector<std::string> function_names;

  // Owned by top_level_statements
  Node* class_body;

  // Will be null if class has no constructor declared.
  const std::shared_ptr<FunctionLink> constructor;
};

}  // namespace Pol::Bscript::Compiler
