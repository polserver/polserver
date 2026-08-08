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
// The server described an object that is nowhere near the client. The client discards it, so this
// is the only trace of it.
const EVT_OUT_OF_RANGE_OBJ := "out_of_range_obj";
const EVT_LIST_OBJS := "list_objs";
const EVT_LIST_EQUIPPED_ITEMS := "list_equipped_items";
const EVT_OPEN_BACKPACK := "open_bp";
const EVT_TARGET := "target";
const EVT_CANCEL_TARGET := "cancel_target";
const EVT_DOUBLE_CLICK := "double_click";
const EVT_LIFT_ITEM := "lift_item";
const EVT_MOVE_ITEM_REJECTED := "move_item_rejected";
const EVT_DROP_ITEM := "drop_item";
const EVT_WEAR_ITEM := "wear_item";
const EVT_BUY_ITEMS := "buy_items";
const EVT_SELL_ITEMS := "sell_items";
const EVT_RACE_CHANGE := "race_change";
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
const EVT_HOUSE_DESIGN := "house_design";
const EVT_HOUSE_EDIT := "house_edit";
const EVT_HOUSE_REV := "house_rev";
const EVT_SPELLBOOK := "spellbook";
const EVT_MAP := "map";
const EVT_MAP_PIN := "map_pin";

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

// what config/repsys.cfg colours a name with, see the NameColoring block there.
// Criminal and Attackable share a colour in the stock configuration, so only the
// notoriety byte tells those two standings apart.
const NAMECOLOR_MURDERER := 38;
const NAMECOLOR_GUILD_ENEMY := 44;
const NAMECOLOR_INVULNERABLE := 53;
const NAMECOLOR_GUILD_ALLY := 68;
const NAMECOLOR_INNOCENT := 90;
const NAMECOLOR_CRIMINAL := 905;
const NAMECOLOR_ATTACKABLE := 905;

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

// How long a wait for nothing in particular blocks. Wait_For_Event takes a
// Double down to a hundredth of a second, and a wait ends the moment an event
// arrives - so this is only the granularity of the idle waits, and the only
// thing it costs is how far past its deadline a wait for nothing runs. Waiting
// a whole second at a time is what made the negative assertions below cost a
// second more than they were asked for.
const POLL_INTERVAL := 0.1;

// The default window of the "nobody was told this" assertions. It has to stay
// comfortably above a round trip to the test client - that is all a wrongly
// sent message needs to show up - without being paid on every single call.
const QUIET_SECS := 0.75;

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

// Waits for a speech event of the given client carrying the given text. The
// clients hear each other and the shard talks at them, so the text is what tells
// the wanted event from the noise.
function wait_for_speech( id, text, timeout := 10 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_SPEECH or CInt( ev.id ) != id or ev.msg != text )
      continue;
    endif
    return ev;
  endwhile
  return ret_error( $"client {id} never heard \"{text}\" within {timeout}s" );
endfunction

// Asserts a client is told nothing carrying the given text.
//
// The whole window is always paid, so it is kept to the round trip a wrongly
// sent message would need rather than to a comfortable-looking couple of
// seconds. Most callers have already waited out something else the core sends,
// which proves it has sent everything it was going to.
function no_speech_for( id, text, timeout := QUIET_SECS )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_SPEECH or CInt( ev.id ) != id or ev.msg != text )
      continue;
    endif
    return ret_error( $"client {id} heard \"{text}\", which was not meant for it" );
  endwhile
  return 1;
endfunction

// Waits for a client to be told about its own character, which is how a test
// knows a move it asked for has reached that client. Bounded by the deadline
// rather than by each wait, so a stream of other events cannot stretch it.
function wait_for_own_create( id, timeout := 10 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( 1 );
    if ( !ev or ev.type != EVT_OWNCREATE or CInt( ev.id ) != id )
      continue;
    endif
    return 1;
  endwhile
  return ret_error( $"client {id} never announced itself within {timeout}s" );
endfunction

// Reads events until none have come for a while. Only for putting the queue
// back into a known state after something went wrong - it costs its whole
// window every time it is called.
function drain_events( quiet := QUIET_SECS )
  var deadline := ReadMillisecondClock() + quiet * 1000;
  while ( ReadMillisecondClock() < deadline )
    if ( Wait_For_Event( POLL_INTERVAL ) )
      deadline := ReadMillisecondClock() + quiet * 1000;
    endif
  endwhile
  return 1;
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

// Asks a client what it knows: the contents of one container when a serial is
// given, everything it has seen otherwise. Drops whatever is queued first, so
// the answer cannot be an older listing.
//
// A client answers this out of its own loop, without regard for the packets
// still on their way to it - the core hands those to a queue that a thread of
// its own writes out - so a listing can be older than the change it is being
// asked about. A caller looking for one object being there, or being gone, has
// to be able to ask again and must go through client_finds_obj() or
// client_lost_obj() rather than pick through the answer of a single listing.
function list_client_objs( clientcon, id, cont_serial := 0, timeout := 10 )
  Clear_Event_Queue();
  var todo := struct{ todo := "list_objects", id := id };
  if ( cont_serial )
    todo.+arg := cont_serial;
  endif
  clientcon.sendevent( todo );
  return waitForClient( id, { EVT_LIST_OBJS }, timeout );
endfunction

// Waits for an object to show up in a client's listing and hands its entry back.
function client_finds_obj( clientcon, id, serial, cont_serial := 0, timeout := 5 )
  var res := poll_client_listing( clientcon, id, serial, cont_serial, 1, timeout );
  if ( !res )
    return res;
  endif
  return res.entry;
endfunction

// Waits for an object to be gone from a client's listing.
function client_lost_obj( clientcon, id, serial, cont_serial := 0, timeout := 5 )
  var res := poll_client_listing( clientcon, id, serial, cont_serial, 0, timeout );
  if ( !res )
    return res;
  endif
  return 1;
endfunction

// Lists over and over until the object is there (wanted := 1) or gone
// (wanted := 0). A listing that already says so costs a single round trip.
function poll_client_listing( clientcon, id, serial, cont_serial, wanted, timeout )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  var answered := 0;

  while ( ReadMillisecondClock() < deadline )
    var ev := list_client_objs( clientcon, id, cont_serial, 1 );
    if ( !ev )
      continue;
    endif
    answered := 1;

    var entry := 0;
    foreach obj in ( ev.objs )
      // the client events come out of UnpackJson, so every number in them is a
      // Double
      if ( CInt( obj.serial ) == CInt( serial ) )
        entry := obj;
        break;
      endif
    endforeach
    if ( wanted == ( entry ? 1 : 0 ) )
      return struct{ entry := entry };
    endif
    Sleepms( POLL_INTERVAL * 1000 );
  endwhile

  var where := cont_serial ? $"the container 0x{cont_serial:x} of client {id}" : $"the object list of client {id}";
  if ( !answered )
    return ret_error( $"client {id} never answered with {where}" );
  elseif ( wanted )
    return ret_error( $"0x{serial:x} never showed up in {where} within {timeout}s" );
  endif
  return ret_error( $"0x{serial:x} is still in {where} after {timeout}s" );
endfunction

// Asks a client what it thinks a mobile is wearing. Drops whatever is queued
// first, so the answer cannot be an older listing.
//
// The same race as list_client_objs(): the client answers out of its own loop
// and knows nothing of the packets still on their way to it, so a listing taken
// straight after equipping something can be older than the equip. A caller
// asking whether a layer is filled, or empty, has to go through
// client_finds_layer() or client_lost_layer().
function list_client_equipment( clientcon, id, serial, timeout := 10 )
  Clear_Event_Queue();
  clientcon.sendevent( struct{ todo := "list_equipped_items", arg := serial, id := id } );
  return waitForClient( id, { EVT_LIST_EQUIPPED_ITEMS }, timeout );
endfunction

// Waits for a layer of a mobile's equipment to be filled as the client sees it,
// and hands that entry back.
function client_finds_layer( clientcon, id, serial, layer, timeout := 5 )
  var res := poll_client_equipment( clientcon, id, serial, layer, 1, timeout );
  if ( !res )
    return res;
  endif
  return res.entry;
endfunction

// Waits for a layer to be empty as the client sees it.
function client_lost_layer( clientcon, id, serial, layer, timeout := 5 )
  var res := poll_client_equipment( clientcon, id, serial, layer, 0, timeout );
  if ( !res )
    return res;
  endif
  return 1;
endfunction

// Lists over and over until the layer is filled (wanted := 1) or empty
// (wanted := 0). A listing that already says so costs a single round trip.
function poll_client_equipment( clientcon, id, serial, layer, wanted, timeout )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  var answered := 0;

  while ( ReadMillisecondClock() < deadline )
    var ev := list_client_equipment( clientcon, id, serial, 1 );
    if ( !ev )
      continue;
    endif
    answered := 1;

    var entry := 0;
    foreach item in ( ev.objs )
      // the client events come out of UnpackJson, so every number in them is a
      // Double
      if ( CInt( item.layer ) == layer )
        entry := item;
        break;
      endif
    endforeach
    if ( wanted == ( entry ? 1 : 0 ) )
      return struct{ entry := entry };
    endif
    Sleepms( POLL_INTERVAL * 1000 );
  endwhile

  if ( !answered )
    return ret_error( $"client {id} never listed the equipment of 0x{serial:x}" );
  elseif ( wanted )
    return ret_error( $"client {id} never saw layer {layer} of 0x{serial:x} filled" );
  endif
  return ret_error( $"client {id} still has layer {layer} of 0x{serial:x} after {timeout}s" );
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
