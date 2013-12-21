/*
History
=======


Notes
=======

*/

#ifndef POLFILE_H
#define POLFILE_H

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <map>
#include <vector>

#include "udatfile.h"

namespace Pol {
  namespace Core {
    int write_pol_static_files( const std::string& realm );
    void load_pol_static_files();
    void readstatics2( StaticList& vec, unsigned short x, unsigned short y );
  }
}
#endif
