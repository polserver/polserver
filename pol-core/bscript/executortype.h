/** @file
 *
 * @par History
 */


#ifndef BSCRIPT_EXECUTORTYPE_H
#define BSCRIPT_EXECUTORTYPE_H

namespace Pol::Bscript
{
class Executor;
class Instruction;

using ExecInstrFunc = void ( Executor::* )( const Instruction& );
}  // namespace Pol::Bscript

#endif
