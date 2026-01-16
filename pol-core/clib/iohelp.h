/** @file
 *
 * @par History
 */


#ifndef CLIB_IOHELP_H
#define CLIB_IOHELP_H

#include <fstream>
#include <string>


namespace Pol::Clib
{
void open_file( std::fstream& ofs, std::string& filename, std::ios::openmode mode );
void open_file( std::ofstream& ofs, std::string& filename, std::ios::openmode mode );
void open_file( std::ifstream& ofs, std::string& filename, std::ios::openmode mode );
}  // namespace Pol::Clib

#endif
