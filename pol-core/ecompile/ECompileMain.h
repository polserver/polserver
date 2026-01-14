#ifndef E_COMPILE_MAIN_H
#define E_COMPILE_MAIN_H

#include <string>

#include "../clib/Program/ProgramMain.h"

namespace Pol
{
namespace ECompile
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
}  // namespace ECompile
}  // namespace Pol

#endif  // E_COMPILE_MAIN_H
