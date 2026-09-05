#ifndef POLSERVER_BINARYOPERATOROPTIMIZER_H
#define POLSERVER_BINARYOPERATOROPTIMIZER_H

#include <memory>

namespace Pol::Bscript::Compiler
{
class BinaryOperator;
class Expression;
class Report;

class BinaryOperatorOptimizer
{
public:
  BinaryOperatorOptimizer( BinaryOperator& op, Report& );

  std::unique_ptr<Expression> optimize();

private:
  std::unique_ptr<Expression> optimized_result;

  BinaryOperator& op;
  Report& report;
};

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_BINARYOPERATOROPTIMIZER_H
