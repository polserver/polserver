/*
History
=======
2005/07/04 Shinigami: mf_AppendConfigFileElem will reload file
2006/09/26 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule

Notes
=======

*/

#include "../../clib/stl_inc.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4786 )
#endif


#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/dict.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/executor.h"
#include "../../bscript/escrutil.h"
#include "../../bscript/impstr.h"
#include "../../bscript/objmembers.h"

#include "../../clib/rawtypes.h"
#include "../../clib/strutil.h"

#include "cfgmod.h"
#include "../../plib/pkg.h"

#include "../cfgrepos.h"

BApplicObjType cfgfile_type;
BApplicObjType cfgelem_type;

template<>
TmplExecutorModule<ConfigFileExecutorModule>::FunctionDef   
TmplExecutorModule<ConfigFileExecutorModule>::function_table[] = 
{
	{ "ReadConfigFile",		 &ConfigFileExecutorModule::mf_ConfigFile },
	{ "FindConfigElem",		 &ConfigFileExecutorModule::mf_FindConfigElement },
	{ "GetElemProperty",		&ConfigFileExecutorModule::mf_GetConfigString },
	{ "GetConfigString",		&ConfigFileExecutorModule::mf_GetConfigString },
	{ "GetConfigStringArray",   &ConfigFileExecutorModule::mf_GetConfigStringArray },
	{ "GetConfigStringDictionary",   &ConfigFileExecutorModule::mf_GetConfigStringDictionary },
	{ "GetConfigInt",		   &ConfigFileExecutorModule::mf_GetConfigInt },
	{ "GetConfigIntArray",		&ConfigFileExecutorModule::mf_GetConfigIntArray },
	{ "GetConfigReal",		  &ConfigFileExecutorModule::mf_GetConfigReal },
	{ "GetConfigMaxIntKey",	 &ConfigFileExecutorModule::mf_GetConfigMaxIntKey },
	{ "GetConfigStringKeys",	&ConfigFileExecutorModule::mf_GetConfigStringKeys },
	{ "GetConfigIntKeys",	   &ConfigFileExecutorModule::mf_GetConfigIntKeys },
	{ "ListConfigElemProps",	&ConfigFileExecutorModule::mf_ListConfigElemProps },
	{ "AppendConfigFileElem",   &ConfigFileExecutorModule::mf_AppendConfigFileElem },
	{ "UnloadConfigFile",	   &ConfigFileExecutorModule::mf_UnloadConfigFile },
	{ "LoadTusScpFile",		 &ConfigFileExecutorModule::mf_LoadTusScpFile }
};

template<>
int TmplExecutorModule<ConfigFileExecutorModule>::function_table_size =
arsize(function_table);

EConfigFileRefObjImp::EConfigFileRefObjImp( ref_ptr<StoredConfigFile> rcfile ) :
	EConfigFileRefObjImpBase( &cfgfile_type, rcfile )
{
}

BObjectRef EConfigFileRefObjImp::OperSubscript( const BObject& obj )
{
	const BObjectImp& imp = obj.impref();
	ref_ptr<StoredConfigElem> celem;

	if (imp.isa( OTString ))
	{
		const char* strval = static_cast<const String*>(&imp)->data();
		celem = obj_->findelem( strval );
	}
	else if (imp.isa( OTLong ))
	{
		int key = static_cast<const BLong*>(&imp)->value();
		celem = obj_->findelem( key );
	}

	if (celem.get() != NULL)
	{
		return BObjectRef(new EConfigElemRefObjImp( celem ));
	}
	else
	{
		return BObjectRef(new BError( "Element not found" ));
	}
}

const char* EConfigFileRefObjImp::typeOf() const
{
	return "ConfigFileRef";
}
int EConfigFileRefObjImp::typeOfInt() const
{
	return OTConfigFileRef;
}
BObjectImp* EConfigFileRefObjImp::copy() const
{
	return new EConfigFileRefObjImp( obj_ );
}

EConfigElemRefObjImp::EConfigElemRefObjImp( ref_ptr<StoredConfigElem> rcelem ) :
	EConfigElemRefObjImpBase( &cfgelem_type, rcelem )
{
}

const char* EConfigElemRefObjImp::typeOf() const
{
	return "ConfigElemRef";
}
int EConfigElemRefObjImp::typeOfInt() const
{
	return OTConfigElemRef;
}
BObjectImp* EConfigElemRefObjImp::copy() const
{
	return new EConfigElemRefObjImp( obj_ );
}

BObjectRef EConfigElemRefObjImp::get_member_id( const int id ) //id test
{
	ObjMember* memb = getObjMember(id);

	return get_member(memb->code);
}
BObjectRef EConfigElemRefObjImp::get_member( const char* membername )
{

	BObjectImp* imp = obj_->getimp( membername );
	if (imp != NULL)
		return BObjectRef(imp);

	return BObjectRef(new UninitObject);
}

bool ConfigFileExecutorModule::get_cfgfilename( const string& cfgdesc, 
												string* cfgfile,
												string* errmsg,
												string* allpkgbase )
{

	if (allpkgbase)
		*allpkgbase = "";
	const Package* pkg = exec.prog()->pkg;

	if (cfgdesc[0] == ':')
	{
		if (cfgdesc[1] == ':') 
		{
			// Austin 2005-11-15
			// This is a bit of a kludge to make /regions/ config files readable.
			// Will go away when the paths here work like they do for file.em.
			// Looks for ::regions/filename
			if( cfgdesc.substr(2, 8) == "regions/" )
			{
				*cfgfile = cfgdesc.substr(2, string::npos) + ".cfg";
				return true;
			}
			else
			{
				// "::cfgfile" - core config file
				*cfgfile = "config/" + cfgdesc.substr( 2, string::npos ) + ".cfg"; 
				return true;
			}
		}
		else	// ":pkgname:configfile" - config file in some package
		{
			string::size_type second_colon = cfgdesc.find( ':', 2 );
			if (second_colon != string::npos)
			{
				string pkgname = cfgdesc.substr( 1, second_colon-1  );
				string cfgbase = cfgdesc.substr( second_colon+1, string::npos );
				
				if (pkgname == "*")
				{
					if (allpkgbase)
					{
						*cfgfile = cfgdesc;
						*allpkgbase = cfgbase;
						return true;
					}
					else
					{
						return false;
					}
				}

				Package* dstpkg = find_package( pkgname );
				if (dstpkg != NULL)
				{
					*cfgfile = GetPackageCfgPath(dstpkg, cfgbase+".cfg");
					return true;
				}
				else
				{
					*errmsg = "Unable to find package " + pkgname;   
					return false;
				}
			}
			else
			{
				*errmsg = "Poorly formed config file descriptor: " + cfgdesc;
				return false;
			}

		}
	}
	else
	{
		if (pkg != NULL)
		{
			*cfgfile = GetPackageCfgPath(const_cast<Package*>(pkg), cfgdesc+".cfg");
			return true;
		}
		else
		{
			*cfgfile = "config/" + cfgdesc + ".cfg";
			return true;
		}
	}
}


BObjectImp* ConfigFileExecutorModule::mf_ConfigFile()
{ 
	const String* cfgdesc_str;
	if (exec.getStringParam( 0, cfgdesc_str ))
	{
		const std::string& cfgdesc = cfgdesc_str->value();
		string cfgfile;
		string errmsg;
		string allpkgbase;
		if (!get_cfgfilename( cfgdesc, &cfgfile, &errmsg, &allpkgbase ))
		{
			return new BError( errmsg );
		}

		ref_ptr<StoredConfigFile> cfile = FindConfigFile( cfgfile, allpkgbase );
		
		if (cfile.get() != NULL)
		{
			return new EConfigFileRefObjImp( cfile );
		}
		else
		{
			return new BError( "Config file not found" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

bool legal_scp_filename( const char* filename )
{
	while (filename && *filename)
	{
		if (!isalnum( *filename ))
			return false;
		++filename;
	}
	return true;
}

BObjectImp* ConfigFileExecutorModule::mf_LoadTusScpFile()
{ 
	const String* filename_str;
	if (!exec.getStringParam( 0, filename_str ))
	{
		return new BError( "Invalid parameter type" );
	}

	if (!legal_scp_filename( filename_str->data() ))
	{
		return new BError( "Filename cannot include path information or special characters" );
	}

	ref_ptr<StoredConfigFile> cfile = LoadTusScpFile( "import/tus/" + filename_str->value() + ".scp" );
	
	if (cfile.get() == NULL)
	{
		return new BError( "File not found" );
	}

	return new EConfigFileRefObjImp( cfile );
}

BObjectImp* ConfigFileExecutorModule::mf_GetConfigMaxIntKey()
{
	StoredConfigFile* cfile;
	
	if (getStoredConfigFileParam( *this, 0, cfile ))
	{
		return new BLong( cfile->maxintkey() );
	}
	else
	{
		return new BError( "Parameter 0 must be a Config File" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_GetConfigStringKeys()
{
	StoredConfigFile* cfile;
	if (getStoredConfigFileParam( *this, 0, cfile ))
	{
		std::unique_ptr<ObjArray> arr (new ObjArray);
		StoredConfigFile::ElementsByName::const_iterator itr = cfile->byname_begin(), end = cfile->byname_end();
		for( ; itr != end; ++itr )
		{
			arr->addElement( new String( (*itr).first.c_str() ) );
		}
		return arr.release();
	}
	else
	{
		return new BError( "GetConfigStringKeys param 0 must be a Config File" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_GetConfigIntKeys()
{
	StoredConfigFile* cfile;
	if (getStoredConfigFileParam( *this, 0, cfile ))
	{
		std::unique_ptr<ObjArray> arr (new ObjArray);
		StoredConfigFile::ElementsByNum::const_iterator itr = cfile->bynum_begin(), end = cfile->bynum_end();
		for( ; itr != end; ++itr )
		{
			arr->addElement( new BLong( (*itr).first ) );
		}
		return arr.release();
	}
	else
	{
		return new BError( "GetConfigIntKeys param 0 must be a Config File" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_FindConfigElement()
{
	StoredConfigFile* cfile;
	
	if (getStoredConfigFileParam( *this, 0, cfile ))
	{
		BObjectImp* keyimp = exec.getParamImp( 1 );
		
		ref_ptr<StoredConfigElem> celem;

		if (keyimp->isa(BObjectImp::OTLong))
		{
			int key = static_cast<BLong*>(keyimp)->value();
			celem = cfile->findelem( key );
		}
		else if (keyimp->isa(BObjectImp::OTString))
		{
			const char* strval = static_cast<String*>(keyimp)->data();
			celem = cfile->findelem( strval );
		}
		else
		{
			return new BError( "Param 1 must be an Integer or a String" );
		}

		if (celem.get() != NULL)
		{
			return new EConfigElemRefObjImp( celem );
		}
		else
		{
			return new BError( "Element not found" );
		}
	}
	else
	{
		return new BError( "Parameter 0 must be a Config File" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_GetConfigString()
{ 
	StoredConfigElem* celem;
	const String* propname_str;

	if (getStoredConfigElemParam( *this, 0, celem ) &&
		getStringParam( 1, propname_str ))
	{
		BObjectImp* imp = celem->getimp( propname_str->value() );
		if (imp != NULL)
		{
			return new String( imp->getStringRep() );
		}
		else
		{
			return new BError( "Property not found" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_GetConfigStringArray()
{ 
	StoredConfigElem* celem;
	const String* propname_str;

	if (getStoredConfigElemParam( *this, 0, celem ) &&
		getStringParam( 1, propname_str ))
	{
		pair<StoredConfigElem::const_iterator,StoredConfigElem::const_iterator> pr = celem->equal_range( propname_str->data() );
		StoredConfigElem::const_iterator itr = pr.first;
		StoredConfigElem::const_iterator end = pr.second;

		std::unique_ptr<ObjArray> ar (new ObjArray);
		for( ; itr != end; ++itr )
		{
			BObjectImp* imp = (*itr).second.get();
			//Added 9-03-2005  Austin
			//Will no longer place the string right into the array.
			//Instead a check is done to make sure something is there.
			if ( imp->getStringRep().length() >= 1 )
				ar->addElement( new String(imp->getStringRep()) );
		}
		return ar.release();
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_GetConfigStringDictionary()
{ 
	StoredConfigElem* celem;
	const String* propname_str;

	if (getStoredConfigElemParam( *this, 0, celem ) && getStringParam( 1, propname_str ))
	{
		pair<StoredConfigElem::const_iterator,StoredConfigElem::const_iterator> pr = celem->equal_range( propname_str->data() );
		StoredConfigElem::const_iterator itr = pr.first;
		StoredConfigElem::const_iterator end = pr.second;
		
		std::unique_ptr<BDictionary> dict ( new BDictionary );
		for( ; itr != end; ++itr )
		{
			BObjectImp* line = (*itr).second.get();

			string line_str = line->getStringRep();
			if ( line_str.length() < 1 )
				continue;

			/* Example:
			 * Elem
			 * {
			 *     PropName data moredata more data
			 *     PropName stuff morestuff stuffity stuff
			 * }
			 * dict "data"->"moredata more data", "stuff"->"morestuff stuffity stuff!"
			 */
			string prop_name, prop_value;
			splitnamevalue(line_str, prop_name, prop_value);

			dict->addMember(new String(prop_name), new String(prop_value));
		}
		
		return dict.release();
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}


BObjectImp* ConfigFileExecutorModule::mf_GetConfigInt()
{ 
	StoredConfigElem* celem;
	const String* propname_str;

	if (getStoredConfigElemParam( *this, 0, celem ) &&
		getStringParam( 1, propname_str ))
	{
		BObjectImp* imp = celem->getimp( propname_str->value() );
		if (imp != NULL)
		{
			if (imp->isa( BObjectImp::OTLong ))
			{
				return imp;
			}
			else if (imp->isa( BObjectImp::OTDouble ))
			{
				Double* dbl = static_cast<Double*>(imp);
				return new BLong( static_cast<int>(dbl->value()) );
			}
			else if (imp->isa( BObjectImp::OTString ))
			{
				String* str = static_cast<String*>(imp);
				return new BLong( strtoul( str->data(), NULL, 0 ) );
			}
			else
			{
				return new BError( "Invalid type in config file! (internal error)" );
			}
		}
		else
		{
			return new BError( "Property not found" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_GetConfigIntArray()
{ 
	StoredConfigElem* celem;
	const String* propname_str;

	if (getStoredConfigElemParam( *this, 0, celem ) &&
		getStringParam( 1, propname_str ))
	{
		pair<StoredConfigElem::const_iterator,StoredConfigElem::const_iterator> pr = celem->equal_range( propname_str->data() );
		StoredConfigElem::const_iterator itr = pr.first;
		StoredConfigElem::const_iterator end = pr.second;

		std::unique_ptr<ObjArray> ar (new ObjArray);
		for( ; itr != end; ++itr )
		{
			BObjectImp* imp = (*itr).second.get();
			//Will no longer place the string right into the array.
			//Instead a check is done to make sure something is there.

			if ( imp->getStringRep().length() >= 1 )
			{
				if (imp->isa( BObjectImp::OTLong ))
				{
					ar->addElement( imp );
				}
				else if (imp->isa( BObjectImp::OTDouble ))
				{
					Double* dbl = static_cast<Double*>(imp);
					ar->addElement( new  BLong( static_cast<int>(dbl->value()) ));
				}
				else if (imp->isa( BObjectImp::OTString ))
				{
					String* str = static_cast<String*>(imp);
					ar->addElement( new  BLong( strtoul( str->data(), NULL, 0 ) ));
				}

			}
		}
		return ar.release();
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_GetConfigReal()
{ 
	StoredConfigElem* celem;
	const String* propname_str;

	if (getStoredConfigElemParam( *this, 0, celem ) &&
		getStringParam( 1, propname_str ))
	{
		BObjectImp* imp = celem->getimp( propname_str->value() );
		if (imp != NULL)
		{
			if (imp->isa( BObjectImp::OTDouble ))
			{
				return imp;
			}
			else if (imp->isa( BObjectImp::OTLong ))
			{
				BLong* blong = static_cast<BLong*>(imp);
				return new Double( blong->value() );
			}
			else if (imp->isa( BObjectImp::OTString ))
			{
				String* str = static_cast<String*>(imp);
				return new Double( strtod( str->data(), NULL ) );
			}
			else
			{
				return new BError( "Invalid type in config file! (internal error)" );
			}
		}
		else
		{
			return new BError( "Property not found" );
		}
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

BObjectImp* ConfigFileExecutorModule::mf_ListConfigElemProps()
{
	StoredConfigElem* celem;
	if (getStoredConfigElemParam( *this, 0, celem ))
	{
		// should return an array or prop-names
		return celem->listprops();
	}
	else
	{
		return new BError( "Invalid parameter type" );
	}
}

/* The elements in the array passed should each be a structure (name, value) */
BObjectImp* ConfigFileExecutorModule::mf_AppendConfigFileElem()
{
	const String* filename;
	const String* elemtype;
	ObjArray* objarr;
	if (!exec.getStringParam( 0, filename ) || 
		!exec.getStringParam( 1, elemtype ) ||
		!exec.getObjArrayParam( 3, objarr )  )
	{
		return new BError( "Invalid parameter type" );
	}
	string elemkey = getParamImp( 2 )->getStringRep();

	string pathname, errmsg;
	const std::string& cfgdesc = filename->value();
	if (!get_cfgfilename( cfgdesc, &pathname, &errmsg ))
	{
		return new BError( errmsg );
	}

	ofstream ofs( pathname.c_str(), ios::app );
	ofs << endl 
		<< elemtype->value() << " " << elemkey << endl
		<< "{" << endl;
	for( ObjArray::const_iterator itr = objarr->ref_arr.begin(), end = objarr->ref_arr.end(); itr != end; ++itr )
	{
		BObject *bo = itr->get();
		if (bo != NULL)
		{
			if (bo->isa( BObjectImp::OTArray ))
			{
				ObjArray* inarr = (ObjArray*)(bo->impptr());
				if (inarr->ref_arr.size() == 2)
				{
					BObject* nobj = inarr->ref_arr[0].get();
					BObject* vobj = inarr->ref_arr[1].get();
					if (nobj != NULL && nobj->isa( BObjectImp::OTString ) && vobj != NULL)
					{
						String* namestr = static_cast<String*>(nobj->impptr());
						string value = vobj->impptr()->getStringRep();

						ofs << "\t" << namestr->value() << "\t" << value << endl;
					}
				}
			}
			else if (bo->isa( BObjectImp::OTStruct ))
			{
				BStruct* instruct = static_cast<BStruct*>(bo->impptr());
				const BObjectImp* name_imp = instruct->FindMember( "name" );
				const BObjectImp* value_imp = instruct->FindMember( "value" );
				if (name_imp && name_imp->isa( BObjectImp::OTString ) && value_imp)
				{
					const String* namestr = static_cast<const String*>(name_imp);
					string value = value_imp->getStringRep();

					ofs << "\t" << namestr->value() << "\t" << value << endl;
				}
			}
		}
	}
	ofs << "}" << endl;

	UnloadConfigFile( pathname );

	return new BLong(1);
}

BObjectImp* ConfigFileExecutorModule::mf_UnloadConfigFile()
{
	const String* filename;
	if (getStringParam( 0, filename ))
	{
		const std::string& cfgdesc = filename->value();
		string cfgfile;
		string errmsg;
		string allpkgbase;
		if (!get_cfgfilename( cfgdesc, &cfgfile, &errmsg, &allpkgbase ))
		{
			return new BError( errmsg );
		}

		return new BLong( UnloadConfigFile( cfgfile ) );
	}
	else
	{
		return new BError( "Invalid parameter" );
	}
}

bool getStoredConfigFileParam( ExecutorModule& exmod, 
							   unsigned param,
							   StoredConfigFile* &cfile )
{
	BApplicObjBase* ao_cfile_base = exmod.exec.getApplicObjParam( param, &cfgfile_type );
	if (ao_cfile_base != NULL)
	{
		EConfigFileRefObjImp* ao_cfile = static_cast<EConfigFileRefObjImp*>(ao_cfile_base);

		cfile = ao_cfile->value().get();

		return true;
	}
	else
	{
		return false;
	}
}

bool getStoredConfigElemParam( ExecutorModule& exmod,
							   unsigned param,
							   StoredConfigElem*& celem )
{
	BApplicObjBase* ao_celem_base = exmod.exec.getApplicObjParam( param, &cfgelem_type );
	if (ao_celem_base != NULL)
	{
		EConfigElemRefObjImp* ao_celem = static_cast<EConfigElemRefObjImp*>(ao_celem_base);
		
		celem = ao_celem->value().get();

		return true;
	}
	else
	{
		return false;
	}
}
