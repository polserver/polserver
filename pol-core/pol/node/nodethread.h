/** @file
 *
 * @par History
 */


#ifndef NODETHREAD_H
#define NODETHREAD_H
#undef ERROR
#include "napi.h"

#include "../bscript/eprog.h"

#include <future>
#include <condition_variable>

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

struct NodeFunctions
{
  ThreadSafeFunction tsfn;
};

extern std::promise<NodeFunctions> tsfnPromise;
extern NodeFunctions nodeFuncs;
extern std::condition_variable condVar;


class Emitter : public Napi::ObjectWrap<Emitter>
{
public:
  static Napi::Object Init( Napi::Env env, Napi::Object exports );
  Emitter( const Napi::CallbackInfo& info );

  static Emitter* INSTANCE;


  Napi::Value Start( const Napi::CallbackInfo& info );
  Napi::Value Stop( const Napi::CallbackInfo& info );


  napi_status emit( const char* name );
  FunctionReference cbFromCToJs, cbFromJsToC;
  Napi::ObjectReference requireRef;

private:
  static Napi::FunctionReference constructor;

};

struct tsfndata
{
  int id;
  int clock;
  int result;
};


std::future<bool> start_node();

void RegisterBuiltinModules();

//void start_node_thread();
}  // namespace Node
}  // namespace Pol
// namespace Pol
#endif
