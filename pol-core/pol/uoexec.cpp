#include "uoexec.h"

#include <stddef.h>

#include "../bscript/berror.h"
#include "../bscript/bobject.h"
#include "../bscript/executor.h"
#include "../bscript/fmodule.h"
#include "../bscript/impstr.h"
#include "../clib/logfacility.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../plib/systemstate.h"
#include "fnsearch.h"
#include "globals/network.h"
#include "globals/settings.h"
#include "globals/uvars.h"
#include "guilds.h"
#include "guildscrobj.h"
#include "item/itemdesc.h"
#include "mobile/attribute.h"
#include "mobile/charactr.h"
#include "module/osmod.h"
#include "multi/multi.h"
#include "network/client.h"
#include "party.h"
#include "partyscrobj.h"
#include "polcfg.h"
#include "polclock.h"
#include "realms/realm.h"
#include "realms/realms.h"
#include "uobject.h"
#include "uoscrobj.h"
#include "vital.h"

namespace Pol
{
namespace Core
{
UOExecutor::UOExecutor()
    : Executor(),
      os_module( nullptr ),
      keep_alive_( false ),
      instr_cycles( 0 ),
      sleep_cycles( 0 ),
      start_time( poltime() ),
      warn_runaway_on_cycle( Plib::systemstate.config.runaway_script_threshold ),
      runaway_cycles( 0 ),
      eventmask( 0 ),
      area_mask( 0 ),
      area_size( 0 ),
      speech_size( 1 ),
      can_access_offline_mobiles_( false ),
      auxsvc_assume_string( false ),
      survive_attached_disconnect( false ),
      pParent( nullptr ),
      pChild( nullptr )
{
  weakptr.set( this );
  os_module = new Module::OSExecutorModule( *this );
  addModule( os_module );
}

UOExecutor::~UOExecutor()
{
  // note, the os_module isn't deleted here because
  // the Executor deletes its ExecutorModules.
  if ( ( instr_cycles >= 500 ) && settingsManager.watch.profile_scripts )
  {
    int elapsed = static_cast<int>(
        poltime() - start_time );  // Doh! A script can't run more than 68 years, for this to work.
    POLLOG_ERROR.Format( "Script {}: {} instr cycles, {} sleep cycles, {} seconds\n" )
        << scriptname() << instr_cycles << sleep_cycles << elapsed;
  }

  pParent = nullptr;
  pChild = nullptr;
}

bool UOExecutor::suspend()
{
  // Run to completion scripts can't be suspended
  if ( running_to_completion() )
    return false;

  os_module->suspend();
  return true;
}

bool UOExecutor::revive()
{
  os_module->revive();
  return true;
}

std::string UOExecutor::state()
{
  if ( halt() )
    return "Debugging";
  else if ( os_module->blocked() )
    return "Sleeping";
  else
    return "Running";
}


bool UOExecutor::signal_event( Bscript::BObjectImp* eventimp )
{
  passert_r( os_module != nullptr, "Object cannot receive events but is receiving them!" );
  return os_module->signal_event( eventimp );
}

size_t UOExecutor::sizeEstimate() const
{
  return sizeof( UOExecutor ) + base::sizeEstimate();
}

bool UOExecutor::critical() const
{
  return os_module->critical();
}
void UOExecutor::critical( bool critical )
{
  os_module->critical( critical );
}

bool UOExecutor::warn_on_runaway() const
{
  return os_module->warn_on_runaway();
}
void UOExecutor::warn_on_runaway( bool warn_on_runaway )
{
  os_module->warn_on_runaway( warn_on_runaway );
}

bool UOExecutor::keep_alive() const
{
  return keep_alive_;
}
void UOExecutor::keep_alive( bool status )
{
  keep_alive_ = status;
}

unsigned char UOExecutor::priority() const
{
  return os_module->priority();
}
void UOExecutor::priority( unsigned char priority )
{
  os_module->priority( priority );
}

void UOExecutor::SleepFor( u32 secs )
{
  os_module->SleepFor( secs );
}
void UOExecutor::SleepForMs( u32 msecs )
{
  os_module->SleepForMs( msecs );
}
unsigned int UOExecutor::pid() const
{
  return os_module->pid();
}
bool UOExecutor::blocked() const
{
  return os_module->blocked();
}
bool UOExecutor::in_debugger_holdlist() const
{
  return os_module->in_debugger_holdlist();
}
void UOExecutor::revive_debugged()
{
  os_module->revive_debugged();
}

Core::polclock_t UOExecutor::sleep_until_clock() const
{
  return os_module->sleep_until_clock();
}

void UOExecutor::sleep_until_clock( Core::polclock_t sleep_until_clock )
{
  os_module->sleep_until_clock( sleep_until_clock );
}

Core::TimeoutHandle UOExecutor::hold_itr() const
{
  return os_module->hold_itr();
}

void UOExecutor::hold_itr( Core::TimeoutHandle hold_itr )
{
  os_module->hold_itr( hold_itr );
}

Core::HoldListType UOExecutor::in_hold_list() const
{
  return os_module->in_hold_list();
}
void UOExecutor::in_hold_list( Core::HoldListType in_hold_list )
{
  return os_module->in_hold_list( in_hold_list );
}

Bscript::BObjectImp* UOExecutor::clear_event_queue()
{
  return os_module->clear_event_queue();
}  // DAVE

using namespace Bscript;
using namespace Module;

bool UOExecutor::getCharacterOrClientParam( unsigned param, Mobile::Character*& chrptr,
                                            Network::Client*& clientptr )
{
  chrptr = nullptr;
  clientptr = nullptr;

  BObjectImp* imp = getParamImp( param );
  if ( imp == nullptr )
  {
    setFunctionResult( new BError( "Missing parameter " + Clib::tostring( param ) ) );
    return false;
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = Clib::explicit_cast<BApplicObjBase*, BObjectImp*>( imp );

    if ( ( aob != nullptr ) && ( aob->object_type() == &echaracterrefobjimp_type ) )
    {
      ECharacterRefObjImp* chrref_imp =
          Clib::explicit_cast<ECharacterRefObjImp*, BApplicObjBase*>( aob );

      chrptr = chrref_imp->value().get();

      if ( chrptr->orphan() )
      {
        setFunctionResult( new BError( "Mobile has been destroyed" ) );
        return false;
      }

      if ( chrptr->logged_in() || chrref_imp->offline_access_ok() || can_access_offline_mobiles_ )
      {
        return true;
      }
      else
      {
        setFunctionResult( new BError( "Mobile is offline" ) );
        return false;
      }
    }
    else if ( ( aob != nullptr ) && ( aob->object_type() == &eclientrefobjimp_type ) )
    {
      EClientRefObjImp* clientref_imp =
          Clib::explicit_cast<EClientRefObjImp*, BApplicObjBase*>( aob );

      clientptr = clientref_imp->value().exists() ? clientref_imp->value().get_weakptr() : nullptr;

      if ( ( clientptr != nullptr ) && clientptr->isConnected() )
      {
        return true;
      }
      else
      {
        setFunctionResult( new BError( "Client is disconnected" ) );
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
      setFunctionResult( new BError( "Serial refers to an Item, or is zero" ) );
      return false;
    }

    chrptr = system_find_mobile( serial );

    if ( chrptr != nullptr )
    {
      if ( chrptr->logged_in() || can_access_offline_mobiles_ )
      {
        return true;
      }
      else
      {
        setFunctionResult( new BError( "Mobile is offline" ) );
        return false;
      }
    }
    else
    {
      setFunctionResult( new BError( "Mobile does not exist" ) );
      return false;
    }
  }
  else
  {
    // FIXME: log error
    return false;
  }
}

bool UOExecutor::getCharacterParam( unsigned param, Mobile::Character*& chrptr )
{
  BObjectImp* imp = getParamImp( param );
  if ( imp == nullptr )
  {
    setFunctionResult( new BError( "Missing parameter " + Clib::tostring( param ) ) );
    return false;
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = Clib::explicit_cast<BApplicObjBase*, BObjectImp*>( imp );

    if ( ( aob != nullptr ) && ( aob->object_type() == &echaracterrefobjimp_type ) )
    {
      ECharacterRefObjImp* chrref_imp =
          Clib::explicit_cast<ECharacterRefObjImp*, BApplicObjBase*>( aob );

      chrptr = chrref_imp->value().get();

      if ( chrptr->orphan() )
      {
        setFunctionResult( new BError( "Mobile has been destroyed" ) );
        return false;
      }

      if ( chrptr->logged_in() || chrref_imp->offline_access_ok() || can_access_offline_mobiles_ )
      {
        return true;
      }
      else
      {
        setFunctionResult( new BError( "Mobile is offline" ) );
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
      setFunctionResult( new BError( "Serial refers to an Item, or is zero" ) );
      return false;
    }

    chrptr = system_find_mobile( serial );

    if ( chrptr != nullptr )
    {
      if ( chrptr->logged_in() || can_access_offline_mobiles_ )
      {
        return true;
      }
      else
      {
        setFunctionResult( new BError( "Mobile is offline" ) );
        return false;
      }
    }
    else
    {
      setFunctionResult( new BError( "Mobile does not exist" ) );
      return false;
    }
  }
  else
  {
    // FIXME: log error
    return false;
  }
}

bool UOExecutor::getItemParam( unsigned param, Items::Item*& itemptr )
{
  BObjectImp* imp = getParamImp( param );
  if ( imp == nullptr )
  {
    return false;
  }
  else if ( imp->isa( BObjectImp::OTApplicObj ) )
  {
    BApplicObjBase* aob = Clib::explicit_cast<BApplicObjBase*, BObjectImp*>( imp );

    if ( ( aob != nullptr ) && ( aob->object_type() == &eitemrefobjimp_type ) )
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

    return ( itemptr != nullptr );
  }
  else
  {
    // FIXME: log error
    return false;
  }
}

bool UOExecutor::getUBoatParam( unsigned param, Multi::UBoat*& boatptr )
{
  BObjectImp* imp = getParamImp( param );
  if ( imp == nullptr )
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
        if ( boatptr == nullptr )
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

    return ( boatptr != nullptr );
  }
  else
  {
    // FIXME: log error
    return false;
  }
}


bool UOExecutor::getMultiParam( unsigned param, Multi::UMulti*& multiptr )
{
  BObjectImp* imp = getParamImp( param );
  if ( imp == nullptr )
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

    return ( multiptr != nullptr );
  }
  else
  {
    // FIXME: log error
    return false;
  }
}

bool UOExecutor::getUObjectParam( unsigned param, UObject*& objptr )
{
  Items::Item* item = nullptr;
  Mobile::Character* chr = nullptr;
  Multi::UMulti* multi = nullptr;

  // This function is a kludge because the individual functions will all test for
  // a serial independently and may set errors.
  // TODO: Refactor this function to test whether the parameter is a BLong in a single place.

  if ( getCharacterParam( param, chr ) )
  {
    objptr = chr;
    return true;
  }
  else if ( getItemParam( param, item ) )
  {
    objptr = item;
    return true;
  }
  else if ( getMultiParam( param, multi ) )
  {
    objptr = multi;
    return true;
  }
  else
  {
    return false;
  }
}

bool UOExecutor::getObjtypeParam( unsigned param, unsigned int& objtype )
{
  BObjectImp* imp = getParamImp( param );
  if ( imp == nullptr )
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
        setFunctionResult( new BError( std::string( "Objtype not defined: " ) + pstring->data() ) );

        return false;
      }
    }
    else
    {
      // a number passed...process below as if passed as a BLong
      objtype_long = strtol( ot_str, nullptr, 0 );
    }
  }
  else
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << PC << ": \n"
             << "\tCall to function " << current_module_function->name.get() << ":\n"
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
      setFunctionResult(
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
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << PC << ": \n"
             << "\tCall to function " << current_module_function->name.get() << ":\n"
             << "\tParameter " << param << ": Value " << objtype_long
             << " is out of range for an objtype\n";
    setFunctionResult( new BError( "Objtype is out of range ( acceptable: 0 - " +
                                   Clib::hexint( Plib::systemstate.config.max_objtype ) + " )" ) );
    return false;
  }
}

bool UOExecutor::getObjtypeParam( unsigned param, const Items::ItemDesc*& itemdesc_out )
{
  BObjectImp* imp = getParamImp( param );
  if ( imp == nullptr )
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
        setFunctionResult( new BError( std::string( "Objtype not defined: " ) + pstring->data() ) );

        return false;
      }
    }
    else
    {
      // a number passed...process below as if passed as a BLong
      objtype_long = strtol( ot_str, nullptr, 0 );
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
      setFunctionResult( new BError( message ) );
      return false;
    }
  }
  else
  {
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << PC << ": \n"
             << "\tCall to function " << current_module_function->name.get() << ":\n"
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
      setFunctionResult(
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
    DEBUGLOG << "Script Error in '" << scriptname() << "' PC=" << PC << ": \n"
             << "\tCall to function " << current_module_function->name.get() << ":\n"
             << "\tParameter " << param << ": Value " << objtype_long
             << " is out of range for an objtype\n";
    setFunctionResult( new BError( "Objtype is out of range (acceptable: 0-0x20000)" ) );
    return false;
  }
}

bool UOExecutor::getSkillIdParam( unsigned param, USKILLID& skillid )
{
  BObjectImp* imp = getParamImp( param );
  if ( imp == nullptr )
  {
    setFunctionResult( new BError( "Missing parameter " + Clib::tostring( param ) ) );
    return false;
  }
  else if ( imp->isa( BObjectImp::OTLong ) )
  {
    BLong* plong = Clib::explicit_cast<BLong*, BObjectImp*>( imp );
    int value = plong->value();
    if ( value >= SKILLID__LOWEST && value <= networkManager.uoclient_general.maxskills )
    {
      skillid = static_cast<USKILLID>( value );
      return true;
    }
    else
    {
      std::string report = "Parameter " + Clib::tostring( param ) + " value " +
                           Clib::tostring( value ) + " out of expected range of [" +
                           Clib::tostring( SKILLID__LOWEST ) + ".." +
                           Clib::tostring( networkManager.uoclient_general.maxskills ) + "]";
      setFunctionResult( new BError( report ) );
      return false;
    }
  }
  else if ( imp->isa( BObjectImp::OTString ) )
  {
    const Mobile::Attribute* attr;
    if ( !getAttributeParam( param, attr ) )
      return false;
    if ( attr->skillid != static_cast<USKILLID>( -1 ) )
    {
      skillid = attr->skillid;
      return true;
    }
    const String* attrname;
    getStringParam( param, attrname );  // no error check needed
    std::string report = "Parameter " + Clib::tostring( param ) + " value " + attrname->value() +
                         " has no skill id defined";
    setFunctionResult( new BError( report ) );
    return false;
  }
  std::string report = "Invalid parameter type.  Expected param " + Clib::tostring( param ) +
                       " as " + BObjectImp::typestr( BObjectImp::OTLong ) + " or " +
                       BObjectImp::typestr( BObjectImp::OTString ) + ", got " +
                       BObjectImp::typestr( imp->type() );
  setFunctionResult( new BError( report ) );
  return false;
}


bool UOExecutor::getAttributeParam( unsigned param, const Mobile::Attribute*& attr )
{
  const String* attrname;
  if ( !getStringParam( param, attrname ) )
    return false;

  attr = Mobile::Attribute::FindAttribute( attrname->value() );
  if ( !attr )
  {
    setFunctionResult( new BError( "Attribute not defined: " + attrname->value() ) );
    return false;
  }

  return true;
}


bool UOExecutor::getVitalParam( unsigned param, const Vital*& vital )
{
  const String* vitalname;
  if ( !getStringParam( param, vitalname ) )
    return false;

  vital = FindVital( vitalname->value() );
  if ( !vital )
  {
    setFunctionResult( new BError( "Vital not defined: " + vitalname->value() ) );
    return false;
  }

  return true;
}

bool UOExecutor::getGuildParam( unsigned param, Core::Guild*& guild, Bscript::BError*& err )
{
  BApplicObjBase* aob = nullptr;
  if ( hasParams( param + 1 ) )
    aob = getApplicObjParam( param, &guild_type );

  if ( aob == nullptr )
  {
    err = new BError( "Invalid parameter type" );
    return false;
  }

  EGuildRefObjImp* gr = static_cast<EGuildRefObjImp*>( aob );
  guild = gr->value().get();
  if ( guild->disbanded() )
  {
    err = new BError( "Guild has disbanded" );
    return false;
  }
  return true;
}

bool UOExecutor::getPartyParam( unsigned param, Core::Party*& party, BError*& err )
{
  BApplicObjBase* aob = nullptr;
  if ( hasParams( param + 1 ) )
    aob = getApplicObjParam( param, &party_type );

  if ( aob == nullptr )
  {
    err = new BError( "Invalid parameter type" );
    return false;
  }

  EPartyRefObjImp* pr = static_cast<EPartyRefObjImp*>( aob );
  party = pr->value().get();
  if ( Core::system_find_mobile( party->leader() ) == nullptr )
  {
    err = new BError( "Party has no leader" );
    return false;
  }
  return true;
}

bool UOExecutor::getRealmParam( unsigned param, Realms::Realm** realm )
{
  const Bscript::String* realm_name;
  if ( !getStringParam( param, realm_name ) )
    return false;
  *realm = find_realm( realm_name->value() );
  if ( !realm )
  {
    setFunctionResult( new Bscript::BError( "Realm not found." ) );
    return false;
  }
  return true;
}

bool UOExecutor::getPos2dParam( unsigned xparam, unsigned yparam, Pos2d* pos,
                                const Realms::Realm* realm )
{
  u16 x;
  u16 y;
  if ( getParam( xparam, x ) && getParam( yparam, y ) )
  {
    *pos = Pos2d( x, y );
    if ( realm && !realm->valid( x, y, 0 ) )
    {
      setFunctionResult( new Bscript::BError( "Invalid Coordinates for Realm" ) );
      return false;
    }
    return true;
  }
  return false;
}
bool UOExecutor::getPos3dParam( unsigned xparam, unsigned yparam, unsigned zparam, Pos3d* pos,
                                const Realms::Realm* realm )
{
  u16 x;
  u16 y;
  s8 z;
  if ( getParam( xparam, x ) && getParam( yparam, y ) && getParam( zparam, z ) )
  {
    *pos = Pos3d( x, y, z );
    if ( realm && !realm->valid( x, y, z ) )
    {
      setFunctionResult( new Bscript::BError( "Invalid Coordinates for Realm" ) );
      return false;
    }
    return true;
  }
  return false;
}
bool UOExecutor::getPos4dParam( unsigned xparam, unsigned yparam, unsigned zparam,
                                unsigned realmparam, Pos4d* pos )
{
  u16 x;
  u16 y;
  s8 z;
  Realms::Realm* realm;
  if ( getParam( xparam, x ) && getParam( yparam, y ) && getParam( zparam, z ) &&
       getRealmParam( realmparam, &realm ) )
  {
    Pos3d p( x, y, z );
    if ( !realm->valid( x, y, z ) )
    {
      setFunctionResult( new Bscript::BError( "Invalid Coordinates for Realm" ) );
      return false;
    }
    *pos = Pos4d( p, realm );
    return true;
  }
  return false;
}
}  // namespace Core
}  // namespace Pol
