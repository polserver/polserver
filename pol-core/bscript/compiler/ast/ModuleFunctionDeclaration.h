#ifndef POLSERVER_MODULEFUNCTIONDECLARATION_H
#define POLSERVER_MODULEFUNCTIONDECLARATION_H

#include "bscript/compiler/ast/Function.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class FunctionParameterDeclaration;
class FunctionParameterList;

class ModuleFunctionDeclaration : public Function
{
public:
  ModuleFunctionDeclaration( const SourceLocation& source_location, std::string module_name,
                             std::string name,
                             std::unique_ptr<FunctionParameterList> parameter_list );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;

  const std::string module_name;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_MODULEFUNCTIONDECLARATION_H
