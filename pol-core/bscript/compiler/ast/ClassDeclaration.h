#pragma once

#include <map>

#include "bscript/compiler/ast/Node.h"
#include "bscript/compiler/astbuilder/AvailableParseTree.h"
namespace Pol::Bscript::Compiler
{
class ClassParameterList;
class ClassBody;
class ClassLink;
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
                    std::shared_ptr<FunctionLink> constructor_link,
                    const std::vector<std::string>& method_names, Node* body,
                    std::vector<std::shared_ptr<ClassLink>> base_classes );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::vector<std::reference_wrapper<ClassParameterDeclaration>> parameters();

  const std::string name;

  // Only contains functions that are `UserFunctionType::Method`, ie. a first
  // `this` parameter and not a constructor.
  std::map<std::string, std::shared_ptr<FunctionLink>, Clib::ci_cmp_pred> methods;

  // Owned by top_level_statements
  Node* class_body;

  // `nullptr` if class has no constructor defined.
  const std::shared_ptr<FunctionLink> constructor_link;

  // Passed as ctor parameter by UserFunctionBuilder when generating this AST
  // node. The class links are immediately registered (inside
  // UserFunctionBuilder) with the FunctionResolver so their parse trees will be
  // visited by the second-pass UserFunctionVisitor.
  std::vector<std::shared_ptr<ClassLink>> base_class_links;
};

}  // namespace Pol::Bscript::Compiler
