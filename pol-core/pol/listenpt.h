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
  ListenPoint( UObject* obj, UOExecutor* uoexec, int range, int flags );
  ~ListenPoint();

  UObjectRef object;
  UOExecutor* uoexec;
  int range;
  int flags;
};

const char* TextTypeToString( u8 texttype );

void sayto_listening_points( Mobile::Character* speaker, const std::string& text, u8 texttype,
                             const char* p_lang = nullptr,
                             Bscript::ObjArray* speechtokens = nullptr );

const int LISTENPT_HEAR_GHOSTS = 0x01;
const int LISTENPT_HEAR_TOKENS = 0x02;
const int LISTENPT_NO_SPEECH = 0x04;

void register_for_speech_events( UObject* obj, UOExecutor* uoexec, int range, int flags );
void deregister_from_speech_events( UOExecutor* uoexec );
void clear_listen_points();

Bscript::BObjectImp* GetListenPoints();
}  // namespace Core
}  // namespace Pol
#endif
