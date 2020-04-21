#ifndef BSCRIPT_EXPRESSION_H
#define BSCRIPT_EXPRESSION_H

#include <queue>
#include <stack>
#include <vector>

namespace Pol
{
namespace Bscript
{
class Token;

class Expression
{
public:
  ~Expression();

  void eat( Expression& expr );
  void eat2( Expression& expr );
  void optimize();
  int get_num_tokens( int idx ) const;

  typedef std::vector<Token*> Tokens;
  Tokens tokens;

public:
  std::stack<Token*> TX;
  std::queue<Token*> CA;

private:
  Token* optimize_operation( Token* left, Token* oper, Token* right ) const;
  Token* optimize_operation( Token* tok, Token* oper ) const;
  void optimize_binary_operations();
  void optimize_unary_operations();
  void optimize_assignments();
  bool optimize_token( int i );
};

}  // namespace Bscript
}  // namespace Pol

#endif
