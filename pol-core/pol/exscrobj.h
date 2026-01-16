/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef EXSCROBJ_H
#define EXSCROBJ_H


#ifndef BSCRIPT_BOBJECT_H
#include "bscript/bobject.h"
#endif

#include "clib/rawtypes.h"
#include "clib/weakptr.h"

#include "polobject.h"

#include <memory>

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

extern Bscript::BApplicObjType scriptexobjimp_type;
using ScriptExPtr = weak_ptr<UOExecutor>;
class ScriptExObjImp final : public PolApplicObj<ScriptExPtr>
{
  using base = PolApplicObj<ScriptExPtr>;

public:
  explicit ScriptExObjImp( UOExecutor* uoexec );
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;
};

struct ScriptWrapper
{
  ScriptWrapper( ScriptExPtr script );
  ~ScriptWrapper();
  ScriptExPtr _script;
};
class ExportScriptObjImp final : public PolObjectImp
{
public:
  explicit ExportScriptObjImp( UOExecutor* exec );
  explicit ExportScriptObjImp( std::shared_ptr<ScriptWrapper> pid, bool delayed );

  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                          bool forcebuiltin = false ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;

  std::string getStringRep() const override;
  size_t sizeEstimate() const override;

private:
  std::shared_ptr<ScriptWrapper> _ex;
  bool _delayed;
};
}  // namespace Core
}  // namespace Pol
#endif
