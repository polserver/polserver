/** @file
 *
 * @par History
 */


#ifndef SQLMOD_H
#define SQLMOD_H

#include <string>

#include "../../bscript/execmodl.h"
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
class SQLExecutorModule : public Bscript::TmplExecutorModule<SQLExecutorModule>
{
public:
  SQLExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_mysql_connect();
  Bscript::BObjectImp* mf_mysql_query();
  Bscript::BObjectImp* mf_mysql_close();
  Bscript::BObjectImp* mf_mysql_num_fields();
  Bscript::BObjectImp* mf_mysql_affected_rows();
  Bscript::BObjectImp* mf_mysql_fetch_row();
  Bscript::BObjectImp* mf_mysql_num_rows();
  Bscript::BObjectImp* mf_mysql_select_db();
  Bscript::BObjectImp* mf_mysql_field_name();

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

private:
  Core::UOExecutor& uoexec;
};
}  // namespace Module
}  // namespace Pol
#endif
