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
  static Token* optimize( Token* tok, Token* oper);
};

}  // namespace Bscript
}  // namespace Pol

#endif
