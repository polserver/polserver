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

  [[nodiscard]] Bscript::BObjectImp* mf_FileExists();
  [[nodiscard]] Bscript::BObjectImp* mf_ReadFile();
  [[nodiscard]] Bscript::BObjectImp* mf_WriteFile();
  [[nodiscard]] Bscript::BObjectImp* mf_AppendToFile();
  [[nodiscard]] Bscript::BObjectImp* mf_LogToFile();
  [[nodiscard]] Bscript::BObjectImp* mf_OpenBinaryFile();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateDirectory();
  [[nodiscard]] Bscript::BObjectImp* mf_ListDirectory();
  [[nodiscard]] Bscript::BObjectImp* mf_OpenXMLFile();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateXMLFile();
};

Bscript::ExecutorModule* CreateFileAccessExecutorModule( Bscript::Executor& exec );
}  // namespace Module
}  // namespace Pol
#endif
