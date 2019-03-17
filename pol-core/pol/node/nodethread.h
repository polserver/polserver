/** @file
 *
 * @par History
 */


#ifndef NODETHREAD_H
#define NODETHREAD_H

#ifdef HAVE_NODEJS
#undef ERROR
#include "napi.h"


#include "../bscript/eprog.h"

#include <condition_variable>
#include <future>

using namespace Napi;

namespace Pol
{
namespace Node
{
extern Napi::ObjectReference obj;

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
};


std::future<bool> start_node();

void RegisterBuiltinModules();

// void start_node_thread();
}  // namespace Node
}  // namespace Pol
// namespace Pol
#endif

#else

#endif 
