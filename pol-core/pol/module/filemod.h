/** @file
 *
 * @par History
 * - 2009/12/18 Turley:    added CreateDirectory() & ListDirectory()
 */


#ifndef FILEMOD_H
#define FILEMOD_H

#include "../../bscript/execmodl.h"

namespace Pol
{
namespace Bscript
{
class BObjectImp;
class ExecutorModule;
}  // namespace Bscript
}  // namespace Pol

namespace Pol
{
namespace Bscript
{
class Executor;
}

namespace Module
{
class FileAccessExecutorModule
    : public Bscript::TmplExecutorModule<FileAccessExecutorModule, Bscript::ExecutorModule>
{
public:
  FileAccessExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_FileExists();
  Bscript::BObjectImp* mf_ReadFile();
  Bscript::BObjectImp* mf_WriteFile();
  Bscript::BObjectImp* mf_AppendToFile();
  Bscript::BObjectImp* mf_LogToFile();
  Bscript::BObjectImp* mf_OpenBinaryFile();
  Bscript::BObjectImp* mf_CreateDirectory();
  Bscript::BObjectImp* mf_ListDirectory();
  Bscript::BObjectImp* mf_OpenXMLFile();
  Bscript::BObjectImp* mf_CreateXMLFile();
};

Bscript::ExecutorModule* CreateFileAccessExecutorModule( Bscript::Executor& exec );
}  // namespace Module
}  // namespace Pol
#endif
