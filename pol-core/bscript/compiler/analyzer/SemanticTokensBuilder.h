#ifndef POLSERVER_SEMANTICTOKENSBUILDER_H
#define POLSERVER_SEMANTICTOKENSBUILDER_H

#include "bscript/compiler/model/SemanticTokens.h"
#include <initializer_list>
#include <vector>

namespace Pol::Bscript::Compiler
{
class CompilerWorkspace;
class Report;

class SemanticTokensBuilder
{
public:
  SemanticTokensBuilder( CompilerWorkspace& workspace, Report& );

  void build();

private:
  CompilerWorkspace& workspace;
  Report& report;
};
}  // namespace Pol::Bscript::Compiler

#endif
