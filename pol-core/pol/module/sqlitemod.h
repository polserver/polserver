#pragma once

#include "../polmodl.h"

namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript

namespace Pol::Module
{
class SQLiteExecutorModule
    : public Bscript::TmplExecutorModule<SQLiteExecutorModule, Core::PolModule>
{
public:
  [[nodiscard]] Bscript::BObjectImp* mf_sqlite_open();

  SQLiteExecutorModule( Bscript::Executor& exec );
};
}  // namespace Pol::Module
