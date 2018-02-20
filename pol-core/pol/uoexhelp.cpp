/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2009/12/02 Turley:    added config.max_tile_id - Tomi
 */


#include "uoexhelp.h"

#include <ctype.h>
#include <exception>
#include <stdlib.h>
#include <string>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../bscript/fmodule.h"
#include "../bscript/impstr.h"
#include "../clib/logfacility.h"
#include "../clib/strutil.h"
#include "../plib/systemstate.h"
#include "fnsearch.h"
#include "globals/network.h"
#include "globals/uvars.h"
#include "item/itemdesc.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "multi/multi.h"
#include "network/client.h"
#include "uobject.h"
#include "uoscrobj.h"
#include "vital.h"

namespace Pol
{
namespace Core
{
using namespace Bscript;
using namespace Module;

bool getCharacterOrClientParam( Executor& exec, unsigned param, Mobile::Character*& chrptr,
                                Network::Client*& clientptr )
{
  BObjectImp* imp = exec.getParamImp( param );
  if ( imp == NULL )
  {
    exec.setFunctionResult( new BError( "Missing parameter " + Clib::decint( param ) ) );
    return false;
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = Clib::explicit_cast<BApplicObjBase*, BObjectImp*>( imp );

    if ( ( aob != NULL ) && ( aob->object_type() == &echaracterrefobjimp_type ) )
    {
      ECharacterRefObjImp* chrref_imp =
          Clib::explicit_cast<ECharacterRefObjImp*, BApplicObjBase*>( aob );

      chrptr = chrref_imp->value().get();

      if ( chrptr->orphan() )
      {
        exec.setFunctionResult( new BError( "Mobile has been destroyed" ) );
        return false;
      }

      if ( chrptr->logged_in() || chrref_imp->offline_access_ok() )
      {
        return true;
      }
      else
      {
        exec.setFunctionResult( new BError( "Mobile is offline" ) );
        return false;
      }
    }
    else if ( ( aob != NULL ) && ( aob->object_type() == &eclientrefobjimp_type ) )
    {
      EClientRefObjImp* clientref_imp =
          Clib::explicit_cast<EClientRefObjImp*, BApplicObjBase*>( aob );

      clientptr = clientref_imp->value().exists() ? clientref_imp->value().get_weakptr() : NULL;

      if ( ( clientptr != NULL ) && clientptr->isConnected() )
      {
        return true;
      }
      else
      {
        exec.setFunctionResult( new BError( "Client is disconnected" ) );
        return false;
      }
    }
    else
    {
      // FIXME: log error
      return false;
    }
  }
  else if ( imp->isa( BObjectImp::OTLong ) )
  {
    BLong* pchar_serial = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    unsigned int serial = pchar_serial->value();
    if ( IsItem( serial ) || serial == 0 )
    {
      exec.setFunctionResult( new BError( "Serial refers to an Item, or is zero" ) );
      return false;
    }

    chrptr = system_find_mobile( serial );

    if ( chrptr != NULL )
    {
      if ( chrptr->logged_in() )
      {
        return true;
      }
      else
      {
        exec.setFunctionResult( new BError( "Mobile is offline" ) );
        return false;
      }
    }
    else
    {
      exec.setFunctionResult( new BError( "Mobile does not exist" ) );
      return false;
    }
  }
  else
  {
    // FIXME: log error
    return false;
  }
}

bool getCharacterParam( Bscript::Executor& exec, unsigned param, Mobile::Character*& chrptr )
{
  BObjectImp* imp = exec.getParamImp( param );
  if ( imp == NULL )
  {
    exec.setFunctionResult( new BError( "Missing parameter " + Clib::decint( param ) ) );
    return false;
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = Clib::explicit_cast<BApplicObjBase*, BObjectImp*>( imp );

    if ( ( aob != NULL ) && ( aob->object_type() == &echaracterrefobjimp_type ) )
    {
      ECharacterRefObjImp* chrref_imp =
          Clib::explicit_cast<ECharacterRefObjImp*, BApplicObjBase*>( aob );

      chrptr = chrref_imp->value().get();

      if ( chrptr->orphan() )
      {
        exec.setFunctionResult( new BError( "Mobile has been destroyed" ) );
        return false;
      }

      if ( chrptr->logged_in() || chrref_imp->offline_access_ok() )
      {
        return true;
      }
      else
      {
        exec.setFunctionResult( new BError( "Mobile is offline" ) );
        return false;
      }
    }
    else
    {
      // FIXME: log error
      return false;
    }
  }
  else if ( imp->isa( BObjectImp::OTLong ) )
  {
    BLong* pchar_serial = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    unsigned int serial = pchar_serial->value();
    if ( IsItem( serial ) || serial == 0 )
    {
      exec.setFunctionResult( new BError( "Serial refers to an Item, or is zero" ) );
      return false;
    }

    chrptr = system_find_mobile( serial );

    if ( chrptr != NULL )
    {
      if ( chrptr->logged_in() )
      {
        return true;
      }
      else
      {
        exec.setFunctionResult( new BError( "Mobile is offline" ) );
        return false;
      }
    }
    else
    {
      exec.setFunctionResult( new BError( "Mobile does not exist" ) );
      return false;
    }
  }
  else
  {
    // FIXME: log error
    return false;
  }
}

bool getItemParam( Executor& exec, unsigned param, Items::Item*& itemptr )
{
  BObjectImp* imp = exec.getParamImp( param );
  if ( imp == NULL )
  {
    return false;
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = Clib::explicit_cast<BApplicObjBase*, BObjectImp*>( imp );

    if ( ( aob != NULL ) && ( aob->object_type() == &eitemrefobjimp_type ) )
    {
      EItemRefObjImp* itemref_imp = Clib::explicit_cast<EItemRefObjImp*, BApplicObjBase*>( aob );

      itemptr = itemref_imp->value().get();
      return ( !itemptr->orphan() );
    }
    else
    {
      // FIXME: log error
      return false;
    }
  }
  else if ( imp->isa( BObjectImp::OTLong ) )
  {
    BLong* pitem_serial = Clib::explicit_cast<BLong*, BObjectImp*>( imp );
    unsigned int serial = pitem_serial->value();

    if ( IsCharacter( serial ) || serial == 0 )
      return false;

    itemptr = system_find_item( serial );

    return ( itemptr != NULL );
  }
  else
  {
    // FIXME: log error
    return false;
  }
}

bool getUBoatParam( Executor& exec, unsigned param, Multi::UBoat*& boatptr )
{
  BObjectImp* imp = exec.getParamImp( param );
  if ( imp == NULL )
  {
    return false;
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = Clib::explicit_cast<BApplicObjBase*, BObjectImp*>( imp );

    if ( aob->object_type() == &euboatrefobjimp_type )
    {
      EUBoatRefObjImp* boatref_imp = Clib::explicit_cast<EUBoatRefObjImp*, BApplicObjBase*>( aob );

      boatptr = boatref_imp->value().get();
      return ( !boatptr->orphan() );
    }
    else if ( aob->object_type() == &eitemrefobjimp_type )
    {
      EItemRefObjImp* itemref_imp = Clib::explicit_cast<EItemRefObjImp*, BApplicObjBase*>( aob );

      Items::Item* item = itemref_imp->value().get();
      if ( item->isa( UOBJ_CLASS::CLASS_MULTI ) )
      {
        Multi::UMulti* multi = static_cast<Multi::UMulti*>( item );
        boatptr = multi->as_boat();
        if ( boatptr == NULL )
          return false;
        else
          return ( !boatptr->orphan() );
      }
      else
      {
        return false;
      }
    }
    else
    {
      // FIXME: log error
      return false;
    }
  }
  else if ( imp->isa( BObjectImp::OTLong ) )
  {
    BLong* pitem_serial = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    Multi::UMulti* multi = system_find_multi( pitem_serial->value() );
    if ( multi )
      boatptr = multi->as_boat();

    return ( boatptr != NULL );
  }
  else
  {
    // FIXME: log error
    return false;
  }
}


bool getMultiParam( Executor& exec, unsigned param, Multi::UMulti*& multiptr )
{
  BObjectImp* imp = exec.getParamImp( param );
  if ( imp == NULL )
  {
    return false;
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = Clib::explicit_cast<BApplicObjBase*, BObjectImp*>( imp );

    if ( aob->object_type() == &emultirefobjimp_type )
    {
      EMultiRefObjImp* multiref_imp = Clib::explicit_cast<EMultiRefObjImp*, BApplicObjBase*>( aob );

      multiptr = multiref_imp->value().get();
      return ( !multiptr->orphan() );
    }
    else if ( aob->object_type() == &euboatrefobjimp_type )
    {
      EUBoatRefObjImp* boatref_imp = Clib::explicit_cast<EUBoatRefObjImp*, BApplicObjBase*>( aob );

      multiptr = boatref_imp->value().get();
      return ( !multiptr->orphan() );
    }
    else
    {
      // FIXME: log error
      return false;
    }
  }
  else if ( imp->isa( BObjectImp::OTLong ) )
  {
    BLong* pitem_serial = Clib::explicit_cast<BLong*, BObjectImp*>( imp );

    multiptr = system_find_multi( pitem_serial->value() );

    return ( multiptr != NULL );
  }
  else
  {
    // FIXME: log error
    return false;
  }
}

bool getUObjectParam( Executor& exec, unsigned param, UObject*& objptr )
{
  Items::Item* item = NULL;
  Mobile::Character* chr = NULL;
  Multi::UMulti* multi = NULL;

  // This function is a kludge because the individual functions will all test for
  // a serial independently and may set errors.
  // TODO: Refactor this function to test whether the parameter is a BLong in a single place.

  if ( getCharacterParam( exec, param, chr ) )
  {
    objptr = chr;
    return true;
  }
  else if ( getItemParam( exec, param, item ) )
  {
    objptr = item;
    return true;
  }
  else if ( getMultiParam( exec, param, multi ) )
  {
    objptr = multi;
    return true;
  }
  else
  {
    return false;
  }
}

bool getObjtypeParam( Executor& exec, unsigned param, unsigned int& objtype )
{
  BObjectImp* imp = exec.getParamImp( param );
  if ( imp == NULL )
  {
    return false;
  }
  unsigned int objtype_long = 0;

  if ( imp->isa( BObjectImp::OTLong ) )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );
    objtype_long = plong->value();
  }
  else if ( imp->isa( BObjectImp::OTString ) )
  {
    // this could be an objtypename, or an objtype in string form.  Cope with either.
    String* pstring = Clib::explicit_cast<String*, BObjectImp*>( imp );
    const char* ot_str = pstring->data();
    if ( !isdigit( ot_str[0] ) )
    {
      objtype = Items::get_objtype_byname( pstring->data() );
      if ( objtype != 0 )
      {
        return true;
      }
      else
      {
        exec.setFunctionResult(
            new BError( std::string( "Objtype not defined: " ) + pstring->data() ) );

        return false;
      }
    }
    else
    {
      // a number passed...process below as if passed as a BLong
      objtype_long = strtol( ot_str, NULL, 0 );
    }
  }
  else
  {
    DEBUGLOG << "Script Error in '" << exec.scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function " << exec.current_module_function->name.get() << ":\n"
             << "\tParameter " << param << ": Expected Long or String, got datatype "
             << BObjectImp::typestr( imp->type() ) << "\n";
    return false;
  }

  if ( ( objtype_long > Plib::systemstate.config.max_tile_id ) &&
       ( objtype_long <= Plib::systemstate.config.max_objtype ) )
  {
    objtype = objtype_long;
    if ( Items::has_itemdesc( objtype ) )
    {
      return true;
    }
    else
    {
      exec.setFunctionResult(
          new BError( "Objtype " + Clib::hexint( objtype_long ) + " is not defined." ) );
      return false;
    }
  }
  else if ( objtype_long <= Plib::systemstate.config.max_tile_id )
  {
    objtype = objtype_long;
    return true;
  }
  else
  {
    DEBUGLOG << "Script Error in '" << exec.scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function " << exec.current_module_function->name.get() << ":\n"
             << "\tParameter " << param << ": Value " << objtype_long
             << " is out of range for an objtype\n";
    exec.setFunctionResult( new BError( "Objtype is out of range ( acceptable: 0 - " +
                                        Clib::hexint( Plib::systemstate.config.max_objtype ) +
                                        " )" ) );
    return false;
  }
}

bool getObjtypeParam( Executor& exec, unsigned param, const Items::ItemDesc*& itemdesc_out )
{
  BObjectImp* imp = exec.getParamImp( param );
  if ( imp == NULL )
  {
    return false;
  }
  unsigned int objtype_long = 0;

  if ( imp->isa( BObjectImp::OTLong ) )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );
    objtype_long = plong->value();
  }
  else if ( imp->isa( BObjectImp::OTString ) )
  {
    // this could be an objtypename, or an objtype in string form.  Cope with either.
    String* pstring = Clib::explicit_cast<String*, BObjectImp*>( imp );
    const char* ot_str = pstring->data();
    if ( !isdigit( ot_str[0] ) )
    {
      unsigned int objtype = Items::get_objtype_byname( pstring->data() );
      if ( objtype != 0 )
      {
        itemdesc_out = &Items::find_itemdesc( objtype );
        return true;
      }
      else
      {
        exec.setFunctionResult(
            new BError( std::string( "Objtype not defined: " ) + pstring->data() ) );

        return false;
      }
    }
    else
    {
      // a number passed...process below as if passed as a BLong
      objtype_long = strtol( ot_str, NULL, 0 );
    }
  }
  else if ( imp->isa( BObjectImp::OTStruct ) )
  {
    BStruct* itemdesc_struct = Clib::explicit_cast<BStruct*, BObjectImp*>( imp );
    try
    {
      itemdesc_out = Items::CreateItemDescriptor( itemdesc_struct );
      return true;
    }
    catch ( std::exception& ex )
    {
      std::string message = std::string( "Unable to create item descriptor: " ) + ex.what();
      exec.setFunctionResult( new BError( message ) );
      return false;
    }
  }
  else
  {
    DEBUGLOG << "Script Error in '" << exec.scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function " << exec.current_module_function->name.get() << ":\n"
             << "\tParameter " << param << ": Expected Long, String, or Struct, got datatype "
             << BObjectImp::typestr( imp->type() ) << "\n";
    return false;
  }

  // we get here if the value passed was an integer - either a BLong, or a String containing a
  // number.
  if ( ( objtype_long > Plib::systemstate.config.max_tile_id ) &&
       ( objtype_long <= Plib::systemstate.config.max_objtype ) )
  {
    const Items::ItemDesc* itemdesc = &Items::find_itemdesc( objtype_long );

    if ( itemdesc != Core::gamestate.empty_itemdesc.get() )
    {
      itemdesc_out = itemdesc;
      return true;
    }
    else
    {
      exec.setFunctionResult(
          new BError( "Objtype " + Clib::hexint( objtype_long ) + " is not defined." ) );
      return false;
    }
  }
  else if ( objtype_long <= Plib::systemstate.config.max_tile_id )
  {
    unsigned int objtype = objtype_long;
    itemdesc_out = &Items::find_itemdesc( objtype );
    if ( itemdesc_out == Core::gamestate.empty_itemdesc.get() )
    {
      // return a temporary item descriptor initialized with the objtype and graphic.
      itemdesc_out = Core::gamestate.temp_itemdesc.get();
      Core::gamestate.temp_itemdesc->objtype = objtype;
      Core::gamestate.temp_itemdesc->graphic = static_cast<u16>( objtype );
      Core::gamestate.temp_itemdesc->decay_time = settingsManager.ssopt.default_decay_time;
      Core::gamestate.temp_itemdesc->doubleclick_range =
          settingsManager.ssopt.default_doubleclick_range;
    }

    return true;
  }
  else
  {
    DEBUGLOG << "Script Error in '" << exec.scriptname() << "' PC=" << exec.PC << ": \n"
             << "\tCall to function " << exec.current_module_function->name.get() << ":\n"
             << "\tParameter " << param << ": Value " << objtype_long
             << " is out of range for an objtype\n";
    exec.setFunctionResult( new BError( "Objtype is out of range (acceptable: 0-0x20000)" ) );
    return false;
  }
}

bool getSkillIdParam( Executor& exec, unsigned param, USKILLID& skillid )
{
  int skillval;
  if ( exec.getParam( param, skillval, SKILLID__LOWEST,
                      networkManager.uoclient_general.maxskills ) )
  {
    skillid = static_cast<USKILLID>( skillval );
    return true;
  }
  else
  {
    return false;
  }
}


bool getAttributeParam( Executor& exec, unsigned param, const Mobile::Attribute*& attr )
{
  const String* attrname;
  if ( !exec.getStringParam( param, attrname ) )
    return false;

  attr = Mobile::Attribute::FindAttribute( attrname->value() );
  if ( !attr )
  {
    exec.setFunctionResult( new BError( "Attribute not defined: " + attrname->value() ) );
    return false;  // new BError( "Attribute not found" );
  }

  return true;
}


bool getVitalParam( Executor& exec, unsigned param, const Vital*& vital )
{
  const String* vitalname;
  if ( !exec.getStringParam( param, vitalname ) )
    return false;

  vital = FindVital( vitalname->value() );
  if ( !vital )
  {
    exec.setFunctionResult( new BError( "Vital not defined: " + vitalname->value() ) );
    return false;
  }

  return true;
}
}
}
