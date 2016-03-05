/** @file
 *
 * @par History
 */


#include "checkpnt.h"

#include "../plib/systemstate.h"
#include "../clib/logfacility.h"

#include "globals/state.h"

namespace Pol
{
namespace Core
{
void checkpoint( const char* msg, unsigned short minlvl /* = 11 */ )
{
  Core::stateManager.last_checkpoint = msg;
  if ( Plib::systemstate.config.loglevel >= minlvl )
  {
    POLLOG_INFO << "checkpoint: " << msg << "\n";
  }
}

Checkpoint::Checkpoint( const char* file ) : _file( file ), _line( 0 )
{}
Checkpoint::~Checkpoint()
{
  if ( _line )
    POLLOG_ERROR << "Abnormal end after checkpoint: File " << _file << ", line " << _line << "\n";
}
}
}