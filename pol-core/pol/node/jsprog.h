/** @file
 *
 * @par History
 */


#ifndef JSPROG_H
#define JSPROG_H

#include "../bscript/eprog.h"
#include "napi-wrap.h"


namespace Pol
{
namespace Node
{
class JavascriptProgram : public Bscript::Program
{
public:
  JavascriptProgram();
  static Program* create();

  bool hasProgram() const override;
  int read( const char* fname ) override;
  void package( const Plib::Package* pkg ) override;
  std::string scriptname() const override;

  ProgramType type() const override;

  Napi::ObjectReference obj;

private:
  std::string name;
  Plib::Package const* pkg;
  ~JavascriptProgram();
};

}  // namespace Node
}  // namespace Pol

#endif
