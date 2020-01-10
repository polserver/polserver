/** @file
 *
 * @par History
 * - 2009/11/30 Turley:    added MD5Encrypt(string)
 */


#ifndef POLSYSTEMEMOD_H
#define POLSYSTEMEMOD_H

#include "../../bscript/execmodl.h"

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
typedef Bscript::BApplicObj<PackagePtrHolder> PackageObjImpBase;
class PackageObjImp final : public PackageObjImpBase
{
  typedef PackageObjImpBase base;

public:
  explicit PackageObjImp( const PackagePtrHolder& other );
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
  virtual Bscript::BObjectImp* call_method( const char* methodname,
                                            Bscript::Executor& ex ) override;
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
};

class PolSystemExecutorModule : public Bscript::TmplExecutorModule<PolSystemExecutorModule, Bscript::ExecutorModule>
{
public:
  PolSystemExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_IncRevision( /* uobject */ );
  Bscript::BObjectImp* mf_Packages();
  Bscript::BObjectImp* mf_GetCmdLevelName();
  Bscript::BObjectImp* mf_GetCmdLevelNumber();
  Bscript::BObjectImp* mf_GetPackageByName();
  Bscript::BObjectImp* mf_ListTextCommands();
  Bscript::BObjectImp* mf_Realms();
  Bscript::BObjectImp* mf_ReloadConfiguration();
  Bscript::BObjectImp* mf_ReadMillisecondClock();
  Bscript::BObjectImp* mf_ListenPoints();
  Bscript::BObjectImp* mf_SetSysTrayPopupText();
  Bscript::BObjectImp* mf_GetItemDescriptor();
  Bscript::BObjectImp* mf_CreatePacket();
  Bscript::BObjectImp* mf_AddRealm( /*name,base*/ );
  Bscript::BObjectImp* mf_DeleteRealm( /*name*/ );
  Bscript::BObjectImp* mf_MD5Encrypt( /*string*/ );
  Bscript::BObjectImp* mf_FormatItemDescription( /*string,amount,suffix*/ );
  Bscript::BObjectImp* mf_LogCPropProfile();
};
}  // namespace Module
}  // namespace Pol
#endif
