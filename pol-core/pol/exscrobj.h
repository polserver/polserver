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
typedef weak_ptr<UOExecutor> ScriptExPtr;
class ScriptExObjImp final : public PolApplicObj<ScriptExPtr>
{
  typedef PolApplicObj<ScriptExPtr> base;

public:
  explicit ScriptExObjImp( UOExecutor* uoexec );
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname,
                                               Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;
};

struct PIDWrapper
{
  PIDWrapper( u32 pid );
  ~PIDWrapper();
  u32 _pid;
};
class ExportScriptObjImp final : public PolObjectImp
{
public:
  explicit ExportScriptObjImp( UOExecutor* exec );
  explicit ExportScriptObjImp( std::shared_ptr<PIDWrapper> pid, bool delayed );

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname,
                                               Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;

  virtual std::string getStringRep() const override;
  virtual size_t sizeEstimate() const override;

private:
  std::shared_ptr<PIDWrapper> _ex;
  bool _delayed;
};
}  // namespace Core
}  // namespace Pol
#endif
