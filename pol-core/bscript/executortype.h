/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_EXECUTORTYPE_H
#define BSCRIPT_EXECUTORTYPE_H
namespace Pol
{
namespace Bscript
{
class Executor;
class Instruction;

using ExecInstrFunc = void ( Executor::* )( const Instruction& );
}  // namespace Bscript
}  // namespace Pol
#endif
