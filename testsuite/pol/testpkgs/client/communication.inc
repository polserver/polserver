use os;
use attributes;
use vitals;

// see pyuo/brain.py method typestr()
const EVT_INIT := "init";
const EVT_EXIT := "exit";
const EVT_HP_CHANGED := "hp_changed";
const EVT_MANA_CHANGED := "mana_changed";
const EVT_STAM_CHANGED := "stam_changed";
const EVT_SPEECH := "speech";
const EVT_NOTORIETY := "notoriety";
const EVT_MOVED := "moved";
const EVT_NEW_MOBILE := "new_mobile";
const EVT_NEW_ITEM := "new_item";
const EVT_REMOVED_OBJ := "removed_obj";
const EVT_LIST_OBJS := "list_objs";
const EVT_LIST_EQUIPPED_ITEMS := "list_equipped_items";
const EVT_OPEN_BACKPACK := "open_bp";
const EVT_TARGET := "target";
const EVT_DOUBLE_CLICK := "double_click";
const EVT_LIFT_ITEM := "lift_item";
const EVT_MOVE_ITEM_REJECTED := "move_item_rejected";
const EVT_DROP_ITEM := "drop_item";
const EVT_WEAR_ITEM := "wear_item";
const EVT_BOAT_MOVE := "boat_move";
const EVT_DROP_APPROVED := "drop_approved";
const EVT_DISABLE_ITEM_LOGGING := "disable_item_logging";
const EVT_NEW_SUBSERVER := "new_subserver";
const EVT_BOAT_MOVED := "boat_moved";
const EVT_OWNCREATE := "owncreate";
const EVT_GUMP := "gump";
const EVT_AOS_TOOLTIP := "aos_tooltip";
const EVT_OPEN_PAPERDOLL := "open_paperdoll";
const EVT_AUTO_DELETE_OBJS := "auto_delete_objs";
const EVT_ATTACK := "attack";
const EVT_WAR_MODE := "war_mode";
const EVT_FIGHT_OCCURING := "fight_occuring";
const EVT_STATUS_BAR := "status_bar";
const EVT_TRADE := "trade";
const EVT_CLILOC := "cliloc";
const EVT_PARTY := "party";

// actions of the secure trade window packet, see PKTBI_6F in the core
const TRADE_ACTION_INIT := 0;
const TRADE_ACTION_CANCEL := 1;
const TRADE_ACTION_STATUS := 2;

// the party subcommands of the 0xBF 0x0006 packet, see PKTBI_BF_06 in the core.
// The client sends ADD/REMOVE/MEMBER_MSG/MSG/LOOT_PERMISSION/ACCEPT/DECLINE, the
// server answers with ADD (the member list), REMOVE (who left, then who is left),
// the two messages, and INVITE_MEMBER
const PARTYCMD_ADD := 0x01;
const PARTYCMD_REMOVE := 0x02;
const PARTYCMD_MEMBER_MSG := 0x03;
const PARTYCMD_MSG := 0x04;
const PARTYCMD_LOOT_PERMISSION := 0x06;
const PARTYCMD_INVITE_MEMBER := 0x07;
const PARTYCMD_ACCEPT_INVITE := 0x08;
const PARTYCMD_DECLINE_INVITE := 0x09;

// the cliloc numbers the party system talks in, see party_cfg.h
const CLP_REJOINED := 1005437;
const CLP_ADD_YOURSELF := 1005439;
const CLP_ALREADY_YOUR_PARTY := 1005440;
const CLP_ALREADY_IN_A_PARTY := 1005441;
const CLP_ADD_LIVING := 1005442;
const CLP_IGNORE_OFFER := 1005444;
const CLP_ADDED := 1005445;
const CLP_ALLOW_LOOT := 1005447;
const CLP_PREVENT_LOOT := 1005448;
const CLP_DISBANDED := 1005449;
const CLP_LAST_PERSON := 1005450;
const CLP_REMOVED := 1005451;
const CLP_PLAYER_REMOVED := 1005452;
const CLP_ADD_NO_LEADER := 1005453;
const CLP_WHO_TO_ADD := 1005454;
const CLP_WHO_TO_REMOVE := 1005455;
const CLP_INVITE := 1008089;
const CLP_INVITED := 1008090;
const CLP_NOTIFY_DECLINE := 1008091;
const CLP_DECLINE := 1008092;
const CLP_JOINED := 1008094;
const CLP_MAX_SIZE := 1008095;
const CLP_NO_PARTY := 3000211;
const CLP_NO_INVITE := 3000222;
const CLP_TOO_LATE_DECLINE := 3000223;

// how the client is told to colour a mobile, see CHAR_HILITE_* in the core and
// the notoriety constants in pyuo's client.py
const NOTO_INNOCENT := 1;
const NOTO_FRIEND := 2;
// 3 is CHAR_HILITE_ATTACKABLE in the core, which pyuo calls "animal"
const NOTO_ATTACKABLE := 3;
const NOTO_CRIMINAL := 4;
const NOTO_ENEMY := 5;
const NOTO_MURDERER := 6;
const NOTO_INVUL := 7;

// the texttype of a speech packet, see TEXTTYPE_* in plib/uconst.h
const TEXTTYPE_NORMAL := 0x00;
const TEXTTYPE_SYS_BROADCAST := 0x01;
const TEXTTYPE_EMOTE := 0x02;
const TEXTTYPE_YOU_SEE := 0x06;
const TEXTTYPE_WHISPER := 0x08;
const TEXTTYPE_YELL := 0x09;
// the two the core never names: speech.cpp matches them as bare literals, and
// they are routed to a guild rather than to whoever is in range
const TEXTTYPE_GUILD := 0x0D;
const TEXTTYPE_ALLIANCE := 0x0E;

// what config/repsys.cfg colours a name with, see the NameColoring block there
const NAMECOLOR_GUILD_ENEMY := 44;
const NAMECOLOR_GUILD_ALLY := 68;
const NAMECOLOR_INNOCENT := 90;

// the serial and body the core sends a system message under, see send_sysmessage
const SYSMESSAGE_SERIAL := 0x01010101;
const SYSMESSAGE_MODEL := 0x0101;
const SYSMESSAGE_NAME := "System";

// the two text types the cliloc packet carries, see clfunc.cpp: 6 puts the
// message in the lower left corner, 7 above the object. Neither is spelled out
// in uconst.h the way the plain speech types are.
const CLILOC_TYPE_SYSTEM := 0x06;
const CLILOC_TYPE_ABOVE := 0x07;

// what a cliloc system message is attributed to, see send_sysmessage_cl - all
// ones rather than the marker serial of a plain system message. 0xFFFFFFFF does
// not fit an integer, so it is kept as the double the client event carries.
const CLILOC_SYSMESSAGE_SERIAL := 4294967295.0;
const CLILOC_SYSMESSAGE_MODEL := 0xFFFF;

function clientTestActive()
  var testclient := GetEnvironmentVariable( "POLCORE_TESTCLIENT" ) == "TRUE";
  if ( !testclient )
    return 0;
  endif
  return 1;
endfunction

// get client connection auxiliary scriptref and transfers ownership, meaning client events get send to the calling script
function getClientConnection()
  Set_Event_Queue_Size( 1000 ); // increase queue size to be sure we dont miss events
  var res := GetGlobalProperty( "#clientcon" );
  var proc := GetProcess( res );
  proc.sendevent( struct{ ownership := GetProcess() } );
  return proc;
endfunction

// wait for events of given types and of given client id
function waitForClient( id, types, timeout := 10 )
  while ( 1 )
    var ev := Wait_For_Event( timeout );
    if ( !ev )
      return ret_error( $"no signal received\n{GetProcess().stacktrace()}" );
    endif
    if ( ev.id != id || !( ev.type in types ) )
      continue;
    endif
    return ev;
  endwhile
endfunction

function waitForClients( types )
  // wait for events of given types of any client
  while ( 1 )
    var ev := Wait_For_Event( 10 );
    if ( !ev )
      return ret_error( $"no signal received\n{GetProcess().stacktrace()}" );
    endif
    if ( !( ev.type in types ) )
      continue;
    endif
    return ev;
  endwhile
endfunction

function disable_auto_delete( clientcon, id )
  clientcon.sendevent( struct{ todo := EVT_AUTO_DELETE_OBJS, arg := false, id := id } );
  while ( !waitForClient( id, { EVT_AUTO_DELETE_OBJS } ) )
  endwhile
endfunction

function enable_auto_delete( clientcon, id )
  clientcon.sendevent( struct{ todo := EVT_AUTO_DELETE_OBJS, arg := true, id := id } );
  while ( !waitForClient( id, { EVT_AUTO_DELETE_OBJS } ) )
  endwhile
endfunction
