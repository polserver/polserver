#ifndef POLSERVER_REFERENCEDFUNCTIONGATHERER_H
#define POLSERVER_REFERENCEDFUNCTIONGATHERER_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "bscript/compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class Function;
class FunctionLink;
class FunctionReference;
class Identifier;
class UserFunction;

class ReferencedFunctionGatherer : public NodeVisitor
{
public:
  ReferencedFunctionGatherer(
      std::vector<std::unique_ptr<ModuleFunctionDeclaration>>& all_module_function_declarations,
      std::vector<std::unique_ptr<UserFunction>> all_user_functions );

  void visit_function_call( FunctionCall& ) override;
  void visit_function_reference( FunctionReference& ) override;
  void visit_function_expression( FunctionExpression& ) override;

  // Visiting a class parameter (that is, a base-class) will reference its
  // constructor.
  void visit_class_parameter_declaration( ClassParameterDeclaration& ) override;

  void reference( FunctionLink& link );
  void reference( ModuleFunctionDeclaration* );
  void reference( UserFunction* );

  std::vector<ModuleFunctionDeclaration*> take_referenced_module_function_declarations();
  std::vector<std::unique_ptr<UserFunction>> take_referenced_user_functions();

private:
  std::map<Function*, std::unique_ptr<UserFunction>> unreferenced_user_functions;
  std::vector<std::unique_ptr<UserFunction>> referenced_user_functions;

  // We keep the whole list of module functions, because for parity we may need to
  // include some that were not referenced.  That's why this is a vector of bare pointers.
  std::set<ModuleFunctionDeclaration*> unreferenced_module_function_declarations;
  std::vector<ModuleFunctionDeclaration*> referenced_module_function_declarations;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REFERENCEDFUNCTIONGATHERER_H
