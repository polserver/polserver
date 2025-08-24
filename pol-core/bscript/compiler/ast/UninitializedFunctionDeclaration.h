#pragma once

#include "bscript/compiler/ast/Function.h"

namespace Pol::Bscript::Compiler
{
class NodeVisitor;
class FunctionParameterList;

class UninitializedFunctionDeclaration : public Function
{
public:
  UninitializedFunctionDeclaration( const SourceLocation& source_location, std::string scope,
                                    std::string name,
                                    std::unique_ptr<FunctionParameterList> parameter_list );

  void accept( NodeVisitor& visitor ) override;
  void describe_to( std::string& ) const override;
};

}  // namespace Pol::Bscript::Compiler
