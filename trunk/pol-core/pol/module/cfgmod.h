/*
History
=======


Notes
=======

*/

#ifndef CFGEMOD_H
#define CFGEMOD_H

#include "../../bscript/execmodl.h"

#include "../cfgrepos.h"
namespace Pol {
  namespace Module {
	class ConfigFileExecutorModule : public Bscript::TmplExecutorModule<ConfigFileExecutorModule>
	{
	public:
	  ConfigFileExecutorModule( Bscript::Executor& exec ) :
		TmplExecutorModule<ConfigFileExecutorModule>( "cfgfile", exec ) {};

	  Bscript::BObjectImp* mf_ConfigFile();
	  Bscript::BObjectImp* mf_FindConfigElement();
	  Bscript::BObjectImp* mf_GetElementProperty();
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
	  bool get_cfgfilename( const std::string& cfgdesc,
							std::string* cfgfile,
							std::string* errmsg,
							std::string* allpkgbase = NULL );
	};


	typedef Bscript::BApplicObj< ref_ptr<Core::StoredConfigFile> > EConfigFileRefObjImpBase;

	class EConfigFileRefObjImp : public EConfigFileRefObjImpBase
	{
	public:
	  EConfigFileRefObjImp( ref_ptr<Core::StoredConfigFile> rcfile );
	  virtual Bscript::BObjectRef OperSubscript( const Bscript::BObject& obj );
	  virtual const char* typeOf() const;
	  virtual int typeOfInt() const;
	  virtual Bscript::BObjectImp* copy() const;
	};



	typedef Bscript::BApplicObj< ref_ptr<Core::StoredConfigElem> > EConfigElemRefObjImpBase;

	class EConfigElemRefObjImp : public EConfigElemRefObjImpBase
	{
	public:
	  EConfigElemRefObjImp( ref_ptr<Core::StoredConfigElem> rcelem );
	  virtual Bscript::BObjectRef get_member( const char* membername );
	  virtual Bscript::BObjectRef get_member_id( const int id ); //id test
	  virtual const char* typeOf() const;
	  virtual int typeOfInt() const;
	  virtual Bscript::BObjectImp* copy() const;
	};



	bool getStoredConfigFileParam( Bscript::ExecutorModule& exmod,
								   unsigned param,
								   Core::StoredConfigFile*& cfile );
	bool getStoredConfigElemParam( Bscript::ExecutorModule& exmod,
								   unsigned param,
								   Core::StoredConfigElem*& celem );
  }
}
#endif
