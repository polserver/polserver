/** @file
 *
 * @par History
 */


#ifndef POLFILE_H
#define POLFILE_H

#include "udatfile.h"

#include <map>
#include <vector>


namespace Pol::Plib
{
int write_pol_static_files( const std::string& realm );
void load_pol_static_files();
void readstatics2( StaticList& vec, unsigned short x, unsigned short y );
}  // namespace Pol::Plib

#endif
