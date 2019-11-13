/** @file
 *
 * @par History
 * - 2005/07/04 Shinigami: mf_AppendConfigFileElem will reload file
 * - 2006/09/26 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 */


#include "cfgmod.h"
#include <ctype.h>
#include <fstream>
#include <string>
#include <utility>

#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/bstruct.h"
#include "../../bscript/dict.h"
#include "../../bscript/execmodl.h"
#include "../../bscript/executor.h"
#include "../../bscript/impstr.h"
#include "../../bscript/objmembers.h"
#include "../../clib/rawtypes.h"
#include "../../clib/refptr.h"
#include "../../clib/strutil.h"
#include "../../plib/pkg.h"
#include "../cfgrepos.h"

#include <module_defs/cfgfile.h>

namespace Pol
{
namespace Module
{
}
namespace Module
{
Bscript::BApplicObjType cfgfile_type;
Bscript::BApplicObjType cfgelem_type;


EConfigFileRefObjImp::EConfigFileRefObjImp( ref_ptr<Core::StoredConfigFile> rcfile )
    : EConfigFileRefObjImpBase( &cfgfile_type, rcfile )
{
}

Bscript::BObjectRef EConfigFileRefObjImp::OperSubscript( const Bscript::BObject& obj )
{
  const Bscript::BObjectImp& imp = obj.impref();
  ref_ptr<Core::StoredConfigElem> celem;

  if ( imp.isa( OTString ) )
  {
    const char* strval = static_cast<const Bscript::String*>( &imp )->data();
    celem = obj_->findelem( strval );
  }
  else if ( imp.isa( OTLong ) )
  {
    int key = static_cast<const Bscript::BLong*>( &imp )->value();
    celem = obj_->findelem( key );
  }

  if ( celem.get() != nullptr )
  {
    return Bscript::BObjectRef( new EConfigElemRefObjImp( celem ) );
  }
  else
  {
    return Bscript::BObjectRef( new Bscript::BError( "Element not found" ) );
  }
}

const char* EConfigFileRefObjImp::typeOf() const
{
  return "ConfigFileRef";
}
u8 EConfigFileRefObjImp::typeOfInt() const
{
  return OTConfigFileRef;
}
Bscript::BObjectImp* EConfigFileRefObjImp::copy() const
{
  return new EConfigFileRefObjImp( obj_ );
}

EConfigElemRefObjImp::EConfigElemRefObjImp( ref_ptr<Core::StoredConfigElem> rcelem )
    : EConfigElemRefObjImpBase( &cfgelem_type, rcelem )
{
}

const char* EConfigElemRefObjImp::typeOf() const
{
  return "ConfigElemRef";
}
u8 EConfigElemRefObjImp::typeOfInt() const
{
  return OTConfigElemRef;
}
Bscript::BObjectImp* EConfigElemRefObjImp::copy() const
{
  return new EConfigElemRefObjImp( obj_ );
}

Bscript::BObjectRef EConfigElemRefObjImp::get_member_id( const int id )  // id test
{
  Bscript::ObjMember* memb = Bscript::getObjMember( id );

  return get_member( memb->code );
}
Bscript::BObjectRef EConfigElemRefObjImp::get_member( const char* membername )
{
  Bscript::BObjectImp* imp = obj_->getimp( membername );
  if ( imp != nullptr )
    return Bscript::BObjectRef( imp );

  return Bscript::BObjectRef( new Bscript::UninitObject );
}

ConfigFileExecutorModule::ConfigFileExecutorModule( Bscript::Executor& exec )
    : TmplExecutorModule<ConfigFileExecutorModule>( exec )
{
}

bool ConfigFileExecutorModule::get_cfgfilename( const std::string& cfgdesc, std::string* cfgfile,
                                                std::string* errmsg, std::string* allpkgbase )
{
  if ( allpkgbase )
    *allpkgbase = "";
  const Plib::Package* pkg = exec.prog()->pkg;

  if ( cfgdesc[0] == ':' )
  {
    if ( cfgdesc[1] == ':' )
    {
      // Austin 2005-11-15
      // This is a bit of a kludge to make /regions/ config files readable.
      // Will go away when the paths here work like they do for file.em.
      // Looks for ::regions/filename
      if ( cfgdesc.substr( 2, 8 ) == "regions/" )
      {
        *cfgfile = cfgdesc.substr( 2, std::string::npos ) + ".cfg";
        return true;
      }
      else
      {
        // "::cfgfile" - core config file
        *cfgfile = "config/" + cfgdesc.substr( 2, std::string::npos ) + ".cfg";
        return true;
      }
    }
    else  // ":pkgname:configfile" - config file in some package
    {
      std::string::size_type second_colon = cfgdesc.find( ':', 2 );
      if ( second_colon != std::string::npos )
      {
        std::string pkgname = cfgdesc.substr( 1, second_colon - 1 );
        std::string cfgbase = cfgdesc.substr( second_colon + 1, std::string::npos );

        if ( pkgname == "*" )
        {
          if ( allpkgbase )
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

        Plib::Package* dstpkg = Plib::find_package( pkgname );
        if ( dstpkg != nullptr )
        {
          *cfgfile = GetPackageCfgPath( dstpkg, cfgbase + ".cfg" );
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
    if ( pkg != nullptr )
    {
      *cfgfile = GetPackageCfgPath( const_cast<Plib::Package*>( pkg ), cfgdesc + ".cfg" );
      return true;
    }
    else
    {
      *cfgfile = "config/" + cfgdesc + ".cfg";
      return true;
    }
  }
}


Bscript::BObjectImp* ConfigFileExecutorModule::mf_ReadConfigFile()
{
  const Bscript::String* cfgdesc_str;
  if ( exec.getStringParam( 0, cfgdesc_str ) )
  {
    const std::string& cfgdesc = cfgdesc_str->value();
    std::string cfgfile;
    std::string errmsg;
    std::string allpkgbase;
    if ( !get_cfgfilename( cfgdesc, &cfgfile, &errmsg, &allpkgbase ) )
    {
      return new Bscript::BError( errmsg );
    }

    ref_ptr<Core::StoredConfigFile> cfile = Core::FindConfigFile( cfgfile, allpkgbase );

    if ( cfile.get() != nullptr )
    {
      return new EConfigFileRefObjImp( cfile );
    }
    else
    {
      return new Bscript::BError( "Config file not found" );
    }
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

bool legal_scp_filename( const char* filename )
{
  while ( filename && *filename )
  {
    if ( !isalnum( *filename ) )
      return false;
    ++filename;
  }
  return true;
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_LoadTusScpFile()
{
  const Bscript::String* filename_str;
  if ( !exec.getStringParam( 0, filename_str ) )
  {
    return new Bscript::BError( "Invalid parameter type" );
  }

  if ( !legal_scp_filename( filename_str->data() ) )
  {
    return new Bscript::BError( "Filename cannot include path information or special characters" );
  }

  ref_ptr<Core::StoredConfigFile> cfile =
      Core::LoadTusScpFile( "import/tus/" + filename_str->value() + ".scp" );

  if ( cfile.get() == nullptr )
  {
    return new Bscript::BError( "File not found" );
  }

  return new EConfigFileRefObjImp( cfile );
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigMaxIntKey()
{
  Core::StoredConfigFile* cfile;

  if ( getStoredConfigFileParam( *this, 0, cfile ) )
  {
    return new Bscript::BLong( cfile->maxintkey() );
  }
  else
  {
    return new Bscript::BError( "Parameter 0 must be a Config File" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigStringKeys()
{
  Core::StoredConfigFile* cfile;
  if ( getStoredConfigFileParam( *this, 0, cfile ) )
  {
    std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
    Core::StoredConfigFile::ElementsByName::const_iterator itr = cfile->byname_begin(),
                                                           end = cfile->byname_end();
    for ( ; itr != end; ++itr )
    {
      arr->addElement( new Bscript::String( ( *itr ).first.c_str() ) );
    }
    return arr.release();
  }
  else
  {
    return new Bscript::BError( "GetConfigStringKeys param 0 must be a Config File" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigIntKeys()
{
  Core::StoredConfigFile* cfile;
  if ( getStoredConfigFileParam( *this, 0, cfile ) )
  {
    std::unique_ptr<Bscript::ObjArray> arr( new Bscript::ObjArray );
    Core::StoredConfigFile::ElementsByNum::const_iterator itr = cfile->bynum_begin(),
                                                          end = cfile->bynum_end();
    for ( ; itr != end; ++itr )
    {
      arr->addElement( new Bscript::BLong( ( *itr ).first ) );
    }
    return arr.release();
  }
  else
  {
    return new Bscript::BError( "GetConfigIntKeys param 0 must be a Config File" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_FindConfigElem()
{
  Core::StoredConfigFile* cfile;

  if ( getStoredConfigFileParam( *this, 0, cfile ) )
  {
    Bscript::BObjectImp* keyimp = exec.getParamImp( 1 );

    ref_ptr<Core::StoredConfigElem> celem;

    if ( keyimp->isa( Bscript::BObjectImp::OTLong ) )
    {
      int key = static_cast<Bscript::BLong*>( keyimp )->value();
      celem = cfile->findelem( key );
    }
    else if ( keyimp->isa( Bscript::BObjectImp::OTString ) )
    {
      const char* strval = static_cast<Bscript::String*>( keyimp )->data();
      celem = cfile->findelem( strval );
    }
    else
    {
      return new Bscript::BError( "Param 1 must be an Integer or a String" );
    }

    if ( celem.get() != nullptr )
    {
      return new EConfigElemRefObjImp( celem );
    }
    else
    {
      return new Bscript::BError( "Element not found" );
    }
  }
  else
  {
    return new Bscript::BError( "Parameter 0 must be a Config File" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetElemProperty()
{
  return mf_GetConfigString();
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigString()
{
  Core::StoredConfigElem* celem;
  const Bscript::String* propname_str;

  if ( getStoredConfigElemParam( *this, 0, celem ) && getStringParam( 1, propname_str ) )
  {
    Bscript::BObjectImp* imp = celem->getimp( propname_str->value() );
    if ( imp != nullptr )
    {
      return new Bscript::String( imp->getStringRep() );
    }
    else
    {
      return new Bscript::BError( "Property not found" );
    }
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigStringArray()
{
  Core::StoredConfigElem* celem;
  const Bscript::String* propname_str;

  if ( getStoredConfigElemParam( *this, 0, celem ) && getStringParam( 1, propname_str ) )
  {
    std::pair<Core::StoredConfigElem::const_iterator, Core::StoredConfigElem::const_iterator> pr =
        celem->equal_range( propname_str->data() );
    Core::StoredConfigElem::const_iterator itr = pr.first;
    Core::StoredConfigElem::const_iterator end = pr.second;

    std::unique_ptr<Bscript::ObjArray> ar( new Bscript::ObjArray );
    for ( ; itr != end; ++itr )
    {
      Bscript::BObjectImp* imp = ( *itr ).second.get();
      // Added 9-03-2005  Austin
      // Will no longer place the string right into the array.
      // Instead a check is done to make sure something is there.
      if ( imp->getStringRep().length() >= 1 )
        ar->addElement( new Bscript::String( imp->getStringRep() ) );
    }
    return ar.release();
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigStringDictionary()
{
  Core::StoredConfigElem* celem;
  const Bscript::String* propname_str;

  if ( getStoredConfigElemParam( *this, 0, celem ) && getStringParam( 1, propname_str ) )
  {
    std::pair<Core::StoredConfigElem::const_iterator, Core::StoredConfigElem::const_iterator> pr =
        celem->equal_range( propname_str->data() );
    Core::StoredConfigElem::const_iterator itr = pr.first;
    Core::StoredConfigElem::const_iterator end = pr.second;

    std::unique_ptr<Bscript::BDictionary> dict( new Bscript::BDictionary );
    for ( ; itr != end; ++itr )
    {
      Bscript::BObjectImp* line = ( *itr ).second.get();

      std::string line_str = line->getStringRep();
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
      std::string prop_name, prop_value;
      Clib::splitnamevalue( line_str, prop_name, prop_value );

      dict->addMember( new Bscript::String( prop_name ), new Bscript::String( prop_value ) );
    }

    return dict.release();
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}


Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigInt()
{
  Core::StoredConfigElem* celem;
  const Bscript::String* propname_str;

  if ( getStoredConfigElemParam( *this, 0, celem ) && getStringParam( 1, propname_str ) )
  {
    Bscript::BObjectImp* imp = celem->getimp( propname_str->value() );
    if ( imp != nullptr )
    {
      if ( imp->isa( Bscript::BObjectImp::OTLong ) )
      {
        return imp;
      }
      else if ( imp->isa( Bscript::BObjectImp::OTDouble ) )
      {
        Bscript::Double* dbl = static_cast<Bscript::Double*>( imp );
        return new Bscript::BLong( static_cast<int>( dbl->value() ) );
      }
      else if ( imp->isa( Bscript::BObjectImp::OTString ) )
      {
        Bscript::String* str = static_cast<Bscript::String*>( imp );
        return new Bscript::BLong( strtoul( str->data(), nullptr, 0 ) );
      }
      else
      {
        return new Bscript::BError( "Invalid type in config file! (internal error)" );
      }
    }
    else
    {
      return new Bscript::BError( "Property not found" );
    }
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigIntArray()
{
  Core::StoredConfigElem* celem;
  const Bscript::String* propname_str;

  if ( getStoredConfigElemParam( *this, 0, celem ) && getStringParam( 1, propname_str ) )
  {
    auto pr = celem->equal_range( propname_str->data() );
    Core::StoredConfigElem::const_iterator itr = pr.first;
    Core::StoredConfigElem::const_iterator end = pr.second;

    std::unique_ptr<Bscript::ObjArray> ar( new Bscript::ObjArray );
    for ( ; itr != end; ++itr )
    {
      Bscript::BObjectImp* imp = ( *itr ).second.get();
      // Will no longer place the string right into the array.
      // Instead a check is done to make sure something is there.

      if ( imp->getStringRep().length() >= 1 )
      {
        if ( imp->isa( Bscript::BObjectImp::OTLong ) )
        {
          ar->addElement( imp );
        }
        else if ( imp->isa( Bscript::BObjectImp::OTDouble ) )
        {
          Bscript::Double* dbl = static_cast<Bscript::Double*>( imp );
          ar->addElement( new Bscript::BLong( static_cast<int>( dbl->value() ) ) );
        }
        else if ( imp->isa( Bscript::BObjectImp::OTString ) )
        {
          Bscript::String* str = static_cast<Bscript::String*>( imp );
          ar->addElement( new Bscript::BLong( strtoul( str->data(), nullptr, 0 ) ) );
        }
      }
    }
    return ar.release();
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_GetConfigReal()
{
  Core::StoredConfigElem* celem;
  const Bscript::String* propname_str;

  if ( getStoredConfigElemParam( *this, 0, celem ) && getStringParam( 1, propname_str ) )
  {
    Bscript::BObjectImp* imp = celem->getimp( propname_str->value() );
    if ( imp != nullptr )
    {
      if ( imp->isa( Bscript::BObjectImp::OTDouble ) )
      {
        return imp;
      }
      else if ( imp->isa( Bscript::BObjectImp::OTLong ) )
      {
        Bscript::BLong* blong = static_cast<Bscript::BLong*>( imp );
        return new Bscript::Double( blong->value() );
      }
      else if ( imp->isa( Bscript::BObjectImp::OTString ) )
      {
        Bscript::String* str = static_cast<Bscript::String*>( imp );
        return new Bscript::Double( strtod( str->data(), nullptr ) );
      }
      else
      {
        return new Bscript::BError( "Invalid type in config file! (internal error)" );
      }
    }
    else
    {
      return new Bscript::BError( "Property not found" );
    }
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_ListConfigElemProps()
{
  Core::StoredConfigElem* celem;
  if ( getStoredConfigElemParam( *this, 0, celem ) )
  {
    // should return an array or prop-names
    return celem->listprops();
  }
  else
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
}

/* The elements in the array passed should each be a structure (name, value) */
Bscript::BObjectImp* ConfigFileExecutorModule::mf_AppendConfigFileElem()
{
  const Bscript::String* filename;
  const Bscript::String* elemtype;
  Bscript::ObjArray* objarr;
  if ( !exec.getStringParam( 0, filename ) || !exec.getStringParam( 1, elemtype ) ||
       !exec.getObjArrayParam( 3, objarr ) )
  {
    return new Bscript::BError( "Invalid parameter type" );
  }
  std::string elemkey = getParamImp( 2 )->getStringRep();

  std::string pathname, errmsg;
  const std::string& cfgdesc = filename->value();
  if ( !get_cfgfilename( cfgdesc, &pathname, &errmsg ) )
  {
    return new Bscript::BError( errmsg );
  }

  std::ofstream ofs( pathname.c_str(), std::ios::app );
  ofs << std::endl << elemtype->value() << " " << elemkey << std::endl << "{" << std::endl;
  for ( Bscript::ObjArray::const_iterator itr = objarr->ref_arr.begin(),
                                          end = objarr->ref_arr.end();
        itr != end; ++itr )
  {
    Bscript::BObject* bo = itr->get();
    if ( bo != nullptr )
    {
      if ( bo->isa( Bscript::BObjectImp::OTArray ) )
      {
        Bscript::ObjArray* inarr = (Bscript::ObjArray*)( bo->impptr() );
        if ( inarr->ref_arr.size() == 2 )
        {
          Bscript::BObject* nobj = inarr->ref_arr[0].get();
          Bscript::BObject* vobj = inarr->ref_arr[1].get();
          if ( nobj != nullptr && nobj->isa( Bscript::BObjectImp::OTString ) && vobj != nullptr )
          {
            Bscript::String* namestr = static_cast<Bscript::String*>( nobj->impptr() );
            std::string value = vobj->impptr()->getStringRep();

            ofs << "\t" << namestr->value() << "\t" << value << std::endl;
          }
        }
      }
      else if ( bo->isa( Bscript::BObjectImp::OTStruct ) )
      {
        Bscript::BStruct* instruct = static_cast<Bscript::BStruct*>( bo->impptr() );
        const Bscript::BObjectImp* name_imp = instruct->FindMember( "name" );
        const Bscript::BObjectImp* value_imp = instruct->FindMember( "value" );
        if ( name_imp && name_imp->isa( Bscript::BObjectImp::OTString ) && value_imp )
        {
          const Bscript::String* namestr = static_cast<const Bscript::String*>( name_imp );
          std::string value = value_imp->getStringRep();

          ofs << "\t" << namestr->value() << "\t" << value << std::endl;
        }
      }
    }
  }
  ofs << "}" << std::endl;

  Core::UnloadConfigFile( pathname );

  return new Bscript::BLong( 1 );
}

Bscript::BObjectImp* ConfigFileExecutorModule::mf_UnloadConfigFile()
{
  const Bscript::String* filename;
  if ( getStringParam( 0, filename ) )
  {
    const std::string& cfgdesc = filename->value();
    std::string cfgfile;
    std::string errmsg;
    std::string allpkgbase;
    if ( !get_cfgfilename( cfgdesc, &cfgfile, &errmsg, &allpkgbase ) )
    {
      return new Bscript::BError( errmsg );
    }

    return new Bscript::BLong( Core::UnloadConfigFile( cfgfile ) );
  }
  else
  {
    return new Bscript::BError( "Invalid parameter" );
  }
}

bool getStoredConfigFileParam( Bscript::ExecutorModule& exmod, unsigned param,
                               Core::StoredConfigFile*& cfile )
{
  Bscript::BApplicObjBase* ao_cfile_base = exmod.exec.getApplicObjParam( param, &cfgfile_type );
  if ( ao_cfile_base != nullptr )
  {
    EConfigFileRefObjImp* ao_cfile = static_cast<EConfigFileRefObjImp*>( ao_cfile_base );

    cfile = ao_cfile->value().get();

    return true;
  }
  else
  {
    return false;
  }
}

bool getStoredConfigElemParam( Bscript::ExecutorModule& exmod, unsigned param,
                               Core::StoredConfigElem*& celem )
{
  Bscript::BApplicObjBase* ao_celem_base = exmod.exec.getApplicObjParam( param, &cfgelem_type );
  if ( ao_celem_base != nullptr )
  {
    EConfigElemRefObjImp* ao_celem = static_cast<EConfigElemRefObjImp*>( ao_celem_base );

    celem = ao_celem->value().get();

    return true;
  }
  else
  {
    return false;
  }
}
}  // namespace Module
}  // namespace Pol
