#pragma once

#include "bscript/compiler/ast/Node.h"

namespace Pol::Bscript::Compiler
{
class ClassParameterList;
class ClassBody;
class NodeVisitor;
class VarStatement;
class UserFunction;

class ClassDeclaration : public Node
{
public:
  ClassDeclaration( const SourceLocation& source_location, std::string name,
                    std::unique_ptr<ClassParameterList> parameters,
                    std::unique_ptr<ClassBody> body );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
  std::vector<std::reference_wrapper<UserFunction>> functions();
  std::vector<std::reference_wrapper<VarStatement>> variables();

  const std::string name;
};

}  // namespace Pol::Bscript::Compiler
