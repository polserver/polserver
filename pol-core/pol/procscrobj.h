/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef EXSCROBJ_H
#define EXSCROBJ_H


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../clib/rawtypes.h"
#include "../clib/weakptr.h"
#include "polobject.h"
#include <boost/process.hpp>

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

extern Bscript::BApplicObjType processobjimp_type;
struct ScriptProcessDetails
{
  weak_ptr<UOExecutor> script;
  std::shared_ptr<boost::process::child> process;
};
class ProcessObjImp final : public PolApplicObj<ScriptProcessDetails>
{
  typedef PolApplicObj<ScriptProcessDetails> base;

public:
  explicit ProcessObjImp( UOExecutor* uoexec, boost::process::child&& process );
  explicit ProcessObjImp( UOExecutor* uoexec, std::shared_ptr<boost::process::child> process );

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname,
                                               Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;

  boost::process::child& process() { return *value().process; };
  UOExecutor* script() const
  {
    return value().script.exists() ? value().script.get_weakptr() : nullptr;
  }
};
}  // namespace Core
}  // namespace Pol
#endif
