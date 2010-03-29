/*
History
=======
2006/06/15 Austin:    Removed .set .priv and .priv
2006/05/30 Shinigami: fixed a smaller mistype in textcmd_startlog()
                      set correct time stamp in textcmd_startlog() and textcmd_stoplog()
2009/09/03 MuadDib:   Relocation of account related cpp/h
2009/09/10 MuadDib:   Cleanup of depreciated internal text commands.
2009/10/17 Turley:    check for priv "plogany" enabled instead of existence - Tomi

Notes
=======

*/

#include "../clib/stl_inc.h"

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include "../bscript/berror.h"
#include "../bscript/impstr.h"

#include "../clib/cfgelem.h"
#include "../clib/cfgfile.h"
#include "../clib/cistring.h"
#include "../clib/endian.h"
#include "../clib/esignal.h"
#include "../clib/fileutil.h"
#include "../clib/logfile.h"
#include "../clib/opnew.h"
#include "../clib/stlutil.h"
#include "../clib/strutil.h"
#include "../clib/threadhelp.h"
#include "../clib/unicode.h"

#include "../plib/realm.h"

#include "accounts/account.h"
#include "action.h"
#include "allocd.h"
#include "item/armor.h"
#include "mobile/charactr.h"
#include "network/client.h"
#include "cmdlevel.h"
#include "item/itemdesc.h"
#include "los.h"
#include "menu.h"
#include "npc.h"
#include "module/osmod.h"
#include "pktboth.h"
#include "../plib/pkg.h"
#include "polcfg.h"
#include "polclock.h"
#include "polopt.h"
#include "polsem.h"
#include "profile.h"
#include "realms.h"
#include "schedule.h"
#include "scrsched.h"
#include "scrstore.h"
#include "target.h"
#include "tmpvars.h"
#include "uobject.h"
#include "ufunc.h"
#include "ufuncstd.h"
#include "uobjcnt.h"
#include "uobjhelp.h"
#include "module/uomod.h"
#include "uoexec.h"
#include "uofile.h"
#include "uoscrobj.h"
#include "uvars.h"
#include "uworld.h"
#include "item/weapon.h"

#include "fnsearch.h"


typedef void (*TextCmdFunc)(Client*);
typedef map<string,TextCmdFunc, ci_cmp_pred> TextCmds;
TextCmds textcmds;

class wordicmp : public less<string>
{
public:
	bool operator()(const string& lhs, const string& rhs) const;
};
bool wordicmp::operator ()(const string& lhs, const string& rhs) const
{
	int len = std::min( lhs.size(), rhs.size() );
	
	return (strnicmp( lhs.c_str(), rhs.c_str(), len ) < 0);
}
typedef void (*ParamTextCmdFunc)(Client*, const char*);
typedef map<string,ParamTextCmdFunc,wordicmp> ParamTextCmds;
//wordicmp p_wordicmp;
ParamTextCmds paramtextcmds;

void register_command( const char *cmd, TextCmdFunc f )
{
	textcmds.insert( TextCmds::value_type( cmd, f ) );
}
void register_command( const char *cmd, ParamTextCmdFunc f )
{
	paramtextcmds.insert( ParamTextCmds::value_type( cmd, f ) );
}

bool FindEquipTemplate( const char* template_name,
						ConfigElem& elem )
{
	try {
		ConfigFile cf( "config/equip.cfg" );
		while (cf.read( elem ))
		{
			if (!elem.type_is( "Equipment" ))
				continue;
			const char* rest = elem.rest();
			if (rest == NULL || *rest == '\0')
				continue;
			if (stricmp( rest, template_name ) == 0)
				return true;
		}
		return false;
	}
	catch( ... )
	{
		return false;
	}
}

BObjectImp* equip_from_template( Character* chr, const char* template_name )
{
	ConfigElem elem;
	if (!FindEquipTemplate( template_name, elem ))
	{
		return new BError( "Equipment template not found" );
	}

	std::string tmp;
	while (elem.remove_prop( "Equip", &tmp ) || 
		   elem.remove_prop( "Weapon", &tmp ) ||
		   elem.remove_prop( "Armor", &tmp ))
	{
		ISTRINGSTREAM is( tmp );
		string objtype_str;
		if (is >> objtype_str)
		{
			unsigned short objtype;
			const char* ot_str = objtype_str.c_str();
			if (isdigit( *ot_str ))
			{
				objtype = static_cast<unsigned short>(strtoul( ot_str, NULL, 0 ));
			}
			else
			{
				objtype = get_objtype_byname( ot_str );
				if (!objtype)
				{
					cerr << "Blech! Can't find '" << ot_str << "' named in equip.cfg" << endl;
					continue;
				}
			}
			string color_str;
			unsigned short color = 0;
			if (is >> color_str)
			{
				color = static_cast<unsigned short>(strtoul( color_str.c_str(), NULL, 0 ));
			}
			Item* it = Item::create( objtype );
			if (it != NULL)
			{
				color &= VALID_ITEM_COLOR_MASK;
				it->color = color;
				it->color_ext = ctBEu16( color );
				it->layer = tilelayer( it->graphic );
				it->realm = chr->realm;
					// FIXME equip scripts, equiptest scripts
				if (chr->equippable( it ))
				{
					chr->equip( it );
					update_item_to_inrange( it );
				}
				else
				{
					it->destroy();
				}
			}
		}
	}
	return new BLong(1);
}

// FIXME should do a command handler table like for messages received.
void send_client_char_data( Character *chr, Client *client );

void send_move_if_inrange2( Character *chr, Client *client )
{
	if (client->ready &&
		client->chr &&
		client->chr != chr &&
		inrange( client->chr, chr ))
	{
		send_move( client, chr );
	}
}

void textcmd_flag1( Client* client, const char* text )
{
	tmp_flag1 = (u8) strtoul( text, NULL, 16 );
	ForEachMobileInRange( client->chr->x, client->chr->y, client->chr->realm, RANGE_VISUAL,
						  send_move_if_inrange2, client );
}
	
void textcmd_flag2( Client* client, const char* text )
{
	tmp_flag2 = (u8) strtoul( text, NULL, 16 );
	ForEachMobileInVisualRange( client->chr, send_move_if_inrange2, client );
}

void textcmd_resendchars( Client* client )
{
	ForEachMobileInVisualRange( client->chr, send_client_char_data, client );
}

void textcmd_shutdown( Client* client )
{
	exit_signalled = 1;
}

void handle_ident_cursor( Character* chr, PKTBI_6C* msgin )
{
	if (chr->client != NULL)
	{
		char s[80];
		sprintf( s, "Serial: 0x%8.08lX, %ld, ObjType 0x%4.04X", 
				static_cast<unsigned long>(cfBEu32(msgin->selected_serial)), 
				static_cast<signed long>(cfBEu32(msgin->selected_serial)),
				cfBEu16(msgin->graphic) );
		send_sysmessage( chr->client, s );
	}
}
FullMsgTargetCursor ident_cursor( handle_ident_cursor );
void textcmd_ident( Client* client )
{
	send_sysmessage( client, "Select something to identify." );
	ident_cursor.send_object_cursor( client );
}

void textcmd_listarmor( Client* client )
{
	client->chr->showarmor();
}

#include "repsys.h"
std::string timeoutstr( polclock_t at )
{
	polclock_t ticks = at - polclock();
	int seconds = ticks / POLCLOCKS_PER_SEC;
	return decint( seconds ) + " seconds";
}

///
/// Internal Command: .i_repdata
///	 Show Reputation System Data for a Targetted Mobile
///	 Displays:
///		 Murderer status
///		 Criminal status and timeout
///		 LawfullyDamaged status and timeouts
///		 AggressorTo status and timeouts
///		 ToBeReportable list
///		 Reportable list
///		 RepSystem Task status
///
void RepSystem::show_repdata( Client* client, Character* mob )
{
	if (mob->is_murderer())
	{
		send_sysmessage( client, "Mobile is a murderer." );
	}
	else if (mob->is_criminal())
	{
		send_sysmessage( client, 
						 "Mobile is criminal for " + timeoutstr( mob->criminal_until_ ) 
						   + " [" + decint(mob->criminal_until_) + "]" );
	}

	for( Character::MobileCont::const_iterator itr = mob->aggressor_to_.begin();
		 itr != mob->aggressor_to_.end(); ++itr )
	{
		send_sysmessage( client, 
						 "Aggressor to " + (*itr).first->name() 
						   + " for "  + timeoutstr( (*itr).second ) 
						   + " [" + decint( (*itr).second ) + "]" );
	}

	for( Character::MobileCont::const_iterator itr = mob->lawfully_damaged_.begin();
		 itr != mob->lawfully_damaged_.end(); ++itr )
	{
		send_sysmessage( client, 
						 "Lawfully Damaged " + (*itr).first->name() 
						   + " for "  + timeoutstr( (*itr).second ) 
						   + " [" + decint( (*itr).second ) + "]" );
	}

	for( Character::ToBeReportableList::const_iterator itr = mob->to_be_reportable_.begin();
		 itr != mob->to_be_reportable_.end(); ++itr )
	{
		USERIAL serial = (*itr);
		send_sysmessage( client, 
						 "ToBeReportable: " + hexint(serial) );
	}

	for( Character::ReportableList::const_iterator itr = mob->reportable_.begin();
		 itr != mob->reportable_.end(); ++itr )
	{
		 const reportable_t& rt = (*itr);
		 send_sysmessage( client, 
						 "Reportable: " + hexint(rt.serial) 
						   + " at " + decint(rt.polclock) );
	}
	
	if (mob->repsys_task_ != NULL)
		send_sysmessage( client, 
						 "Repsys task is active, runs in " + timeoutstr( mob->repsys_task_->next_run_clock() ) 
						   + " [" + decint( mob->repsys_task_->next_run_clock() ) + "]" );
}

void show_repdata( Character* looker, Character* mob )
{
	RepSystem::show_repdata( looker->client, mob );
}
NoLosCharacterCursor repdata_cursor( show_repdata );

void textcmd_repdata( Client* client )
{
	send_sysmessage( client, "Please target a mobile to display repdata for." );
	repdata_cursor.send_object_cursor( client );
}

void start_packetlog( Character* looker, Character* mob )
{
	if (mob->connected) // gotta be connected to get packets right?
	{
		if (mob->client->fpLog == NULL)
		{
			string filename = "log/";
			filename += mob->client->acct->name();
			filename += ".log";

			mob->client->fpLog = fopen( filename.c_str(), "a+t" );
			if (mob->client->fpLog != NULL)
			{
				time_t now = time(NULL);
				fprintf( mob->client->fpLog, "Log opened at %s\n", asctime( localtime( &now ) ) );
				send_sysmessage( looker->client, "I/O log file opened for " + mob->name());
			}
			else
			{
				send_sysmessage( looker->client, "Unable to open I/O log file for " + mob->name());
			}
		}			
	}
}
NoLosCharacterCursor startlog_cursor( start_packetlog );

void textcmd_startlog( Client* client )
{
    if (client->chr->can_plogany())
	{
		send_sysmessage( client, "Please target a player to start packet logging for." );
		startlog_cursor.send_object_cursor( client );
	}
	else
	{
		if (client->fpLog == NULL)
		{
			string filename = "log/";
			filename += client->acct->name();
			filename += ".log";

			client->fpLog = fopen( filename.c_str(), "a+t" );
			if (client->fpLog != NULL)
			{
				time_t now = time(NULL);
				fprintf( client->fpLog, "Log opened at %s\n", asctime( localtime( &now ) ) );
				send_sysmessage( client, "I/O log file opened." );
			}
			else
			{
				send_sysmessage( client, "Unable to open I/O log file." );
			}
		}
	}
}

void stop_packetlog( Character* looker, Character* mob )
{
	if (mob->connected) // gotta be connected to already have packets right?
	{
		if (mob->client->fpLog != NULL)
		{
			time_t now = time(NULL);
			fprintf( mob->client->fpLog, "Log closed at %s\n", asctime( localtime( &now ) ) );
	
			fclose( mob->client->fpLog );
			mob->client->fpLog = NULL;
			send_sysmessage( looker->client, "I/O log file closed for " + mob->name());
		}
		else
		{
			send_sysmessage( looker->client, "Packet Logging not enabled for " + mob->name());
		}
	}
}
NoLosCharacterCursor stoplog_cursor( stop_packetlog );

void textcmd_stoplog( Client* client )
{
	if (client->chr->can_plogany())
	{
		send_sysmessage( client, "Please target a player to stop packet logging for." );
		stoplog_cursor.send_object_cursor( client );
	}
	else
	{
		if (client->fpLog != NULL)
		{
			time_t now = time(NULL);
			fprintf( client->fpLog, "Log closed at %s\n", asctime( localtime( &now ) ) );
	
			fclose( client->fpLog );
			client->fpLog = NULL;
			send_sysmessage( client, "I/O log file closed." );
		}
		else
		{
			send_sysmessage( client, "Packet Logging not enabled." );
		}
	}
}

void textcmd_orphans( Client* client )
{
	OSTRINGSTREAM os;
	os << "Unreaped orphans: " << unreaped_orphans;

	send_sysmessage( client, OSTRINGSTREAM_STR(os) );

	OSTRINGSTREAM os2;
	os2 << "EChrRef count: " << uobj_count_echrref;
	send_sysmessage( client, OSTRINGSTREAM_STR(os2) );
}

void list_scripts();
void textcmd_list_scripts( Client* )
{
	list_scripts();
}
void list_crit_scripts();
void textcmd_list_crit_scripts( Client* )
{
	list_crit_scripts();
}
void textcmd_procs( Client* client )
{
	send_sysmessage( client, "Process Information:" );

	send_sysmessage( client, "Running: " + decint( (unsigned int)(runlist.size()) ) );
	send_sysmessage( client, "Blocked: " + decint( (unsigned int)(holdlist.size() )) );
}

void textcmd_log_profile( Client* client )
{
	log_all_script_cycle_counts( false );
	send_sysmessage( client, "Script profile written to logfile" );
}

void textcmd_log_profile_clear( Client* client )
{
	log_all_script_cycle_counts( true );
	send_sysmessage( client, "Script profile written to logfile and cleared" );
}

void textcmd_heapcheck( Client* client )
{
	PrintAllocationData();
	PrintHeapData();
}

void textcmd_threads( Client* client )
{
	string s = "Child threads: " + decint( threadhelp::child_threads );
	send_sysmessage( client, s );
}

void textcmd_constat( Client* client )
{
	int i = 0;
	send_sysmessage( client, "Connection statuses:" );
	for( Clients::const_iterator itr = clients.begin(); itr != clients.end(); ++itr )
	{  
		OSTRINGSTREAM os;
		os << i << ": " << (*itr)->status() << " ";
		send_sysmessage( client, OSTRINGSTREAM_STR(os) );
		++i;
	}
}


bool check_single_zone_item_integrity(int, int, Realm*);
void textcmd_singlezone_integ_item( Client* client)
{
	unsigned short wx, wy;
	zone_convert( client->chr->x, client->chr->y, wx, wy, client->chr->realm );
	bool ok = check_single_zone_item_integrity(wx,wy, client->chr->realm);
	if (ok)
		send_sysmessage( client, "Item integrity checks out OK!" );
	else
		send_sysmessage( client, "Item integrity problems detected. ");
}

bool check_item_integrity();
void textcmd_integ_item( Client* client )
{
	bool ok = check_item_integrity();
	if (ok)
		send_sysmessage( client, "Item integrity checks out OK!" );
	else
		send_sysmessage( client, "Item integrity problems detected.  Check logfile" );
}
void check_character_integrity();
void textcmd_integ_chr( Client* client )
{
	check_character_integrity();
}

string get_textcmd_help( Character* chr, const char* cmd )
{
	const char* t = cmd;
	while (*t)
	{
		if (!isalpha(*t) && (*t != '_'))
		{
			//cout << "illegal command char: as int = " << int(*t) << ", as char = " << *t << endl;
			return "";
		}
		++t;
	}

	string upp(cmd);
	mkupper( upp );
	if (upp == "AUX" || upp == "CON" || upp == "PRN" || upp == "NUL")
		return "";

	for( int i = chr->cmdlevel; i >= 0; --i )
	{
		CmdLevel& cmdlevel = cmdlevels2[i];
		for( unsigned diridx = 0; diridx < cmdlevel.searchlist.size(); ++diridx )
		{
			string filename;

			Package* pkg = cmdlevel.searchlist[diridx].pkg;
			filename = cmdlevel.searchlist[diridx].dir + cmd + string(".txt");
			if (pkg)
				filename = pkg->dir() + filename;

			//cout << "Searching for " << filename << endl;
			if (FileExists( filename.c_str() ))
			{
				// cout << "Found " << filename << endl;
				string result;
				ifstream ifs( filename.c_str(), ios::binary );
				char temp[ 64 ];
				do {
					ifs.read( temp, sizeof temp );
					if (ifs.gcount())
						result.append( temp, static_cast<size_t>(ifs.gcount()) );
				} while (!ifs.eof());
				return result;
			}
		}
	}
	return "";
}

bool start_textcmd_script( Client* client, const char *text,
						   const u16* wtext = NULL, const char* lang = NULL )
{
	string scriptname;
	string params;
	const char* t = strchr( text, ' ' );
	if (t != NULL)
	{
		scriptname = string(text, t);
		params = t + 1;
	}
	else
	{
		scriptname = text;
		params = "";
	}
	
	//cout << "scriptname='" << scriptname << "', params='" << params << "'" << endl;
	
	t = scriptname.c_str();
	while (*t)
	{
		if (!isalnum(*t) && (*t != '_'))
		{
			//cout << "illegal command char: as int = " << int(*t) << ", as char = " << *t << endl;
			return false;
		}
		++t;
	}

	string upp(scriptname);
	mkupper( upp );
	if (upp == "AUX" || upp == "CON" || upp == "PRN" || upp == "NUL")
		return false;

	for( int i = client->chr->cmdlevel; i >= 0; --i )
	{
		// cout << "checking cmdlevel " << i << endl;
		CmdLevel& cmdlevel = cmdlevels2[i];
		for( unsigned diridx = 0; diridx < cmdlevel.searchlist.size(); ++diridx )
		{
			ScriptDef sd;
			Package* pkg = cmdlevel.searchlist[diridx].pkg;
			if (pkg)
				sd.quickconfig( pkg, cmdlevel.searchlist[diridx].dir + scriptname + ".ecl" );
			else
				sd.quickconfig( cmdlevel.searchlist[diridx].dir + scriptname + ".ecl" );
			if (!sd.exists())
				continue;

			// cout << "Searching for " << sd.name() << endl;
			ref_ptr<EScriptProgram> prog = find_script2( sd, 
														 false, // don't complain if not found
														 config.cache_interactive_scripts );
			if (prog.get() != NULL)
			{
				// Unicode stuff

				auto_ptr<UOExecutor> ex(create_script_executor());
				if (prog->haveProgram)
				{
					if ( wtext && lang )
					{
						ObjArray* arr;
						size_t woffset;
						bool UCconv = false;
						// calc offset to either the null after the
						// scriptname (+1) or the start of the param (+2)
						woffset = static_cast<size_t>(scriptname.length()+(params==""?1:2));
						unsigned wtlen = 0; // wcslen(static_cast<const wchar_t*>(wtext+woffset))

						// Need to calc length with a loop (coz linux is a PITA with 4-byte unicode!)
						while( *(wtext+woffset+wtlen) )
							++wtlen;
						UCconv = convertUCtoArray(wtext+woffset, arr,
													wtlen,
													true); // convert back with ctBEu16()
						if ( UCconv )
						{
							ex->pushArg( new String( lang ) );
							ex->pushArg( arr );
						}
						else
							ex->pushArg( new BError("Invalid Unicode speech received.") );
					}
					ex->pushArg( new String( params ) );
					ex->pushArg( new ECharacterRefObjImp( client->chr ) );
				}

				UOExecutorModule* uoemod = new UOExecutorModule( *ex );
				ex->addModule( uoemod );
				ex->os_module->priority = 100;

				if (ex->setProgram( prog.get() ))
				{
					uoemod->controller_.set( client->chr ); //DAVE added 12/04, let character issuing textcmd be the script controller
					schedule_executor( ex.release() );
					return true;
				}
				else
				{
					cerr << "script " << scriptname << ": setProgram failed" << endl;
					// TODO: it seems to keep looking until it finds one it can use..guess this is okay?
				}
			}
		}

	}
	return false;
}

bool process_command( Client *client, const char *text,
					  const u16* wtext /*NULL*/, const char* lang /*NULL*/)
{
	static int init;
	if (!init)
	{
		init = 1;
		register_command( "armor", textcmd_listarmor );
		register_command( "constat", textcmd_constat );
		register_command( "heapcheck", &textcmd_heapcheck );
		register_command( "i_repdata", textcmd_repdata );
		register_command( "t_ident", textcmd_ident );
		register_command( "integ_item", textcmd_integ_item );
		register_command( "integ_chr", textcmd_integ_chr );
		register_command( "i_s_item_integ",textcmd_singlezone_integ_item); //davedebug
		register_command( "list_crit", &textcmd_list_crit_scripts );
		register_command( "list_scripts", &textcmd_list_scripts );
		register_command( "log_profile", &textcmd_log_profile );
		register_command( "log_profile_clear", &textcmd_log_profile_clear );
		register_command( "orphans", &textcmd_orphans );
		register_command( "procs", &textcmd_procs );
		register_command( "resendchars", &textcmd_resendchars );
		register_command( "shutdown", &textcmd_shutdown );
		register_command( "startlog", &textcmd_startlog );
		register_command( "stoplog", &textcmd_stoplog );
		register_command( "threads", &textcmd_threads );
		register_command( "flag1 ", &textcmd_flag1 );
		register_command( "flag2 ", &textcmd_flag2 );
	}

	++text; // skip the "/" or "."

	if (start_textcmd_script( client, text, wtext, lang ))
		return true;

	//cout << "checking for builtin commands" << endl;
	if (client->chr->cmdlevel >= cmdlevels2.size()-2)
	{
		TextCmds::iterator itr2 = textcmds.find( text );
		if (itr2 != textcmds.end())
		{
			TextCmdFunc f = (*itr2).second;
			(*f)(client);
			return true;
		}

		ParamTextCmds::iterator itr1 = paramtextcmds.find( text );
		if (itr1 != paramtextcmds.end())
		{
			ParamTextCmdFunc f = (*itr1).second;
			(*f)(client, text + (*itr1).first.size());
			return true;
		}
	}

	return false;
}
