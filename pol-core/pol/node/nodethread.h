/** @file
 *
 * @par History
 */


#ifndef NODETHREAD_H
#define NODETHREAD_H

#ifdef HAVE_NODEJS
#undef ERROR
#include "../bscript/eprog.h"
#include "napi.h"
#include <condition_variable>
#include <future>


enum
{
  NM_F_BUILTIN = 1 << 0,
  NM_F_LINKED = 1 << 1,
  NM_F_INTERNAL = 1 << 2,
};

#define NODE_API_MODULE_LINKED( modname, regfunc )                \
  napi_value __napi_##regfunc( napi_env env, napi_value exports ) \
  {                                                               \
    return Napi::RegisterModule( env, exports, regfunc );         \
  }                                                               \
  NAPI_MODULE_X( modname, __napi_##regfunc, nullptr,              \
                 NM_F_LINKED )  // NOLINT (readability/null_usage)


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
  ~JavascriptProgram();
};


std::future<bool> start_node();
std::future<Napi::ObjectReference> require( const std::string& name );
std::future<bool> release( Napi::ObjectReference ref );
std::future<bool> call( Napi::ObjectReference& ref );

extern std::atomic<bool> running;

void cleanup();

void RegisterBuiltinModules();

// void start_node_thread();
}  // namespace Node
}  // namespace Pol
// namespace Pol
#endif

#else

#endif
