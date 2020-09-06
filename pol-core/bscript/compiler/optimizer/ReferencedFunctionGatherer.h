#ifndef POLSERVER_REFERENCEDFUNCTIONGATHERER_H
#define POLSERVER_REFERENCEDFUNCTIONGATHERER_H

#include <memory>
#include <set>
#include <vector>

#include "compiler/ast/NodeVisitor.h"

namespace Pol::Bscript::Compiler
{
class Function;
class FunctionLink;
class FunctionReference;

class ReferencedFunctionGatherer : public NodeVisitor
{
public:
  ReferencedFunctionGatherer(
      std::vector<std::unique_ptr<ModuleFunctionDeclaration>>& all_module_function_declarations );

  void visit_function_call( FunctionCall& ) override;

  void reference( FunctionLink& link );
  void reference( ModuleFunctionDeclaration* );

  std::vector<ModuleFunctionDeclaration*> take_referenced_module_function_declarations();

private:
  // We keep the whole list of module functions, because for parity we may need to
  // include some that were not referenced.  That's why this is a vector of bare pointers.
  std::set<ModuleFunctionDeclaration*> unreferenced_module_function_declarations;
  std::vector<ModuleFunctionDeclaration*> referenced_module_function_declarations;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_REFERENCEDFUNCTIONGATHERER_H
