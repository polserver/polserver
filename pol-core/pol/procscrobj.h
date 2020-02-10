/** @file
 *
 * @par History
 * - 2009/12/21 Turley:    ._method() call fix
 */


#ifndef PROCSCROBJ_H
#define PROCSCROBJ_H


#ifndef BSCRIPT_BOBJECT_H
#include "../bscript/bobject.h"
#endif

#include "../clib/rawtypes.h"
#include "../clib/refptr.h"
#include "../clib/weakptr.h"
#include "polobject.h"
#include <boost/asio/streambuf.hpp>
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
class ScriptProcessDetails : public ref_counted
{
public:
  ScriptProcessDetails( boost::asio::io_context& ios, std::string exeName,
                        std::vector<std::string> args );
  ~ScriptProcessDetails();
  std::vector<weak_ptr<UOExecutor>> waitingScripts;
  boost::process::pipe in;
  boost::process::async_pipe out;
  boost::process::async_pipe err;
  boost::asio::streambuf outBuf;
  boost::asio::streambuf errBuf;
  std::string exeName;
  int exitCode;
  boost::process::child process;
};

class ScriptProcessDetailsRef : public ref_ptr<ScriptProcessDetails>
{
public:
  explicit ScriptProcessDetailsRef( ScriptProcessDetails* details );
};
class ProcessObjImp final : public PolApplicObj<ScriptProcessDetailsRef>
{
  typedef PolApplicObj<ScriptProcessDetailsRef> base;

public:
  explicit ProcessObjImp( UOExecutor* uoexec, boost::asio::io_context& ios, std::string exeName,
                          std::vector<std::string> args );
  explicit ProcessObjImp( ScriptProcessDetailsRef other );

  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_polmethod( const char* methodname,
                                               Core::UOExecutor& ex ) override;
  virtual Bscript::BObjectImp* call_polmethod_id( const int id, Core::UOExecutor& ex,
                                                  bool forcebuiltin = false ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;

  boost::process::child& process() const { return value()->process; }
  const std::string& exeName() const { return value()->exeName; }
};
}  // namespace Core
}  // namespace Pol
#endif
