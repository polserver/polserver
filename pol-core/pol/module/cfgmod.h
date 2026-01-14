/** @file
 *
 * @par History
 */


#ifndef CFGEMOD_H
#define CFGEMOD_H

#include <stddef.h>
#include <string>

#include "../../bscript/bobject.h"
#include "../../bscript/execmodl.h"
#include "../../clib/rawtypes.h"
#include "../cfgrepos.h"

namespace Pol
{
namespace Bscript
{
class Executor;
class ExecutorModule;
}  // namespace Bscript
namespace Core
{
class StoredConfigElem;
class StoredConfigFile;
}  // namespace Core
}  // namespace Pol
template <class T>
class ref_ptr;

namespace Pol
{
namespace Module
{
class ConfigFileExecutorModule
    : public Bscript::TmplExecutorModule<ConfigFileExecutorModule, Bscript::ExecutorModule>
{
public:
  ConfigFileExecutorModule( Bscript::Executor& exec );

  [[nodiscard]] Bscript::BObjectImp* mf_ReadConfigFile();
  [[nodiscard]] Bscript::BObjectImp* mf_FindConfigElem();
  [[nodiscard]] Bscript::BObjectImp* mf_GetElemProperty();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigString();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigStringArray();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigStringDictionary();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigInt();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigIntArray();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigReal();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigMaxIntKey();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigStringKeys();
  [[nodiscard]] Bscript::BObjectImp* mf_GetConfigIntKeys();
  [[nodiscard]] Bscript::BObjectImp* mf_ListConfigElemProps();

  [[nodiscard]] Bscript::BObjectImp* mf_AppendConfigFileElem();
  [[nodiscard]] Bscript::BObjectImp* mf_UnloadConfigFile();

  [[nodiscard]] Bscript::BObjectImp* mf_LoadTusScpFile();


protected:
  bool get_cfgfilename( const std::string& cfgdesc, std::string* cfgfile, std::string* errmsg,
                        std::string* allpkgbase = nullptr );
};


typedef Bscript::BApplicObj<ref_ptr<Core::StoredConfigFile>> EConfigFileRefObjImpBase;

class EConfigFileRefObjImp final : public EConfigFileRefObjImpBase
{
public:
  EConfigFileRefObjImp( ref_ptr<Core::StoredConfigFile> rcfile );
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::ContIterator* createIterator( Bscript::BObject* pIterVal ) override;
  friend class ConfigFileIterator;
};


typedef Bscript::BApplicObj<ref_ptr<Core::StoredConfigElem>> EConfigElemRefObjImpBase;

class EConfigElemRefObjImp final : public EConfigElemRefObjImpBase
{
public:
  EConfigElemRefObjImp( ref_ptr<Core::StoredConfigElem> rcelem );
  Bscript::BObjectRef get_member( const char* membername ) override;
  Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  const char* typeOf() const override;
  u8 typeOfInt() const override;
  Bscript::BObjectImp* copy() const override;
  Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
};


bool getStoredConfigFileParam( Bscript::ExecutorModule& exmod, unsigned param,
                               Core::StoredConfigFile*& cfile );
bool getStoredConfigElemParam( Bscript::ExecutorModule& exmod, unsigned param,
                               Core::StoredConfigElem*& celem );
}  // namespace Module
}  // namespace Pol
#endif
