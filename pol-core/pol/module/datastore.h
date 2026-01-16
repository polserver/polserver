/** @file
 *
 * @par History
 */


#ifndef DATASTORE_H
#define DATASTORE_H

#include <string>

#include "../../bscript/execmodl.h"


namespace Pol::Bscript
{
class BObjectImp;
class Executor;
}  // namespace Pol::Bscript


namespace Pol::Module
{
class DataStoreFile;

class DataFileExecutorModule
    : public Bscript::TmplExecutorModule<DataFileExecutorModule, Bscript::ExecutorModule>
{
public:
  DataFileExecutorModule( Bscript::Executor& exec );
  [[nodiscard]] Bscript::BObjectImp* mf_ListDataFiles();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateDataFile();
  [[nodiscard]] Bscript::BObjectImp* mf_OpenDataFile();
  [[nodiscard]] Bscript::BObjectImp* mf_UnloadDataFile();

private:
  DataStoreFile* GetDataStoreFile( const std::string& inspec );
};
}  // namespace Pol::Module

#endif
