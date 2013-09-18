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

class BObject;

class ConfigFileExecutorModule : public TmplExecutorModule<ConfigFileExecutorModule>
{
public:
    ConfigFileExecutorModule( Executor& exec ) :
        TmplExecutorModule<ConfigFileExecutorModule>( "cfgfile", exec ) {};

    BObjectImp* mf_ConfigFile();
    BObjectImp* mf_FindConfigElement();
    BObjectImp* mf_GetElementProperty();
    BObjectImp* mf_GetConfigString();
    BObjectImp* mf_GetConfigStringArray();
	BObjectImp* mf_GetConfigStringDictionary();
    BObjectImp* mf_GetConfigInt();
	BObjectImp* mf_GetConfigIntArray();
    BObjectImp* mf_GetConfigReal();
    BObjectImp* mf_GetConfigMaxIntKey();
    BObjectImp* mf_GetConfigStringKeys();
    BObjectImp* mf_GetConfigIntKeys();
	BObjectImp* mf_ListConfigElemProps();

    BObjectImp* mf_AppendConfigFileElem();
    BObjectImp* mf_UnloadConfigFile();

    BObjectImp* mf_LoadTusScpFile();


protected:
    bool get_cfgfilename( const std::string& cfgdesc, 
                            std::string* cfgfile,
                            std::string* errmsg,
                            std::string* allpkgbase = NULL );


};



typedef BApplicObj< ref_ptr<StoredConfigFile> > EConfigFileRefObjImpBase;

class EConfigFileRefObjImp : public EConfigFileRefObjImpBase
{
public:
    EConfigFileRefObjImp( ref_ptr<StoredConfigFile> rcfile );
    virtual BObjectRef OperSubscript( const BObject& obj );
    virtual const char* typeOf() const;
	virtual int typeOfInt() const;
    virtual BObjectImp* copy() const;
};



typedef BApplicObj< ref_ptr<StoredConfigElem> > EConfigElemRefObjImpBase;

class EConfigElemRefObjImp : public EConfigElemRefObjImpBase
{
public:
    EConfigElemRefObjImp( ref_ptr<StoredConfigElem> rcelem );
    virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); //id test
    virtual const char* typeOf() const;
	virtual int typeOfInt() const;
    virtual BObjectImp* copy() const;
};



bool getStoredConfigFileParam( ExecutorModule& exmod, 
                               unsigned param,
                               StoredConfigFile*& cfile );
bool getStoredConfigElemParam( ExecutorModule& exmod,
                               unsigned param,
                               StoredConfigElem*& celem );

#endif
