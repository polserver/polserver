#ifndef UO_CONVERT_MAIN_H
#define UO_CONVERT_MAIN_H

#include <string>

#include "../clib/Program/ProgramMain.h"

namespace Pol
{
namespace UoConvert
{
class UoConvertMain final : public Pol::Clib::ProgramMain
{
public:
  UoConvertMain();
  virtual ~UoConvertMain();

protected:
  virtual int main();

private:
  virtual void showHelp();
};
}
}  // namespaces

#endif  // UO_CONVERT_MAIN_H
