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
    if ( auto fn = dir_entry.path().filename().string(); !fn.empty() && *fn.begin() == '.' )
      continue;
    const auto path = dir_entry.path();
    if ( !path.extension().compare( ".txt" ) )
    {
      gamestate.tipfilenames.push_back( path.filename().string() );
    }
  }
}
}  // namespace Core
}  // namespace Pol
