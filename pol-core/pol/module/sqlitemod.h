#pragma once

#include "../../bscript/execmodl.h"

namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript

namespace Pol::Module
{
class SQLiteExecutorModule
    : public Bscript::TmplExecutorModule<SQLiteExecutorModule, Bscript::ExecutorModule>
{
public:
  [[nodiscard]] Bscript::BObjectImp* mf_Sqlite3_Test();

  SQLiteExecutorModule( Bscript::Executor& exec );
};
}  // namespace Pol::Module
