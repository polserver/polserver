/** @file
 *
 * @par History
 */


#ifndef CLIB_IOHELP_H
#define CLIB_IOHELP_H

#include <string>
#include <fstream>

namespace Pol
{
namespace Clib
{
void open_file(std::fstream& ofs, std::string& filename, std::ios::openmode mode);
void open_file(std::ofstream& ofs, std::string& filename, std::ios::openmode mode);
void open_file(std::ifstream& ofs, std::string& filename, std::ios::openmode mode);
}
}
#endif
