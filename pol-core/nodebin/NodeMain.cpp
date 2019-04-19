
#include "pol_global_config.h"

#include <cstring>
#include "../clib/logfacility.h"
#include "NodeMain.h"
#include "node.h"


namespace Pol
{
namespace Clib
{
using namespace std;
///////////////////////////////////////////////////////////////////////////////

NodeMain::NodeMain() : ProgramMain() {}
NodeMain::~NodeMain() {}
///////////////////////////////////////////////////////////////////////////////

int NodeMain::main()
{
  Clib::Logging::global_logger->disableFileLog();

  const std::vector<std::string>& binArgs = programArgs();
  std::ostringstream os;
  std::copy( binArgs.begin(), binArgs.end() - 1,
             std::ostream_iterator<std::string>( os, " " ) );
  os << *binArgs.rbegin();
  std::string argstr = os.str();

  size_t len = argstr.size();
  char* argcstr = new char[len + 1];
  std::vector<char*> argv;

  std::memcpy( argcstr, argstr.c_str(), len + 1 );
  argv.push_back( &argcstr[0] );
  for ( size_t i = 0; i < len; ++i )
  {
    if ( argcstr[i] == ' ' )
    {
      argcstr[i] = 0;
      argv.push_back( &argcstr[i] + sizeof( char ) );
    }
  }
  argcstr[len + sizeof( char )] = 0;

  return node::Start( argv.size(), argv.data() );
}

void NodeMain::showHelp()
{
}
}  // namespace Clib
}  // namespace Pol

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[] )
{
  Pol::Clib::NodeMain* NodeMain = new Pol::Clib::NodeMain();
  NodeMain->start( argc, argv );
}
