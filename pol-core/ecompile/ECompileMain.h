#ifndef E_COMPILE_MAIN_H
#define E_COMPILE_MAIN_H

#include <string>

#include "../clib/Program/ProgramMain.h"


namespace Pol::ECompile
{
class ECompileMain final : public Pol::Clib::ProgramMain
{
public:
  ECompileMain();
  ~ECompileMain() override;

protected:
  int main() override;

private:
  void showHelp() override;
};
}  // namespace Pol::ECompile


#endif  // E_COMPILE_MAIN_H
