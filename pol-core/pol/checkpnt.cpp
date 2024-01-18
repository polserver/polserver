/** @file
 *
 * @par History
 */


#include "checkpnt.h"

#include "../clib/logfacility.h"
#include "../plib/systemstate.h"
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
    POLLOG_INFOLN( "checkpoint: {}", msg );
  }
}

Checkpoint::Checkpoint( const char* file ) : _file( file ), _line( 0 ) {}
Checkpoint::~Checkpoint()
{
  if ( _line )
    POLLOG_ERRORLN( "Abnormal end after checkpoint: File {}, line {}", _file, _line );
}
}
}
