/** @file
 *
 * @par History
 * - 2020/04/24 Syzygy:    Moved here from parser.h
 */

#ifndef H_EXPRESSION_H
#define H_EXPRESSION_H

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

  void consume_tokens( Expression& expr );
  void optimize();
  int get_num_tokens( int idx ) const;

  typedef std::vector<Token*> Tokens;
  Tokens tokens;

public:
  std::stack<Token*> TX;
  std::queue<Token*> CA;

private:
  void optimize_binary_operations();
  void optimize_unary_operations();
  void optimize_assignments();
  bool optimize_token( int i );
  void remove_non_emitting_tokens();
};

}  // namespace Bscript
}  // namespace Pol

#endif // H_EXPRESSION_H
