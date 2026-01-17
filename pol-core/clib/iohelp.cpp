/** @file
 *
 * @par History
 */


#include "iohelp.h"

#include <stdexcept>

// note this is only useful if ofs.exceptions( std::ios_base::failbit | std::ios_base::badbit ) is
// called

namespace Pol::Clib
{
void open_file( std::fstream& ofs, std::string& filename, std::ios::openmode mode )
{
  try
  {
    ofs.open( filename.c_str(), mode );
  }
  catch ( std::exception& ex )
  {
    std::string message = "Error opening file " + filename + ": " + ex.what();
    throw std::runtime_error( message );
  }
}
void open_file( std::ofstream& ofs, std::string& filename, std::ios::openmode mode )
{
  try
  {
    ofs.open( filename.c_str(), mode );
  }
  catch ( std::exception& ex )
  {
    std::string message = "Error opening file " + filename + ": " + ex.what();
    throw std::runtime_error( message );
  }
}

void open_file( std::ifstream& ifs, std::string& filename, std::ios::openmode mode )
{
  try
  {
    ifs.open( filename.c_str(), mode );
  }
  catch ( std::exception& ex )
  {
    std::string message = "Error opening file " + filename + ": " + ex.what();
    throw std::runtime_error( message );
  }
}
}  // namespace Pol::Clib
