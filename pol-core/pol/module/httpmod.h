/** @file
 *
 * @par History
 */


#ifndef __HTTPMOD_H
#define __HTTPMOD_H

#include <string>

#include "../../clib/network/wnsckt.h"
#include "../polmodl.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
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
class HttpExecutorModule : public Bscript::TmplExecutorModule<HttpExecutorModule, Core::PolModule>
{
public:
  HttpExecutorModule( Bscript::Executor& exec, Clib::Socket&& isck );
  ~HttpExecutorModule();

  [[nodiscard]] Bscript::BObjectImp* mf_WriteStatus();
  [[nodiscard]] Bscript::BObjectImp* mf_WriteHeader();
  [[nodiscard]] Bscript::BObjectImp* mf_WriteHtml();
  [[nodiscard]] Bscript::BObjectImp* mf_WriteHtmlRaw();
  [[nodiscard]] Bscript::BObjectImp* mf_QueryParam();
  [[nodiscard]] Bscript::BObjectImp* mf_QueryIP();

  void read_query_string( const std::string& query_string );
  void read_query_ip();
  virtual size_t sizeEstimate() const override;

  // TODO: clean up the socket ownership thing so these can be private again
public:
  Clib::Socket sck_;
  typedef std::map<std::string, std::string, Clib::ci_cmp_pred> QueryParamMap;
  QueryParamMap params_;
  int continuing_offset;
  std::string query_ip_;
  bool cannotSendStatus = false;
  bool cannotSendHeaders = false;
  bool hasCustomContentType = false;
};
}  // namespace Module
}  // namespace Pol
#endif  // HTTPMOD_H
