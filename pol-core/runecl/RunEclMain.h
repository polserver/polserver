#ifndef RUN_ECL_MAIN_H
#define RUN_ECL_MAIN_H

#include "../clib/Program/ProgramMain.h"

#include <string>

namespace Pol
{
namespace Clib
{
class RunEclMain final : public ProgramMain
{
public:
  RunEclMain();
  virtual ~RunEclMain();

protected:
  virtual int main();

private:
  virtual void showHelp();
  void dumpScript( std::string fileName );
  int runeclScript( std::string fileName );
  int runecl();

  bool m_quiet;
  bool m_debug;
  bool m_profile;
};
}
}  // namespaces

#endif  // RUN_ECL_MAIN_H
