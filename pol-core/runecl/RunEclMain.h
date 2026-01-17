#ifndef RUN_ECL_MAIN_H
#define RUN_ECL_MAIN_H

#include "../clib/Program/ProgramMain.h"

#include <string>


namespace Pol::Clib
{
class RunEclMain final : public ProgramMain
{
public:
  RunEclMain();
  ~RunEclMain() override;

protected:
  int main() override;

private:
  void showHelp() override;
  void dumpScript( std::string fileName );
  int runeclScript( std::string fileName );
  int runecl();

  bool m_quiet;
  bool m_debug;
  bool m_profile;
};
}  // namespace Pol::Clib


#endif  // RUN_ECL_MAIN_H
