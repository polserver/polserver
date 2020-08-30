/** @file
 *
 * @par History
 *
 * @note Make sure to set the module up in scrsched.cpp too
 * add_common_exmods()
 */


#ifndef STORAGEEMOD_H
#define STORAGEEMOD_H

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
namespace Module
{
class StorageExecutorModule
    : public Bscript::TmplExecutorModule<StorageExecutorModule, Core::PolModule>
{
public:
  StorageExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_StorageAreas();
  [[nodiscard]] Bscript::BObjectImp* mf_DestroyRootItemInStorageArea();
  [[nodiscard]] Bscript::BObjectImp* mf_FindStorageArea();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateStorageArea();
  [[nodiscard]] Bscript::BObjectImp* mf_FindRootItemInStorageArea();
  [[nodiscard]] Bscript::BObjectImp* mf_CreateRootItemInStorageArea();
};
}  // namespace Module
}  // namespace Pol
#endif
