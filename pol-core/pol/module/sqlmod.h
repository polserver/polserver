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

  Bscript::BObjectImp* mf_ConnectToDB();
  Bscript::BObjectImp* mf_Query();
  Bscript::BObjectImp* mf_Close();
  Bscript::BObjectImp* mf_NumFields();
  Bscript::BObjectImp* mf_AffectedRows();
  Bscript::BObjectImp* mf_FetchRow();
  Bscript::BObjectImp* mf_NumRows();
  Bscript::BObjectImp* mf_SelectDb();
  Bscript::BObjectImp* mf_FieldName();

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
