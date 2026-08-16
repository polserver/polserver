/** @file
 *
 * @par History
 * - 2026/08/16 Nando: extracted from the scripts_thread_script* globals added 2005/09/16
 */

#include "clib/scriptstatus.h"

#include <cstring>

namespace Pol::Clib
{
ScriptStatus script_status;

void ScriptStatus::set_script( const std::string& name )
{
  std::lock_guard<std::mutex> guard( name_mutex_ );
  std::size_t len = name.size();
  if ( len >= sizeof name_ )
    len = sizeof name_ - 1;
  std::memcpy( name_, name.data(), len );
  name_[len] = '\0';
}

bool ScriptStatus::snapshot( std::string& name, unsigned& pc ) const
{
  pc = pc_.load( std::memory_order_relaxed );

  std::unique_lock<std::mutex> guard( name_mutex_, std::try_to_lock );
  if ( !guard.owns_lock() )
  {
    // Distinct from an empty name, which means no script has run yet: a reader that cannot
    // tell those apart will read "no script" off a report and stop looking.
    name = "(unavailable)";
    return false;
  }
  name.assign( name_ );
  if ( name.empty() )
    name = "(none)";
  return true;
}

std::string ScriptStatus::script_name() const
{
  std::string name;
  unsigned pc;
  snapshot( name, pc );
  return name;
}
}  // namespace Pol::Clib
