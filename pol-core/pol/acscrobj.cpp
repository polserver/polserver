/*
History
=======
2005/04/04 Shinigami: account.Set_UO_Expansion( string ) - extended for Samurai Empire,
                      added value check, added to AccountObjImp::call_method (was missing)
2005/04/04 Shinigami: call ondelete script in account.DeleteCharacter( index ) too
2005/04/04 Shinigami: added candelete script
2005/05/24 Shinigami: added account.delete() to delete this account
2005/05/25 Shinigami: added account.split( newacctname : string, index : 1..5 )
                      to create a new account and move character to it
2005/05/25 Shinigami: added account.move_char( destacctname : string, index : 1..5 )
                      to move character from this account to destaccount
2005/11/25 Shinigami: MTH_GET_MEMBER/"get_member" - GCC fix
2005/11/26 Shinigami: changed "strcmp" into "stricmp" to suppress Script Errors
2006/05/16 Shinigami: account.Set_UO_Expansion( string ) - extended for Mondain's Legacy
2009/08/06 MuadDib:   Removed PasswordOnlyHash support

Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/MD5.h"

#include "bscript/berror.h"
#include "bscript/bobject.h"
#include "bscript/executor.h"
#include "bscript/impstr.h"
#include "bscript/objmembers.h"
#include "bscript/objmethods.h"

#include "account.h"
#include "accounts.h"
#include "acscrobj.h"
#include "charactr.h"
#include "client.h"
#include "cmdlevel.h"
#include "core.h"
#include "objecthash.h"
#include "polcfg.h"
#include "realms.h"
#include "ufunc.h"
#include "uoscrobj.h"
#include "uoexhelp.h"

BApplicObjType accountobjimp_type;

const char* AccountObjImp::typeOf() const
{
	return "AccountRef";
}

BObjectImp* AccountObjImp::copy() const
{
	return new AccountObjImp( obj_ );
}

bool can_delete_character( Character* chr, int delete_by );
void call_ondelete_scripts( Character* chr );
void delete_character( Account* acct, Character* chr, int charidx );
void createchar2(Account* acct, unsigned index);

///
/// [1] Account Scripting Object Methods
///
///	 All methods except GetProp and GetCharacter return 1 on success
///	 All methods except GetProp and GetCharacter write the accounts.txt file on success.
///	 All methods return Error("Not enough parameters") if too few parameters were passed.
///	 All methods return Error("Invalid parameter type") if the wrong type was passed.
///
BObjectImp* AccountObjImp::call_method_id( const int id, Executor& ex )
{
	BObjectImp* result = NULL;
	bool changed = false; // only set for proplist methods

	long index;
	Character* chr;
	switch(id)
	{
	case MTH_GET_MEMBER:
	{
		if ( !ex.hasParams(1) )
			return new BError("Not enough parameters");
		
		const String* mname;
		if ( ex.getStringParam(0, mname) )
		{
			BObjectRef ref_temp = get_member(mname->value().c_str());
			BObjectRef& ref = ref_temp;
			BObject *bo = ref.get();
			BObjectImp *ret = bo->impptr();
			ret = ret->copy();
			if ( ret->isa(OTUninit) )
			{
				string message = string("Member ") + string(mname->value()) + string(" not found on that object");
				return new BError(message);
			}
			else
			{
				return ret;
			}
		}
		else
			return new BError( "Invalid parameter type" );
		break;
	}
	///	
	/// account.Ban() : bans the account.  Disconnects connected client if any.
	///
	case MTH_BAN:
		if (ex.numParams() == 0)
		{
			obj_->banned_ = true;
			if (obj_->active_character)
			{
				if (obj_->active_character->client)
					obj_->active_character->client->disconnect = true;
			}
		}
		else
			return new BError( "account.Ban() doesn't take parameters." );
		break;
	///
	/// account.Unban() : unbans the account.
	///
	case MTH_UNBAN:
		if (ex.numParams() == 0)
			obj_->banned_ = false;
		else
			return new BError( "account.Unban() doesn't take parameters." );
		break;
	///
	/// account.Enable() : enables the account
	///
	case MTH_ENABLE:
		if (ex.numParams() == 0)
			obj_->enabled_ = true;
		else
			return new BError( "account.Enable() doesn't take parameters." );
		break;
	///
	/// account.Disable() : disables the account.  Disconnects connected client if any.
	///
	case MTH_DISABLE:
		if (ex.numParams() == 0)
		{
			obj_->enabled_ = false;
			if (obj_->active_character)
			{
				if (obj_->active_character->client)
					obj_->active_character->client->disconnect = true;
			}
			break;
		}
		else
			return new BError( "account.Disable() doesn't take parameters." );
	///
	/// account.SetPassword( newpassword : string ) : changes the password..
	///
	case MTH_SETPASSWORD:
		if (ex.numParams() == 1)
		{
			const String* pwstr;
			if (ex.getStringParam( 0, pwstr ))
			{
				if(config.retain_cleartext_passwords)
					obj_->password_ = pwstr->value();

				std::string temp;
				MD5_Encrypt(obj_->name_ + pwstr->value(),temp);
				obj_->passwordhash_ = temp; //MD5
				break;
			}
			else
			{
				return new BError( "Invalid parameter type" );
			}
		}
		else
			return new BError( "account.SetPassword(newpass) requires a parameter." );
	///
	/// account.CheckPassword( password : string ) : checks if the password is valid
	///
	case MTH_CHECKPASSWORD:
		if (ex.numParams() == 1)
		{
			const String* pwstr;
			if (ex.getStringParam( 0, pwstr ))
			{
				bool ret;
				string temp;

				MD5_Encrypt(obj_->name_ + pwstr->value(),temp);//MD5
				ret = MD5_Compare(obj_->passwordhash_,temp);

				result = new BLong( ret ); 
			}
			else
			{
				return new BError( "Invalid parameter type" );
			}
		}
		else
			return new BError( "account.CheckPassword(password) requires a parameter." );
		break;
	///
	/// account.SetAcctName( newname : string ) : changes the account name
	///	- deprecated in favor of:
	/// account.SetName( newname : string ) : changes the account name
	///  ACK, bug - since account data is saved immediately,
	///  a crash w/o save will result in a server that can't start
	///  because account names in accounts.txt will refer to the old name
	///
	case MTH_SETNAME:
		//passed only new account name, and cleartext password is saved
		if ( (ex.numParams() == 1) && config.retain_cleartext_passwords )
		{
			const String* nmstr;
			if (ex.getStringParam( 0, nmstr ))
			{
				if(nmstr->value().empty())
					return new BError( "Account name must not be empty." );
				std::string temp;
				//passing the new name, and recalc name+pass hash (pass only hash is unchanged)
				obj_->name_ = nmstr->value();
				MD5_Encrypt(obj_->name_ + obj_->password_,temp);
				obj_->passwordhash_ = temp; //MD5
			}
			else
			{
				return new BError( "Invalid parameter type" );
			}
		}
		//passed new account name and password
		else if( ex.numParams() == 2 )
		{
			const String* nmstr;
			const String* pwstr;
			if (ex.getStringParam( 0, nmstr ) && 
				ex.getStringParam( 1, pwstr ))
			{
				if(nmstr->value().empty())
					return new BError( "Account name must not be empty." );
				obj_->name_ = nmstr->value();
				//this is the same as the "setpassword" code above
				if(config.retain_cleartext_passwords)
					obj_->password_ = pwstr->value();

				std::string temp;
				MD5_Encrypt(obj_->name_ + pwstr->value(),temp);
				obj_->passwordhash_ = temp; //MD5
			}
			else
			{
				return new BError( "Invalid parameter type" );
			}			
		}
		else if ((ex.numParams() == 1) && !config.retain_cleartext_passwords )
			return new BError( "Usage: account.SetName(name,pass) if RetainCleartextPasswords is off." );
		else
			return new BError( "account.SetName needs at least 1 parameter." );
		break;
	///
	/// account.GetProp( propname : string ) : gets a custom account property
	///	 returns Error( "Property not found" ) if property does not exist.
	///
	///
	/// account.SetProp( propname : string, propval : packable ) : sets a custom account property
	///
	///
	/// account.EraseProp( propname : string ) : erases a custom account property.
	///
	///
	/// account.PropNames() : array of property names
	///
	case MTH_GETPROP:
	case MTH_SETPROP:
	case MTH_ERASEPROP:
	case MTH_PROPNAMES:
		result = CallPropertyListMethod_id( obj_->props_, id, ex, changed );
		if (result && !changed)
			return result;
		break;
	case MTH_SETDEFAULTCMDLEVEL:
	{
		if ( ex.numParams() != 1 )
			return new BError("account.SetDefaultCmdLevel(int) requires a parameter.");

		long cmdlevel;
		if ( !ex.getParam(0, cmdlevel) )
			return new BError("Invalid parameter type.");
		else if ( cmdlevel >= long(cmdlevels2.size()) )
			cmdlevel = cmdlevels2.size()-1;

		obj_->default_cmdlevel_ = char(cmdlevel);

		break;
	}
	///
	/// account.GetCharacter( index : 1..5 ) : retrieve a reference to a character belonging to this account.
	///	This reference may be used even if the character is offline.
	case MTH_GETCHARACTER:
		if (ex.numParams()!=1)
			return new BError( "account.GetCharacter(index) requires a parameter." );

		if (!ex.getParam( 0, index, 1, config.character_slots ))
			return NULL;
		chr = obj_->get_character( index-1 );

		if (chr == NULL)
			return new BError( "No such character on this account" );
		return new EOfflineCharacterRefObjImp( chr );
	///
	/// account.DeleteCharacter( index : 1..5 ) : delete a character
	///  character to be deleted cannot be logged in.
	///
	case MTH_DELETECHARACTER:
		if (ex.numParams()!=1)
			return new BError( "account.DeleteCharacter(index) requires a parameter." );

		if (!ex.getParam( 0, index, 1, config.character_slots ))
			return NULL;
		chr = obj_->get_character( index-1 );

		if (chr == NULL)
			return new BError( "No such character on this account" );
		if (chr->client != NULL || chr->logged_in)
			return new BError( "That character is in use" );
		
		if (can_delete_character( chr, DELETE_BY_SCRIPT ))
		{
			call_ondelete_scripts( chr );
			delete_character( obj_.Ptr(), chr, index-1 );
		}
		else
			return new BError( "CanDelete blocks Character deletion." );
		
		break;
	///
	/// account.Set_UO_Expansion( string ) : recognized values: ML, SE, AOS, LBR, T2A (default)
	///  this determines what flag is sent with packet 0xB9 during login.
	///
	case MTH_SET_UO_EXPANSION:
		if (ex.numParams()!=1)
			return new BError( "account.Set_UO_Expansion(string) requires a parameter." );
		
		const String* expansion_str;
		if (ex.getStringParam( 0, expansion_str ))
		{
			if ( expansion_str->value().empty() || (expansion_str->value()=="KR") ||
			   (expansion_str->value()=="ML") || (expansion_str->value()=="SE") ||
			   (expansion_str->value()=="AOS") || (expansion_str->value()=="LBR") ||
			   (expansion_str->value()=="T2A") )
			{
				obj_->uo_expansion_ = expansion_str->value();
				if (obj_->active_character)
					send_feature_enable(obj_->active_character->client);
			}
			else
				return new BError( "Invalid Parameter Value. Supported Values: \"\", T2A, LBR, AOS, SE, ML, KR" );
		}
		else
			return new BError("Invalid Parameter Type");
		break;
	///	
	/// account.Delete() : delete this account
	///
	case MTH_DELETE:
		if (ex.numParams() == 0)
		{
			int result = delete_account(obj_->name());
			if (result == -1)
				return new BError( "You must delete all Character first." );
			else if (result == -2) // Should never happen ;o)
				return new BError( "Invalid Account Name." );
		}
		else
			return new BError( "account.Delete() doesn't take parameters." );
		break;
	///
	/// account.Split( newacctname : string, index : 1..5 ) : create a new account and move character to it
	///
	case MTH_SPLIT:
		if( ex.numParams() == 2 )
		{
			const String* acctname;
			long index;
			if (ex.getStringParam( 0, acctname ) && 
				ex.getParam( 1, index, 1, config.character_slots ))
			{
				if (acctname->value().empty())
					return new BError( "Account name must not be empty." );
				
				if (find_account( acctname->data() ))
					return new BError( "Account already exists." );
				
				Character* chr = obj_->get_character( index-1 );
				
				if (chr == NULL)
					return new BError( "No such character on this account." );
				if (chr->client != NULL || chr->logged_in)
					return new BError( "That character is in use." );
				
				Account* account = duplicate_account( obj_->name_, acctname->value() );
				if (account != NULL)
				{
					obj_->clear_character( index-1 );
					chr->acct.set( account );
					account->set_character( 0, chr );
				}
				else
					return new BError( "Was impossible to create new Account." );
			}
			else
				return new BError( "Invalid parameter type." );
		}
		else
			return new BError( "account.Split requires two parameters." );
		break;
	///
	/// account.Move_Char( destacctname : string, index : 1..5 ) : move character from this account to destaccount
	///
	case MTH_MOVE_CHAR:
		if( ex.numParams() == 2 )
		{
			const String* acctname;
			long index;
			if (ex.getStringParam( 0, acctname ) && 
				ex.getParam( 1, index, 1, config.character_slots ))
			{
				if (acctname->value().empty())
					return new BError( "Account name must not be empty." );
				
				Account* account = find_account( acctname->data() );
				if (account == NULL)
					return new BError( "Account doesn't exists." );
				
				Character* chr = obj_->get_character( index-1 );
				
				if (chr == NULL)
					return new BError( "No such character on this account." );
				if (chr->client != NULL || chr->logged_in)
					return new BError( "That character is in use." );

				int charid = account->getnextfreeslot();
				if (charid != -1)
				{
					obj_->clear_character( index-1 );
					chr->acct.set( account );
					account->set_character( charid-1, chr );
				}
				else
					return new BError( "Account is full." );
			}
			else
				return new BError( "Invalid parameter type." );
		}
		else
			return new BError( "account.Move_Char requires two parameters." );
		break;
	case MTH_ADD_CHARACTER:
	{
		long index;
		if ( !ex.getParam(0, index, 0, config.character_slots) )
			return new BError("Account.AddCharacter() requires one parameter.");

		if ( index <= 0 )
		{
			index = obj_->getnextfreeslot();
			if ( index == -1 )
				return new BError("Account has no free character slots.");
		}
		
		// We take in 1-5 to match account.GetCharacter()
		// Internally it uses it as 0-4
		index--;

		if ( obj_->get_character(index) )
		{
			return new BError("That character slot is already in use.");
		}
		
		result = new BLong(index+1);
		Account* acct = find_account(obj_->name_.c_str());
		createchar2(acct, unsigned(index));

		break;
	}
	default:
		return NULL;
	}
	
	// if any of the methods hit & worked, we'll come here
	write_account_data();
	return result?result:new BLong(1);
}


///
/// [1] Account Scripting Object Methods
///
///	 All methods except GetProp and GetCharacter return 1 on success
///	 All methods except GetProp and GetCharacter write the accounts.txt file on success.
///	 All methods return Error("Not enough parameters") if too few parameters were passed.
///	 All methods return Error("Invalid parameter type") if the wrong type was passed.
///
BObjectImp* AccountObjImp::call_method( const char* methodname, Executor& ex )
{
	ObjMethod* objmethod = getKnownObjMethod(methodname);
	if( objmethod != NULL )
		return this->call_method_id(objmethod->id, ex);
	else
		return NULL;
}

///
/// [2] Account Scripting Object Members
///	 MemberName	  Type		Access
///	 name			string	  read-only
///	 enabled		 boolean	 read-only
///	 banned		  boolean	 read-only
/// 

BObjectRef AccountObjImp::get_member_id( const int id ) //id test
{
	switch(id)
	{
		case MBR_NAME: 
			return BObjectRef( new String( obj_->name() ) ); 
			break;
		case MBR_ENABLED:
			return BObjectRef( new BLong( obj_->enabled() ) );
			break;
		case MBR_BANNED: 
			return BObjectRef( new BLong( obj_->banned() ) );
			break;
		case MBR_USERNAMEPASSWORDHASH: 
			return BObjectRef( new String( obj_->passwordhash() ) );
			break;
		case MBR_UO_EXPANSION: 
			return BObjectRef( new String( obj_->uo_expansion() ) );
			break;
		case MBR_DEFAULTCMDLEVEL: 
			return BObjectRef(new BLong(obj_->default_cmdlevel_));
			break;
		default: 
			return BObjectRef( UninitObject::create() );
			break;
	}
}

BObjectRef AccountObjImp::get_member( const char* membername )
{
	ObjMember* objmember = getKnownObjMember(membername);
	if( objmember != NULL )
		return this->get_member_id(objmember->id);
	else
		return BObjectRef( UninitObject::create() );
}

