#ifndef GUILDSCROBJ_H
#define GUILDSCROBJ_H


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../clib/rawtypes.h"
#include "../clib/weakptr.h"
#include "globals/uvars.h"
#include "polobject.h"

namespace Pol
{
namespace Bscript
{
class Executor;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Core
{
class UOExecutor;
}

namespace Module
{
extern Bscript::BApplicObjType guild_type;

class EGuildRefObjImp final : public Core::PolApplicObj<Core::GuildRef>
{
public:
  EGuildRefObjImp( Core::GuildRef gref );
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  BObjectImp* copy() const override;
  bool isTrue() const override;
  bool operator==( const Bscript::BObjectImp& objimp ) const override;

  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
};
}  // namespace Module
}  // namespace Pol
#endif
