/** @file
 *
 * @par History
 * - 2009/11/30 Turley:    added MD5Encrypt(string)
 */


#ifndef POLSYSTEMEMOD_H
#define POLSYSTEMEMOD_H

#include "../polmodl.h"
#include "../polobject.h"
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
class PackagePtrHolder
{
public:
  PackagePtrHolder( const Plib::Package* pkg ) : m_pPkg( pkg ) {}
  const Plib::Package* operator->() const { return m_pPkg; }
  const Plib::Package* Ptr() const { return m_pPkg; }

private:
  const Plib::Package* m_pPkg;
};

// typedef BApplicObj< ref_ptr<Package> > PackageObjImpBase;
using PackageObjImpBase = Core::PolApplicObj<PackagePtrHolder>;
class PackageObjImp final : public PackageObjImpBase
{
  using base = PackageObjImpBase;

public:
  explicit PackageObjImp( const PackagePtrHolder& other );
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectImp* call_polmethod( const char* methodname, Core::UOExecutor& ex ) override;
  Bscript::BObjectRef get_member( const char* membername ) override;
};

class PolSystemExecutorModule
    : public Bscript::TmplExecutorModule<PolSystemExecutorModule, Core::PolModule>
{
public:
  PolSystemExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_IncRevision( /* uobject */ );
  [[nodiscard]] Bscript::BObjectImp* mf_Packages();
  [[nodiscard]] Bscript::BObjectImp* mf_GetCmdLevelName();
  [[nodiscard]] Bscript::BObjectImp* mf_GetCmdLevelNumber();
  [[nodiscard]] Bscript::BObjectImp* mf_GetPackageByName();
  [[nodiscard]] Bscript::BObjectImp* mf_ListTextCommands();
  [[nodiscard]] Bscript::BObjectImp* mf_Realms();
  [[nodiscard]] Bscript::BObjectImp* mf_ReloadConfiguration();
  [[nodiscard]] Bscript::BObjectImp* mf_ReadMillisecondClock();
  [[nodiscard]] Bscript::BObjectImp* mf_ListenPoints();
  [[nodiscard]] Bscript::BObjectImp* mf_SetSysTrayPopupText();
  [[nodiscard]] Bscript::BObjectImp* mf_GetItemDescriptor();
  [[nodiscard]] Bscript::BObjectImp* mf_CreatePacket();
  [[nodiscard]] Bscript::BObjectImp* mf_AddRealm( /*name,base*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_DeleteRealm( /*name*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_GetRealmDecay( /*name*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_SetRealmDecay( /*name,has_deacy*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_MD5Encrypt( /*string*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_FormatItemDescription( /*string,amount,suffix*/ );
  [[nodiscard]] Bscript::BObjectImp* mf_LogCPropProfile();
};
}  // namespace Module
}  // namespace Pol
#endif
