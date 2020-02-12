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
#include "../clib/spinlock.h"
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


class BufferReader
{
public:
  BufferReader( UOExecutor* uoexec, boost::asio::io_context& context );
  ~BufferReader();
  void bufferOne();
  void readAll();
  Bscript::BObjectImp* getline( UOExecutor* uoexec, int timeout );

public:
  boost::process::async_pipe pipe;

private:
  boost::asio::streambuf buffer;
  std::istream stream;
  std::unique_ptr<std::string> line;
  weak_ptr<UOExecutor> uoexec_w;
  bool isWaiting;
  Clib::SpinLock lineLock;
  Clib::SpinLock requestLock;
};

extern Bscript::BApplicObjType processobjimp_type;
class ScriptProcessDetails : public ref_counted
{
public:
  ScriptProcessDetails( UOExecutor* uoexec, boost::asio::io_context& ios, std::string exeName,
                        std::vector<std::string> args );
  ~ScriptProcessDetails();
  std::vector<weak_ptr<UOExecutor>> waitingScripts;
  weak_ptr_owner<ScriptProcessDetails> weakptr;
  BufferReader readerOut;
  BufferReader readerErr;
  boost::process::pipe in;
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
