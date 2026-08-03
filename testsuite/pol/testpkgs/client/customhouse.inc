// Shared helpers for the custom house design editor tests.
//
// A design session only exists while a client is in it: the core looks the house
// up by the editing character's serial and drops any 0xD7 that does not belong
// to one. So every assertion about design content is made from escript through
// house.house_parts, which reads the working design while editing is on, and the
// client is only there to send commands and to be told what the server thinks
// the design is.
//
// The two views can disagree - that is the point of decoding 0xD8 rather than
// swallowing it - so a case that cares about what the client was actually sent
// asserts on the design event, and one that cares about the model asserts on
// house_parts.
//
// Several cases in these files are written but commented out, because they fail
// against the core as it stands and some of them take the shard or the test
// client down with them. Each is headed by a line beginning
// "TODO core bug <n>", so `grep -rn "TODO core bug [0-9]" testsuite/pol/testpkgs`
// finds all of them and says what has to change before it can be uncommented.
// The numbers are the ones in the write-up the branch was handed over with.

use os;
use uo;

include "testutil";
include "communication";

// the subcommands of the 0xD7 packet, see PKTBI_D7 in the core. Keep in sync
// with the SUB_ constants of pyuo's CustomHouseCommandPacket.
const CH_BACKUP := 0x02;
const CH_RESTORE := 0x03;
const CH_COMMIT := 0x04;
const CH_ERASE := 0x05;
const CH_ADD := 0x06;
const CH_QUIT := 0x0C;
const CH_ADD_MULTI := 0x0D;
const CH_SYNCH := 0x0E;
const CH_CLEAR := 0x10;
const CH_SELECT_FLOOR := 0x12;
const CH_SELECT_ROOF := 0x13;
const CH_DELETE_ROOF := 0x14;
const CH_REVERT := 0x1A;

// what the 0xBF sub 0x20 packet says about the editing session
const CH_EDIT_BEGIN := 0x04;
const CH_EDIT_END := 0x05;

// the z of each design plane, see custom_house_z_xlate_table in the core. A
// part added at any z between two of these belongs to the lower one.
const CH_PLANE_Z := 0;
const CH_PLANE_Z1 := 7;
const CH_PLANE_Z2 := 27;
const CH_PLANE_Z3 := 47;
const CH_PLANE_Z4 := 67;

// the stair multiids the editor accepts, see STAIR_MULTIID_MIN/MAX. Only the
// first of them is in the generated test data, as a ten tile pyramid of 0x751
// climbing north over four tiles.
const CH_STAIR_MIN := 0x1DB0;
const CH_STAIR_MAX := 0x1DF3;
const CH_STAIR_PARTS := 10;
const CH_STAIR := 0x751;
// a multi that is not one of them, and does exist - the range check has to
// answer before anything looks the multi up
const CH_NOT_A_STAIR := 0x6C;

// what an emptied ground floor tile is paved with again, see ReplaceDirtFloor
const CH_DIRT := 0x31F4;

// a roof shingle, the marker for everything that has to be a non-floor tile:
// AddOrReplace only replaces like with like, and Plib::tileheight decides which
// is which. It is part of the house multi at x 1 and nowhere else.
const CH_ROOF := 0x5C2;

// a wall of the house multi, on the west side of the foundation
const CH_FOUNDATION := 0x64;
const CH_FOUNDATION_X := -3;
const CH_FOUNDATION_Y := 0;

// a floor board of the house multi, in the middle of the room and alone on its
// plane there
const CH_BOARD := 0x4AC;

// The most tiles one plane of a design packet can describe. Its plane header
// gives the uncompressed length 12 bits, and a tile is five bytes, so 4095/5
// tiles is all that fits - see the packing in CustomHousesSendFull.
const CH_PLANE_MAX_TILES := 819;

// Where the design tests put their house. Every case builds its own through the
// resource manager and hands it back at the end of the case, so only ever one
// stands here and none of them outlives the case that made it.
//
// That matters for more than tidiness. A house within visual range of another
// test's path shows that test the door and the sign arriving as items out of
// nowhere: while these houses were being left standing for a whole package, one
// of them broke multi_update_range in test_client_range.src, which walks a
// character east from (100,100) and measures where a multi first appears. The
// spot below is still kept well away from everything else the package touches,
// because a case is free to move a character about while its house is up.
const HOUSE_X := 100;
const HOUSE_Y := 155;
// the small house of testpkgs/house/itemdesc.cfg, multi 0x6b. The dirt plots
// there carry a WalkOnScript, which fires as a character is moved into them.
const HOUSE_OBJTYPE := 0x12000;

// What that multi covers: offsets -3..4 in x and -3..4 in y, which the design
// widens by one row in y for the front steps. Everything the design model
// indexes is derived from those bounds, so a test that reaches outside them is
// asking the core to read past an array rather than to refuse.
const HOUSE_MIN_OFF := -3;
const HOUSE_MAX_OFF := 4;
// the row the core forces to z 0 because it hangs off the south side
const HOUSE_STEP_Y := 5;

var clientcon := getClientConnection();
var char;
var house;
var house_x;
var house_y;
var was_expansion;

// Finds the character the design tests drive and puts its account onto AOS, for
// the program of every design test file. The expansion is account state the rest
// of the run sees, so it is set once per file rather than per case.
function house_setup()
  var acct := FindAccount( "testclient0" );
  char := acct.getcharacter( 1 );
  if ( !char )
    return ret_error( "Could not find char at slot 1" );
  endif

  // the shard default is T2A, under which SendHousingTool refuses outright
  was_expansion := set_expansion( "testclient0", "AOS" );
  if ( !was_expansion )
    return was_expansion;
  endif
  return 1;
endfunction

// Puts a fresh custom house up with the client 0 character standing in it, ready
// for SendHousingTool. Every case opens with this.
//
// The house belongs to the resource manager, which the test framework empties
// after each exported function whether it passed or not - so each case gets a
// house nothing has edited, and none of them is left standing for the tests that
// come after. That is what makes the cases independent of the order they run in
// and of each other's failures; the alternative, one house per file, has to undo
// by hand whatever the last case did to it.
function house_start( resmgn )
  house_x := HOUSE_X;
  house_y := HOUSE_Y;

  // A case that failed with its design session still open leaves its house
  // standing: the resource manager cannot take down a multi that is being
  // edited. Two houses on the plot then both hold the character up, and
  // SendHousingTool refuses the new one with "You must be inside the house to
  // customize it." - which says nothing about the case that really failed. Clear
  // the plot first so a failure is reported once instead of once per case.
  foreach leftover in ( ListMultisInBox( HOUSE_X - 8, HOUSE_Y - 8, -100, HOUSE_X + 8, HOUSE_Y + 8,
                                         100 ) )
    if ( leftover.custom && leftover.house_editing )
      leftover.cancelediting( char, 1 );
    endif
    DestroyMulti( leftover );
    drain_events();
  endforeach

  house := resmgn.CreateMultiAtLocation( house_x, house_y, 0, HOUSE_OBJTYPE, CRMULTI_IGNORE_ALL );
  if ( !house )
    return ret_error( $"Could not place the house: {house}" );
  endif
  house.setcustom( 1 );

  // the tool only opens for a character the house itself is holding up
  MoveObjectToLocation( char, house_x, house_y, 0, flags := MOVEOBJECT_FORCELOCATION );
  var ev := waitForClient( 0, { EVT_OWNCREATE } );
  if ( !ev )
    return ret_error( "client 0 was not moved into the house" );
  endif
  return 1;
endfunction

// Puts the account expansion back the way the rest of the run expects it.
function house_teardown()
  if ( was_expansion )
    set_expansion( "testclient0", was_expansion );
  endif
  return 1;
endfunction

// Puts an account onto AOS and hands back what it was on before. The shard
// default is T2A, under which SendHousingTool refuses to do anything, and the
// expansion is account state the rest of the run sees - so whatever this returns
// belongs in a cleanup.
function set_expansion( acctname, expansion )
  var acct := FindAccount( acctname );
  if ( !acct )
    return ret_error( $"no account {acctname}" );
  endif
  var was := acct.uo_expansion;
  var res := acct.Set_UO_Expansion( expansion );
  if ( !res )
    return ret_error( $"could not set {acctname} to {expansion}: {res}" );
  endif
  return was;
endfunction

// Sends one design command as the given client. The arguments each subcommand
// takes are the ones its packet carries, see CustomHouseCommandPacket.fill():
// a graphic and an offset, optionally a z, or a floor number on its own.
function house_cmd( id, sub, arg := 0 )
  var todo := struct{ todo := "house", id := id };
  if ( !arg )
    arg := struct{};
  endif
  arg.+sub := sub;
  todo.+arg := arg;
  clientcon.sendevent( todo );
  return 1;
endfunction

function house_add( id, graphic, xoffset, yoffset )
  return house_cmd( id, CH_ADD, struct{ graphic := graphic, x := xoffset, y := yoffset } );
endfunction

function house_add_multi( id, multiid, xoffset, yoffset )
  return house_cmd( id, CH_ADD_MULTI, struct{ graphic := multiid, x := xoffset, y := yoffset } );
endfunction

function house_erase( id, graphic, xoffset, yoffset, zoffset )
  return house_cmd( id, CH_ERASE, struct{ graphic := graphic, x := xoffset,
                                          y := yoffset, z := zoffset } );
endfunction

function house_roof( id, sub, graphic, xoffset, yoffset, zoffset )
  return house_cmd( id, sub, struct{ graphic := graphic, x := xoffset,
                                     y := yoffset, z := zoffset } );
endfunction

function house_select_floor( id, floor )
  return house_cmd( id, CH_SELECT_FLOOR, struct{ floor := floor } );
endfunction

// Opens the design editor on a house and waits for the client to be put into it.
// Answers the design the server sent to start the session with.
//
// A house that is still open refuses the tool, so a case that failed before it
// could quit would take every later one in the file down with it. Closing the
// session here rather than in a cleanup keeps the cases independent of the order
// they run in.
//
// Quitting a session keeps the working design rather than dropping it, so the
// tiles a case laid down are still there when the next one opens the editor.
// The session therefore starts with a revert, which is what puts the working
// design back to the one the house is really built from - without it every case
// would have to know what every case before it left behind.
function enter_design( id, chr, multi )
  // the member is house_editing, not editing - a plain "editing" reads back as
  // an uninitialized object rather than failing, which looks exactly like a
  // house that is not being edited
  if ( multi.house_editing )
    // a case that failed before it could quit; cancelling ends the session the
    // way a quit does, which puts an end packet and a design on the wire. They
    // have to be off it again before this session's own can be told apart from
    // them - otherwise one failing case turns every later one in the file into
    // "wanted a begin, got an end"
    multi.cancelediting( chr, 1 );
    drain_events();
  endif
  // leaving a design session parks the character on the front step, outside the
  // footprint, and the tool only opens for one the house is holding up - so
  // every entry after the first has to put it back first
  MoveObjectToLocation( chr, house_x, house_y, 0, flags := MOVEOBJECT_FORCELOCATION );

  Clear_Event_Queue();
  var res := SendHousingTool( chr, multi );
  if ( !res )
    return ret_error( $"SendHousingTool refused: {res}" );
  endif

  var ev := waitForClient( id, { EVT_HOUSE_EDIT } );
  if ( !ev )
    return ret_error( $"client {id} was never put into design mode" );
  endif
  if ( CInt( ev.action ) != CH_EDIT_BEGIN )
    return ret_error( $"client {id} got design action {ev.action}, wanted a begin" );
  endif

  // the design the tool sends, which still holds whatever the last session left
  ev := wait_for_design( id );
  if ( !ev )
    return ev;
  endif

  // a revert answers with the committed design, so this is both the reset and
  // the read of what the case starts from
  house_cmd( id, CH_REVERT );
  return wait_for_design( id );
endfunction

// Closes the editor from the client side and waits for the end packet. Answers a
// plain 1 rather than the event: a case usually ends on this call, and the test
// framework wants exactly 1 back from one that passed.
//
// Quitting also hands the client the design the house is really built from, and
// that one is waited for too - every design the server sends has to be taken off
// the queue by the case that caused it, or the next case reads it as the answer
// to something of its own.
function quit_design( id )
  house_cmd( id, CH_QUIT );
  var ev := waitForClient( id, { EVT_HOUSE_EDIT } );
  if ( !ev )
    return ret_error( $"client {id} was never taken out of design mode" );
  endif
  if ( CInt( ev.action ) != CH_EDIT_END )
    return ret_error( $"client {id} got design action {ev.action}, wanted an end" );
  endif
  ev := wait_for_design( id );
  if ( !ev )
    return ev;
  endif
  return 1;
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

// Makes the working design the one the house is really built from, and waits
// for the client to be taken out of the editor. A commit ends the session the
// same way a quit does - the character is put back on the front step and the
// design goes out to everyone in range - so nothing is left to close afterwards.
//
// A house with a commit script attached would answer by asking the character to
// accept rather than by ending the session; the test shard has none, so this is
// the straight-through path.
function commit_design( id )
  house_cmd( id, CH_COMMIT );
  var ev := waitForClient( id, { EVT_HOUSE_EDIT } );
  if ( !ev )
    return ret_error( $"client {id} was never taken out of design mode by a commit" );
  endif
  if ( CInt( ev.action ) != CH_EDIT_END )
    return ret_error( $"client {id} got design action {ev.action}, wanted an end" );
  endif
  return 1;
endfunction

// Waits for the next design the server sends a client.
function wait_for_design( id, timeout := 10 )
  var ev := waitForClient( id, { EVT_HOUSE_DESIGN }, timeout );
  if ( !ev )
    return ret_error( $"client {id} was sent no design within {timeout}s" );
  endif
  return ev;
endfunction

// Asserts that a client is sent no design at all. The whole cost of a case that
// wants this is the wait, so it is kept short - see the wait cost model in
// communication.inc.
function wait_for_no_design( id, timeout := QUIET_SECS )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_HOUSE_DESIGN or CInt( ev.id ) != id )
      continue;
    endif
    return ret_error( $"client {id} was sent a design at revision {ev.revision}" );
  endwhile
  return 1;
endfunction

// Asks for the working design and waits for it, which is how a case knows the
// commands it sent before have been read. Answers that design.
//
// This is the synchronisation every case needs and the revision is not: a synch
// travels the same connection as the commands ahead of it, so by the time its
// answer is in, the design escript reads through house_parts is the one those
// commands left behind.
//
// What it cannot be used for is the revision. CustomHousesSendFull keeps the
// packet it built in WorkingCompressed and replays it byte for byte until an
// edit throws it away, so a design built before a revision was incremented goes
// on being sent under the old number. Erase and clear both send before they
// increment, which is what erase_revision_lags in test_client_house_erase.src
// pins down.
function house_settle( id )
  house_cmd( id, CH_SYNCH );
  return wait_for_design( id );
endfunction

// Asks the server for the working design over and over until the one it sends
// back is at the wanted revision, and hands that design over.
//
// Most subcommands bump the revision without sending anything, so a client is
// usually behind the model it is editing. A synch is the only way to make it
// catch up, and a design already on its way can be older than the one asked for
// - never one-shot this.
function client_sees_revision( id, revision, timeout := 5 )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  var last := -1;

  while ( ReadMillisecondClock() < deadline )
    Clear_Event_Queue();
    house_cmd( id, CH_SYNCH );
    var ev := waitForClient( id, { EVT_HOUSE_DESIGN }, 1 );
    if ( !ev )
      continue;
    endif
    last := CInt( ev.revision );
    if ( last == revision )
      return ev;
    endif
    Sleepms( POLL_INTERVAL * 1000 );
  endwhile

  if ( last < 0 )
    return ret_error( $"client {id} answered no synch within {timeout}s" );
  endif
  return ret_error( $"client {id} is at design revision {last}, wanted {revision}" );
endfunction

// Every part of the design escript sees with that graphic at that offset. The
// counterpart of design_tiles_at(), which reads what the client was sent.
function house_parts_at( multi, graphic, xoffset, yoffset )
  var found := array{};
  foreach part in ( multi.house_parts )
    if ( part.graphic == graphic && part.xoffset == xoffset && part.yoffset == yoffset )
      found.append( part );
    endif
  endforeach
  return found;
endfunction

// Every tile of a design event with that graphic at that offset, whatever its z.
// The design the client is sent carries its offsets the way the multi does, so
// they line up with the xoffset/yoffset of house_parts.
function design_tiles_at( ev, graphic, xoffset, yoffset )
  var found := array{};
  // the client events come out of UnpackJson, so every number in them is a Double
  foreach tile in ( ev.tiles )
    if ( CInt( tile.graphic ) == graphic && CInt( tile.x ) == xoffset && CInt( tile.y ) == yoffset )
      found.append( tile );
    endif
  endforeach
  return found;
endfunction

// Asserts that the design holds exactly one part with that graphic at that
// offset, and that it sits at the wanted z. A part keeps the z it was given
// while the plane it belongs to is picked from it, so the two are worth telling
// apart in a failure message.
function expect_part( multi, graphic, xoffset, yoffset, zoffset )
  var found := house_parts_at( multi, graphic, xoffset, yoffset );
  var count := found.size();
  if ( count != 1 )
    return ret_error( $"0x{graphic:x} at {xoffset},{yoffset} is in the design {count} times" );
  endif
  var z := found[1].z;
  if ( z != zoffset )
    return ret_error( $"0x{graphic:x} at {xoffset},{yoffset} sits at z {z}, wanted {zoffset}" );
  endif
  return 1;
endfunction

// Asserts that the design has no part with that graphic at that offset at all.
function expect_no_part( multi, graphic, xoffset, yoffset )
  var count := house_parts_at( multi, graphic, xoffset, yoffset ).size();
  if ( count )
    return ret_error( $"0x{graphic:x} at {xoffset},{yoffset} is still in the design {count} times" );
  endif
  return 1;
endfunction

// Asserts the design is the size it should be. Almost every command is judged
// by what it did to the part count, and a bare number in a failure says nothing.
function expect_parts( multi, wanted, what )
  var count := multi.house_parts.size();
  if ( count != wanted )
    return ret_error( $"the design has {count} parts after {what}, wanted {wanted}" );
  endif
  return 1;
endfunction

// Whether the design the client was sent holds together: what its header claims
// and what its planes actually carried have to be the same number of tiles.
function design_is_consistent( ev )
  var claimed := CInt( ev.numtiles );
  var got := ev.tiles.size();
  if ( claimed != got )
    return ret_error( $"design says {claimed} tiles, its planes carried {got}" );
  endif
  return 1;
endfunction
