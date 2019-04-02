#ifndef NODE_MODWRAP_H
#define NODE_MODWRAP_H

#include "../../../clib/logfacility.h"
#include "../nodecall.h"
#include "../bscript/bobject.h"
#include "../napi-wrap.h"


namespace Pol
{
namespace Node
{

using namespace Napi;

class NodeModuleWrap : public Napi::ObjectWrap<NodeModuleWrap>
{
public:

  NodeModuleWrap( const Napi::CallbackInfo& info );
  static void Init( Napi::Env env, Napi::Object exports );

  /**
   * Return a Value corresponding to this impptr
   */

private:
  static Napi::FunctionReference constructor;

};
}
}


#endif
