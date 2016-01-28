/** @file
 *
 * @par History
 * - 2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
 * - 2006/09/27 Shinigami: GCC 3.4.x fix - added "template<>" to TmplExecutorModule
 * - 2007/06/17 Shinigami: added config.world_data_path
 * - 2009/12/21 Turley:    ._method() call fix
 */


#include "guildmod.h"
#include "../guilds.h"

#include "uomod.h"

#include "../../bscript/execmodl.h"
#include "../../bscript/berror.h"
#include "../../bscript/bobject.h"
#include "../../bscript/impstr.h"
#include "../../bscript/objmembers.h"
#include "../../bscript/objmethods.h"

#include "../../clib/refptr.h"
#include "../../clib/stlutil.h"

#include "../../plib/systemstate.h"

#include "../globals/uvars.h"
#include "../mobile/charactr.h"
#include "../fnsearch.h"
#include "../polcfg.h"
#include "../proplist.h"
#include "../ufunc.h"
#include "../uobject.h"
#include "../uoscrobj.h"

#include <map>

namespace Pol {

  /// Guild Object
  ///  Properties:
  ///   guild.guildid : integer
  ///   guild.members : array of offline mobilerefs
  ///   guild.allyguilds  : array of allied guilds
  ///   guild.enemyguilds : array of enemy guilds
  ///
  ///  Methods:
  ///   guild.ismember( who )
  ///   guild.isallyguild( guild )
  ///   guild.isenemyguild(guild)
  ///
  ///   guild.addmember( who )
  ///   guild.addallyguild( guild )
  ///   guild.addenemyguild( guild )
  ///
  ///   guild.removemember( who )
  ///   guild.removeallyguild( guild )
  ///   guild.removeenemyguild( guild )
  ///
  ///   guild.getprop( propname )
  ///   guild.setprop( propname, propvalue )
  ///   guild.eraseprop( propname )
  ///
  namespace Bscript {
    using namespace Module;
    template<>
    TmplExecutorModule<GuildExecutorModule>::FunctionDef
      TmplExecutorModule<GuildExecutorModule>::function_table[] =
    {
      { "ListGuilds", &GuildExecutorModule::mf_ListGuilds },
      { "CreateGuild", &GuildExecutorModule::mf_CreateGuild },
      { "FindGuild", &GuildExecutorModule::mf_FindGuild },
      { "DestroyGuild", &GuildExecutorModule::mf_DestroyGuild },
    };
    template<>
    int TmplExecutorModule<GuildExecutorModule>::function_table_size =
      arsize( function_table );
  }
  namespace Module {
    using namespace Bscript;

	class EGuildRefObjImp : public BApplicObj<Core::GuildRef>
	{
	public:
	  EGuildRefObjImp( Core::GuildRef gref );
	  virtual const char* typeOf() const POL_OVERRIDE;
	  virtual u8 typeOfInt() const POL_OVERRIDE;
	  virtual BObjectImp* copy() const POL_OVERRIDE;
	  virtual bool isTrue() const POL_OVERRIDE;
	  virtual bool operator==( const BObjectImp& objimp ) const POL_OVERRIDE;

	  virtual BObjectRef get_member( const char* membername ) POL_OVERRIDE;
	  virtual BObjectRef get_member_id( const int id ) POL_OVERRIDE; //id test
	  virtual BObjectImp* call_method( const char* methodname, Executor& ex ) POL_OVERRIDE;
	  virtual BObjectImp* call_method_id( const int id, Executor& ex, bool forcebuiltin = false ) POL_OVERRIDE;
	};

	BApplicObjType guild_type;


	EGuildRefObjImp::EGuildRefObjImp( Core::GuildRef gref ) : BApplicObj<Core::GuildRef>( &guild_type, gref )
	{};

	const char* EGuildRefObjImp::typeOf() const
	{
	  return "GuildRef";
	}
	u8 EGuildRefObjImp::typeOfInt() const
	{
	  return OTGuildRef;
	}

	BObjectImp* EGuildRefObjImp::copy() const
	{
	  return new EGuildRefObjImp( obj_ );
	}

	bool EGuildRefObjImp::isTrue() const
	{
	  return ( !obj_->_disbanded );
	}

	bool EGuildRefObjImp::operator==( const BObjectImp& objimp ) const
	{
	  if ( objimp.isa( BObjectImp::OTApplicObj ) )
	  {
		const BApplicObjBase* aob = Clib::explicit_cast<const BApplicObjBase*, const BObjectImp*>( &objimp );

		if ( aob->object_type() == &guild_type )
		{
		  const EGuildRefObjImp* guildref_imp =
			Clib::explicit_cast<const EGuildRefObjImp*, const BApplicObjBase*>( aob );

		  return ( guildref_imp->obj_->_guildid == obj_->_guildid );
		}
		else
		  return false;
	  }
	  else
		return false;
	}

	BObjectImp* GuildExecutorModule::CreateGuildRefObjImp( Core::Guild* guild )
	{
	  return new EGuildRefObjImp( ref_ptr<Core::Guild>( guild ) );
	}

	bool getGuildParam( Executor& exec,
						unsigned param,
						Core::Guild*& guild,
						BError*& err )
	{
	  BApplicObjBase* aob = NULL;
	  if ( exec.hasParams( param + 1 ) )
		aob = exec.getApplicObjParam( param, &guild_type );

	  if ( aob == NULL )
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

	BObjectRef EGuildRefObjImp::get_member_id( const int id ) //id test
	{
	  if ( obj_->_disbanded )
		return BObjectRef( new BError( "Guild has disbanded" ) );

	  switch ( id )
	  {
		case MBR_MEMBERS:
		{
						  std::unique_ptr<ObjArray> arr;
						  arr.reset( new ObjArray );
						  for ( Core::SerialSet::iterator itr = obj_->_member_serials.begin();
								itr != obj_->_member_serials.end();
								/* do this earlier */ )
						  {
							unsigned int mserial = ( *itr );
							Core::SerialSet::iterator last_itr = itr;
							++itr;

                            Mobile::Character* chr = Core::system_find_mobile( mserial );
							if ( chr != NULL )
							{
							  arr->addElement( new EOfflineCharacterRefObjImp( chr ) );
							}
							else
							{
							  obj_->_member_serials.erase( last_itr );
							}
						  }
						  return BObjectRef( arr.release() );
		}

		case MBR_ALLYGUILDS:
		{
							 std::unique_ptr<ObjArray> arr;
							 arr.reset( new ObjArray );
							 for ( Core::SerialSet::iterator itr = obj_->_allyguild_serials.begin();
								   itr != obj_->_allyguild_serials.end();
								   /* do this earlier */ )
							 {
							   unsigned int gserial = ( *itr );
							   Core::SerialSet::iterator last_itr = itr;
							   ++itr;

                               Core::Guild* guild = Core::Guild::FindGuild( gserial );

							   if ( guild != NULL )
							   {
								 arr->addElement( new EGuildRefObjImp( ref_ptr<Core::Guild>( guild ) ) );
							   }
							   else
							   {
								 obj_->_allyguild_serials.erase( last_itr );
							   }
							 }
							 return BObjectRef( arr.release() );
		}

		case MBR_ENEMYGUILDS:
		{
							  std::unique_ptr<ObjArray> arr;
							  arr.reset( new ObjArray );
							  for ( Core::SerialSet::iterator itr = obj_->_enemyguild_serials.begin();
									itr != obj_->_enemyguild_serials.end();
									/* do this earlier */ )
							  {
								unsigned int gserial = ( *itr );
								Core::SerialSet::iterator last_itr = itr;
								++itr;

								Core::Guild* guild = Core::Guild::FindGuild( gserial );

								if ( guild != NULL )
								{
								  arr->addElement( new EGuildRefObjImp( ref_ptr<Core::Guild>( guild ) ) );
								}
								else
								{
								  obj_->_enemyguild_serials.erase( last_itr );
								}
							  }
							  return BObjectRef( arr.release() );
		}

		case MBR_GUILDID:
		  return BObjectRef( new BLong( obj_->_guildid ) );
		default:
		  return BObjectRef( UninitObject::create() );
	  }
	}
	BObjectRef EGuildRefObjImp::get_member( const char* membername )
	{
	  if ( obj_->_disbanded )
		return BObjectRef( new BError( "Guild has disbanded" ) );

	  ObjMember* objmember = getKnownObjMember( membername );
	  if ( objmember != NULL )
		return this->get_member_id( objmember->id );
	  else
		return BObjectRef( UninitObject::create() );
	}

	BObjectImp* EGuildRefObjImp::call_method_id( const int id, Executor& ex, bool /*forcebuiltin*/ )
	{
	  if ( obj_->_disbanded )
		return new BError( "Guild has disbanded" );

	  switch ( id )
	  {
		case MTH_ISMEMBER:
		{
						   if ( !ex.hasParams( 1 ) )
							 return new BError( "Not enough parameters" );
                           Mobile::Character* chr;
						   if ( !getCharacterParam( ex, 0, chr ) )
							 return new BError( "Invalid parameter type" );

						   return new BLong( static_cast<int>( obj_->_member_serials.count( chr->serial ) ) );
		}

		case MTH_ISALLYGUILD:
		{
							  if ( !ex.hasParams( 1 ) )
								return new BError( "Not enough parameters" );
							  Core::Guild* allyguild;
							  BError* err;
							  if ( getGuildParam( ex, 0, allyguild, err ) )
								return new BLong( static_cast<int>( obj_->_allyguild_serials.count( allyguild->guildid() ) ) );
							  else
								return err;
		}

		case MTH_ISENEMYGUILD:
		{
							   if ( !ex.hasParams( 1 ) )
								 return new BError( "Not enough parameters" );
							   Core::Guild* enemyguild;
							   BError* err;
							   if ( getGuildParam( ex, 0, enemyguild, err ) )
								 return new BLong( static_cast<int>( obj_->_enemyguild_serials.count( enemyguild->guildid() ) ) );
							   else
								 return err;
		}

		case MTH_ADDMEMBER:
		{
							if ( !ex.hasParams( 1 ) )
							  return new BError( "Not enough parameters" );
                            Mobile::Character* chr;
							if ( !getCharacterParam( ex, 0, chr ) )
							  return new BError( "Invalid parameter type" );
							if ( chr->guildid() )
							  return new BError( "Character already belongs to a guild" );

							chr->guild( obj_.get() );
							obj_->_member_serials.insert( chr->serial );

							// MuadDib Added to update online members when status changes.
							obj_->update_online_members();

							return new BLong( 1 );
		}

		case MTH_ADDALLYGUILD:
		{
							   if ( !ex.hasParams( 1 ) )
								 return new BError( "Not enough parameters" );
							   Core::Guild* allyguild;
							   BError* err;
							   if ( getGuildParam( ex, 0, allyguild, err ) )
							   {
								 if ( obj_->_enemyguild_serials.count( allyguild->guildid() ) )
								   return new BError( "That is an enemy guild" );
								 if ( allyguild->guildid() == obj_->guildid() )
								   return new BError( "Passed self as new ally guild" );

								 obj_->_allyguild_serials.insert( allyguild->guildid() );
								 allyguild->_allyguild_serials.insert( obj_->_guildid );

								 // MuadDib Added to update online members when status changes.
								 obj_->update_online_members();
								 allyguild->update_online_members();

								 return new BLong( 1 );
							   }
							   else
								 return err;
		}

		case MTH_ADDENEMYGUILD:
		{
								if ( !ex.hasParams( 1 ) )
								  return new BError( "Not enough parameters" );
								Core::Guild* enemyguild;
								BError* err;
								if ( getGuildParam( ex, 0, enemyguild, err ) )
								{
								  if ( obj_->_allyguild_serials.count( enemyguild->guildid() ) )
									return new BError( "That is an ally guild" );
								  if ( enemyguild->guildid() == obj_->guildid() )
									return new BError( "Passed self as new enemy guild" );

								  obj_->_enemyguild_serials.insert( enemyguild->guildid() );
								  enemyguild->_enemyguild_serials.insert( obj_->_guildid );

								  // MuadDib Added to update online members when status changes.
								  obj_->update_online_members();
								  enemyguild->update_online_members();

								  return new BLong( 1 );
								}
								else
								  return err;
		}

		case MTH_REMOVEMEMBER:
		{
							   if ( !ex.hasParams( 1 ) )
								 return new BError( "Not enough parameters" );
                               Mobile::Character* chr;
							   if ( !getCharacterParam( ex, 0, chr ) )
								 return new BError( "Invalid parameter type" );
							   if ( chr->guildid() != obj_->_guildid )
								 return new BError( "Character does not belong to this guild" );

							   chr->guild( NULL );
							   obj_->_member_serials.erase( chr->serial );

							   // MuadDib Added to update online members when status changes.
							   obj_->update_online_members_remove( chr );

							   return new BLong( 1 );
		}

		case MTH_REMOVEALLYGUILD:
		{
								  if ( !ex.hasParams( 1 ) )
									return new BError( "Not enough parameters" );
								  Core::Guild* allyguild;
								  BError* err;
								  if ( getGuildParam( ex, 0, allyguild, err ) )
								  {
									if ( !obj_->_allyguild_serials.count( allyguild->guildid() ) )
									  return new BError( "That is not an ally guild" );

									obj_->_allyguild_serials.erase( allyguild->guildid() );
									allyguild->_allyguild_serials.erase( obj_->_guildid );

									// MuadDib Added to update online members when status changes.
									obj_->update_online_members();
									allyguild->update_online_members();

									return new BLong( 1 );
								  }
								  else
									return err;
		}

		case MTH_REMOVEENEMYGUILD:
		{
								   if ( !ex.hasParams( 1 ) )
									 return new BError( "Not enough parameters" );
								   Core::Guild* enemyguild;
								   BError* err;
								   if ( getGuildParam( ex, 0, enemyguild, err ) )
								   {
									 if ( !obj_->_enemyguild_serials.count( enemyguild->guildid() ) )
									   return new BError( "That is not an enemy guild" );

									 obj_->_enemyguild_serials.erase( enemyguild->guildid() );
									 enemyguild->_enemyguild_serials.erase( obj_->_guildid );

									 // MuadDib Added to update online members when status changes.
									 obj_->update_online_members();
									 enemyguild->update_online_members();

									 return new BLong( 1 );
								   }
								   else
									 return err;
		}
		default:
		{
				 bool changed = false;
				 return CallPropertyListMethod_id( obj_->_proplist, id, ex, changed );
		}
	  }
	}

	BObjectImp* EGuildRefObjImp::call_method( const char* methodname, Executor& ex )
	{
	  if ( obj_->_disbanded )
		return new BError( "Guild has disbanded" );

	  ObjMethod* objmethod = getKnownObjMethod( methodname );
	  if ( objmethod != NULL )
		return this->call_method_id( objmethod->id, ex );
	  else
	  {
		bool changed = false;
		return CallPropertyListMethod( obj_->_proplist, methodname, ex, changed );
	  }
	}


	/// uo.em functions:
	///  ListGuilds(); // returns an array of Guild objects
	BObjectImp* GuildExecutorModule::mf_ListGuilds()
	{
	  std::unique_ptr<ObjArray> result( new ObjArray );
	  for ( Core::Guilds::iterator itr = Core::gamestate.guilds.begin(), end = Core::gamestate.guilds.end(); itr != end; ++itr )
	  {
		Core::Guild* guild = ( *itr ).second.get();
		result->addElement( new EGuildRefObjImp( Core::GuildRef( guild ) ) );
	  }
	  return result.release();
	}

	///  CreateGuild(); // returns a new Guild object
	BObjectImp* GuildExecutorModule::mf_CreateGuild()
	{
	  Core::Guild* guild = new Core::Guild( Core::gamestate.nextguildid++ );

	  Core::gamestate.guilds[guild->guildid()].set( guild );

	  return new EGuildRefObjImp( ref_ptr<Core::Guild>( guild ) );
	}

	BObjectImp* GuildExecutorModule::mf_DestroyGuild()
	{
	  Core::Guild* guild;
	  BError* err;
	  if ( getGuildParam( exec, 0, guild, err ) )
	  {
		if ( guild->hasMembers() )
		  return new BError( "Guild has members" );
		if ( guild->hasAllies() )
		  return new BError( "Guild has allies" );
		if ( guild->hasEnemies() )
		  return new BError( "Guild has enemies" );

		guild->disband();
		Core::gamestate.guilds.erase( guild->guildid() );
		return new BLong( 1 );
	  }
	  else
	  {
		return err;
	  }
	}

	///  FindGuild( guildid );
	BObjectImp* GuildExecutorModule::mf_FindGuild()
	{
	  int guildid;
	  if ( getParam( 0, guildid ) )
	  {
		Core::Guilds::iterator itr = Core::gamestate.guilds.find( guildid );
		if ( itr != Core::gamestate.guilds.end() )
		  return new EGuildRefObjImp( ref_ptr<Core::Guild>( ( *itr ).second.get() ) );
		else
		  return new BError( "Guild not found" );
	  }
	  else
	  {
		return new BError( "Invalid parameter type" );
	  }
	}

	

  }
  
}