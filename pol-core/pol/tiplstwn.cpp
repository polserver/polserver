/** @file
 *
 * @par History
 */


#include <filesystem>
#include <string>

#include "globals/uvars.h"

namespace Pol
{
namespace Core
{
namespace fs = std::filesystem;
void load_tips()
{
  gamestate.tipfilenames.clear();

  std::error_code ec;
  for ( const auto& dir_entry : fs::directory_iterator( "tips", ec ) )
  {
    if ( !dir_entry.is_regular_file() )
      continue;
    const auto path = dir_entry.path();
    if ( !path.extension().compare( ".txt" ) )
    {
      gamestate.tipfilenames.push_back( path.filename().u8string() );
    }
  }
}
}  // namespace Core
}  // namespace Pol
