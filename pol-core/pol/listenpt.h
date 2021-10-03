/** @file
 *
 * @par History
 */


#ifndef LISTENPT_H
#define LISTENPT_H

#include "../clib/rawtypes.h"
#include "reftypes.h"
#include <string>

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class ObjArray;
}  // namespace Bscript
namespace Mobile
{
class Character;
}
namespace Core
{
class UOExecutor;
class UObject;

class ListenPoint
{
public:
  ListenPoint( UObject* obj, UOExecutor* uoexec, u16 range, int flags );
  ~ListenPoint() = default;


  static void sayto_listening_points( Mobile::Character* speaker, const std::string& text,
                                      u8 texttype, const char* p_lang = nullptr,
                                      Bscript::ObjArray* speechtokens = nullptr );
  static void register_for_speech_events( UObject* obj, UOExecutor* uoexec, int range, int flags );
  static void deregister_from_speech_events( UOExecutor* uoexec );
  static Bscript::BObjectImp* GetListenPoints();
  static std::string TextTypeToString( u8 texttype );

private:
  UObjectRef object;
  UOExecutor* uoexec;
  u16 range;
  int flags;
};

const int LISTENPT_HEAR_GHOSTS = 0x01;
const int LISTENPT_HEAR_TOKENS = 0x02;
const int LISTENPT_NO_SPEECH = 0x04;
}  // namespace Core
}  // namespace Pol
#endif
