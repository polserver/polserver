#ifndef POLSERVER_CONSTANTFOLDER_H
#define POLSERVER_CONSTANTFOLDER_H

#include <memory>

namespace Pol::Bscript::Compiler
{
class BinaryOperator;
class Expression;
class Report;
class UnaryOperator;

/**
 * Folds an operator applied to constant operands.
 *
 * The fold is performed by building the BObjectImp values the operands stand for and applying the
 * very operator implementations the VM applies, rather than re-deriving the arithmetic here. A
 * folded expression and its unfolded twin therefore cannot disagree, and they stay in step when
 * the VM's semantics change. See specs/escript/03.
 *
 * Returns an empty pointer when the operands are not constants, or when the operator produces
 * something with no literal form (an error object, an array, ...); the expression is then left for
 * the VM to evaluate.
 */
namespace ConstantFolder
{
std::unique_ptr<Expression> fold( BinaryOperator& op, Report& report );
std::unique_ptr<Expression> fold( UnaryOperator& op );
}  // namespace ConstantFolder

}  // namespace Pol::Bscript::Compiler

#endif  // POLSERVER_CONSTANTFOLDER_H
