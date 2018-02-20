#ifndef E_COMPILE_MAIN_H
#define E_COMPILE_MAIN_H

#include <string>

#include "../clib/Program/ProgramMain.h"

namespace Pol
{
namespace ECompile
{
class ECompileMain : public Pol::Clib::ProgramMain
{
public:
  ECompileMain();
  virtual ~ECompileMain();

protected:
  virtual int main();

private:
  virtual void showHelp();
};
}
}  // namespaces

#endif  // E_COMPILE_MAIN_H
