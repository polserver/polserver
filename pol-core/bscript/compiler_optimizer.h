#ifndef BSCRIPT_COMPILEROPT_H
#define BSCRIPT_COMPILEROPT_H

namespace Pol
{
namespace Bscript
{
class Token;

struct CompilerOptimization
{
  static Token* optimize( Token* left, Token* oper, Token* right );
};

}  // namespace Bscript
}  // namespace Pol

#endif
