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

  void eat2( Expression& expr );
  void optimize();
  void optimize_binary_operations();
  void optimize_unary_operations();
  void optimize_assignments();
  int get_num_tokens( int idx ) const;
  bool optimize_token( int i );

  typedef std::vector<Token*> Tokens;
  Tokens tokens;

public:
  std::stack<Token*> TX;
  std::queue<Token*> CA;
};

}  // namespace Bscript
}  // namespace Pol

#endif // H_EXPRESSION_H
