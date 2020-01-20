#ifndef PARTYSCROBJ_H
#define PARTYSCROBJ_H


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
extern Bscript::BApplicObjType party_type;

class EPartyRefObjImp final : public Core::PolApplicObj<Core::PartyRef>
{
public:
  EPartyRefObjImp( Core::PartyRef pref );
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual bool isTrue() const override;
  virtual bool operator==( const BObjectImp& objimp ) const override;

  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname,
                                               Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                         bool forcebuiltin = false ) override;
};

}  // namespace Module
}  // namespace Pol
#endif
