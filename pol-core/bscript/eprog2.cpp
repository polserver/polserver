/** @file
 *
 * @par History
 */


#include <stdio.h>
#include <string.h>
#include <string>

#include "../clib/clib.h"
#include "../clib/logfacility.h"
#include "../clib/passert.h"
#include "../clib/rawtypes.h"
#include "../clib/stlutil.h"
#include "eprog.h"
/*#include "filefmt.h"
#include "fmodule.h"
#include "symcont.h"
#include "token.h"
#include "tokens.h"
*/
namespace Pol
{
namespace Bscript
{
std::string EScriptProgram::dbg_get_instruction( size_t atPC ) const
{
  OSTRINGSTREAM os;
  os << instr[atPC].token;
  return OSTRINGSTREAM_STR( os );
}

size_t EScriptProgram::sizeEstimate() const
{
  using namespace Clib;
  size_t size = sizeof( EScriptProgram );
  size += memsize( globalvarnames );
  for ( const auto& l : globalvarnames )
    size += l.capacity();
  size += memsize( dbg_filenames );
  for ( const auto& l : dbg_filenames )
    size += l.capacity();
  size += memsize( dbg_filenum ) + memsize( dbg_linenum ) + memsize( dbg_ins_blocks ) +
          memsize( dbg_ins_statementbegin ) + memsize( modules ) + memsize( exported_functions ) +
          memsize( instr ) + memsize( blocks ) + memsize( dbg_functions );

  return size;
}
}  // namespace Bscript
}  // namespace Pol
