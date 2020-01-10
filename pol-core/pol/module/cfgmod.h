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
class ConfigFileExecutorModule : public Bscript::TmplExecutorModule<ConfigFileExecutorModule, Bscript::ExecutorModule>
{
public:
  ConfigFileExecutorModule( Bscript::Executor& exec );

  Bscript::BObjectImp* mf_ReadConfigFile();
  Bscript::BObjectImp* mf_FindConfigElem();
  Bscript::BObjectImp* mf_GetElemProperty();
  Bscript::BObjectImp* mf_GetConfigString();
  Bscript::BObjectImp* mf_GetConfigStringArray();
  Bscript::BObjectImp* mf_GetConfigStringDictionary();
  Bscript::BObjectImp* mf_GetConfigInt();
  Bscript::BObjectImp* mf_GetConfigIntArray();
  Bscript::BObjectImp* mf_GetConfigReal();
  Bscript::BObjectImp* mf_GetConfigMaxIntKey();
  Bscript::BObjectImp* mf_GetConfigStringKeys();
  Bscript::BObjectImp* mf_GetConfigIntKeys();
  Bscript::BObjectImp* mf_ListConfigElemProps();

  Bscript::BObjectImp* mf_AppendConfigFileElem();
  Bscript::BObjectImp* mf_UnloadConfigFile();

  Bscript::BObjectImp* mf_LoadTusScpFile();


protected:
  bool get_cfgfilename( const std::string& cfgdesc, std::string* cfgfile, std::string* errmsg,
                        std::string* allpkgbase = nullptr );
};


typedef Bscript::BApplicObj<ref_ptr<Core::StoredConfigFile>> EConfigFileRefObjImpBase;

class EConfigFileRefObjImp final : public EConfigFileRefObjImpBase
{
public:
  EConfigFileRefObjImp( ref_ptr<Core::StoredConfigFile> rcfile );
  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj ) override;
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
};


typedef Bscript::BApplicObj<ref_ptr<Core::StoredConfigElem>> EConfigElemRefObjImpBase;

class EConfigElemRefObjImp final : public EConfigElemRefObjImpBase
{
public:
  EConfigElemRefObjImp( ref_ptr<Core::StoredConfigElem> rcelem );
  virtual Bscript::BObjectRef get_member( const char* membername ) override;
  virtual Bscript::BObjectRef get_member_id( const int id ) override;  // id test
  virtual const char* typeOf() const override;
  virtual u8 typeOfInt() const override;
  virtual Bscript::BObjectImp* copy() const override;
};


bool getStoredConfigFileParam( Bscript::ExecutorModule& exmod, unsigned param,
                               Core::StoredConfigFile*& cfile );
bool getStoredConfigElemParam( Bscript::ExecutorModule& exmod, unsigned param,
                               Core::StoredConfigElem*& celem );
}
}
#endif
