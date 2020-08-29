/** @file
 *
 * @par History
 */


#ifndef SQLMOD_H
#define SQLMOD_H

#include <string>

#include "../polmodl.h"
#include "../uoexec.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class Executor;
class ObjArray;
}  // namespace Bscript
namespace Core
{
class UOExecutor;
}  // namespace Core
}  // namespace Pol
template <class T>
class weak_ptr;

namespace Pol
{
namespace Core
{
class BSQLConnection;
}
namespace Module
{
class SQLExecutorModule : public Bscript::TmplExecutorModule<SQLExecutorModule, Core::PolModule>
{
public:
  SQLExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_mysql_connect();
  [[nodiscard]] Bscript::BObjectImp* mf_mysql_query();
  [[nodiscard]] Bscript::BObjectImp* mf_mysql_close();
  [[nodiscard]] Bscript::BObjectImp* mf_mysql_num_fields();
  [[nodiscard]] Bscript::BObjectImp* mf_mysql_affected_rows();
  [[nodiscard]] Bscript::BObjectImp* mf_mysql_fetch_row();
  [[nodiscard]] Bscript::BObjectImp* mf_mysql_num_rows();
  [[nodiscard]] Bscript::BObjectImp* mf_mysql_select_db();
  [[nodiscard]] Bscript::BObjectImp* mf_mysql_field_name();

  static Bscript::BObjectImp* background_connect( weak_ptr<Core::UOExecutor> uoexec,
                                                  const std::string host,
                                                  const std::string username,
                                                  const std::string password );
  static Bscript::BObjectImp* background_select( weak_ptr<Core::UOExecutor> uoexec,
                                                 Core::BSQLConnection* sql, const std::string db );
  static Bscript::BObjectImp* background_query( weak_ptr<Core::UOExecutor> uoexec,
                                                Core::BSQLConnection* sql, const std::string query,
                                                const Bscript::ObjArray* params );

  virtual size_t sizeEstimate() const override;
};
}  // namespace Module
}  // namespace Pol
#endif
