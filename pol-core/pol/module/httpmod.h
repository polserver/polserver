/** @file
 *
 * @par History
 */


#ifndef __HTTPMOD_H
#define __HTTPMOD_H

#include <string>

#include "clib/network/wnsckt.h"
#include "pol/polclock.h"
#include "pol/polmodl.h"


namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript


namespace Pol
{
namespace Core
{
class UOExecutor;
}
namespace Module
{
class HttpExecutorModule : public Bscript::TmplExecutorModule<HttpExecutorModule, Core::PolModule>
{
public:
  HttpExecutorModule( Bscript::Executor& exec, Clib::Socket&& isck );
  ~HttpExecutorModule() override;

  [[nodiscard]] Bscript::BObjectImp* mf_WriteStatus();
  [[nodiscard]] Bscript::BObjectImp* mf_WriteHeader();
  [[nodiscard]] Bscript::BObjectImp* mf_WriteHtml();
  [[nodiscard]] Bscript::BObjectImp* mf_WriteHtmlRaw();
  [[nodiscard]] Bscript::BObjectImp* mf_QueryParam();
  [[nodiscard]] Bscript::BObjectImp* mf_QueryIP();

  void read_query_string( const std::string& query_string );
  void read_query_ip();
  size_t sizeEstimate() const override;

  // True while a partially sent write should be retried; false once the peer has gone
  // long enough without accepting data that the connection is considered dead, in which
  // case the socket is closed so the script stops resuming it. nsent is what the last
  // attempt managed to send, so any progress refreshes the budget.
  bool keep_retrying_send( unsigned nsent );

  // TODO: clean up the socket ownership thing so these can be private again
public:
  Clib::Socket sck_;
  using QueryParamMap = std::map<std::string, std::string, Clib::ci_cmp_pred>;
  QueryParamMap params_;
  int continuing_offset;
  // when the current partially-sent write started making no progress; zero while no
  // write is being resumed
  Core::polclock_t send_stalled_since = 0;
  std::string query_ip_;
  bool cannotSendStatus = false;
  bool cannotSendHeaders = false;
  bool hasCustomContentType = false;
};
}  // namespace Module
}  // namespace Pol
#endif  // HTTPMOD_H
