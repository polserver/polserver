// Shared helpers for the client-driven half of the spell tests.
//
// Everything here drives do_cast and handle_open_spellbook, the two entry
// points nothing but a client can reach. The spells themselves, the circles and
// the books come from testpkgs/spell - they are shard data, so this package can
// cast them without defining any of its own.
//
// Two things shape every case:
//
// - A cast is not answered. do_cast either schedules the spell or sends a
//   refusal, and a refusal is the only thing that arrives as a packet. So a
//   case that expects the cast to go through waits for the spell script's
//   record, and a case that expects it to be refused waits for the text. There
//   is nothing to synchronise on in between, which is why the negative
//   assertions all cost a quiet period.
// - The spell script is started attached to the caster, so the caster carries
//   an executor for as long as it runs. That is what spell_hold() below is for
//   and also why nothing may cast while a hold is on.

use os;
use uo;

include "testutil";
include "communication";

// The spells of testpkgs/spell/config/spells.cfg. Only the ones this file's
// cases name are here; the rest are in that package's own spell.inc.
const SPELL_CIRCLE := 1;
const SPELL_SEVEN := 7;
const SPELL_NECRO := 101;
const SPELL_MYSTICISM := 678;
// an id inside the table with no spell behind it, which is what "That spell
// does not function." is for
const SPELL_UNDEFINED := 5;
// a spell that names no Script, which is what USpell::cast has its own refusal
// for. do_cast never sees it - it schedules the spell either way
const SPELL_NOSCRIPT := 4;
// one past the highest id defined, so past the end of the table. do_cast drops
// it without a word rather than refusing it
const SPELL_PAST_END := SPELL_MYSTICISM + 1;

// The objtypes the magery scrolls of testpkgs/spell sit at. The core hardcodes
// the range (gamestate.spell_scroll_objtype_limits) and swaps the first seven,
// so the lowest objtype carries spell 7 and the next one spell 1.
const SCROLL_OBJTYPE_SEVEN := 0x1F2D;
const SCROLL_OBJTYPE_ONE := 0x1F2E;

// the itemdesc names of testpkgs/spell that these cases use
const BOOK_MAGIC := "TestBookMagic";
const BOOK_NECRO := "TestBookNecro";
const BOOK_MYSTICISM := "TestBookMysticism";
const BOOK_BUSHIDO := "TestBookBushido";

// The layer the core looks in for a spellbook a character is holding, and the
// only one 0xEFA can go on. Both routes into a book - casting and opening -
// check that hand before they look in the pack.
const LAYER_BOOK_HAND := 1;

// what the core refuses each way in, see do_cast and handle_open_spellbook
const MSG_UNKNOWN_SPELL := "You don't know that spell.";
const MSG_NO_SUCH_SPELL := "That spell does not function.";
const MSG_FROZEN := "I am frozen and cannot cast spells";
const MSG_PARALYZED := "I am paralyzed and cannot cast spells";
const MSG_BUSY := "You are already doing something else.";
const MSG_NOCAST := "Spells cannot be cast here.";
const MSG_DEAD := "I am dead and cannot do that.";
const MSG_NEEDS_SE := "This item requires at least the Samurai Empire Expansion.";
const MSG_NEEDS_AOS := "This item requires at least the Age of Shadows Expansion.";
const MSG_LOCKED := "That is locked.";
// USpell::cast's own refusal, which only the route that skips do_cast reaches
const MSG_NO_EFFECT := "That spell doesn't seem to work.";

// A cast is scheduled rather than run inline, so the record the spell script
// leaves has to be waited for.
const CAST_TIMEOUT := 5000;
const CAST_POLL := 50;

// Where the casting tests stand. Clear of the paths the movement and range
// cases walk and of the plot the design cases build on, because a spell script
// running here must not be reached by anything else.
const CAST_X := 60;
const CAST_Y := 60;

// Inside the one no-cast region of the shard, ../../regions/nocast.cfg. Regions
// are painted a zone at a time, so that file's range really does cover 160..167
// in both axes and this is comfortably inside it.
const NOCAST_X := 163;
const NOCAST_Y := 163;

var clientcon := getClientConnection();
var char;
var was_expansion;

// Finds the character the casting tests drive, stands it on clear ground and
// puts its account onto AOS - the expansion decides which of the two spellbook
// packets the core sends and which schools it will open at all. For the program
// of every casting test file.
function spell_setup()
  var acct := FindAccount( "testclient0" );
  char := acct.getcharacter( 1 );
  if ( !char )
    return ret_error( "Could not find char at slot 1" );
  endif

  was_expansion := set_expansion( "testclient0", "AOS" );
  if ( !was_expansion )
    return was_expansion;
  endif

  return stand_at( CAST_X, CAST_Y );
endfunction

// Puts the character somewhere and waits for the client to be told about it,
// which is what makes a case that depends on where it stands deterministic.
//
// A move to where the character already is sends the client nothing, so there
// would be nothing to wait for - and since spell_start() calls this on every
// case, that wait would be its whole timeout every time.
function stand_at( x, y )
  if ( char.x == x and char.y == y )
    return 1;
  endif
  MoveObjectToLocation( char, x, y, 0, flags := MOVEOBJECT_FORCELOCATION );
  return wait_for_own_create( 0 );
endfunction

// Puts the account expansion back the way the rest of the run expects it.
function spell_teardown()
  if ( was_expansion )
    set_expansion( "testclient0", was_expansion );
  endif
  return 1;
endfunction

// Puts an account onto an expansion and hands back the one it was on. The shard
// default is T2A, and the expansion is account state the rest of the run sees,
// so whatever this returns belongs in a teardown.
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

// Clears whatever the case before left behind. Every case opens with this: the
// record, the two arming properties and everything it puts on the character are
// shard-wide, so a case that failed halfway would otherwise be read as this
// one's answer - and where the character stands decides whether it may cast at
// all, so that is reset here too rather than left to each case to undo.
function spell_start()
  EraseGlobalProperty( "#TestSpellCast" );
  EraseGlobalProperty( "#TestSpellHold" );
  EraseGlobalProperty( "#CastHookCall" );
  EraseGlobalProperty( "#CastHookCancel" );
  EraseGlobalProperty( "#OpenBookCall" );
  EraseGlobalProperty( "#OpenBookCancel" );
  char.frozen := 0;
  char.paralyzed := 0;
  if ( char.dead )
    Resurrect( char );
  endif
  Clear_Event_Queue();
  return stand_at( CAST_X, CAST_Y );
endfunction

// Makes a spellbook of the given kind holding the given spells and puts it in
// the character's backpack. The book belongs to the resource manager, so it is
// gone again the moment the case ends - which matters more here than usual,
// since knows_spell walks the whole backpack and would find a book an earlier
// case left behind.
function make_book( resmgn, name, spells := 0 )
  var book := resmgn.CreateItemInContainer( char.backpack, name );
  if ( !book )
    return ret_error( $"could not create {name}: {book}" );
  endif
  if ( spells )
    foreach spellid in ( spells )
      var res := book.addspell( spellid );
      if ( !res )
        return ret_error( $"could not add spell {spellid} to {name}: {res}" );
      endif
    endforeach
  endif
  return book;
endfunction

// Same, held in the casting hand instead of carried. Both routes into a book
// look there first, so a book on the layer proves a different branch than one
// in the pack.
function equip_book( resmgn, name, spells := 0 )
  var book := make_book( resmgn, name, spells );
  if ( !book )
    return book;
  endif
  var res := EquipItem( char, book );
  if ( !res )
    return ret_error( $"could not equip {name}: {res}" );
  endif
  return book;
endfunction

// Asks a client to cast a spell. Without a book serial the request goes as the
// short text command that carries the id alone; with one it goes as the form
// that names a book, which the core reads the id out of and then ignores the
// rest of. select switches to the spellbook gump's own 0xBF sub 0x1C instead.
function cast_spell( id, spellid, bookserial := 0, select := 0 )
  var arg := struct{};
  arg.+spellid := spellid;
  if ( bookserial )
    arg.+bookserial := bookserial;
  endif
  if ( select )
    arg.+select := 1;
  endif
  clientcon.sendevent( struct{ todo := "cast", id := id, arg := arg } );
  return 1;
endfunction

// Asks a client to open the spellbook its character is carrying.
function open_spellbook( id )
  clientcon.sendevent( struct{ todo := "spellbook", id := id } );
  return 1;
endfunction

// Makes a client say something. The tilde commands are the third way into
// do_cast and they arrive as ordinary speech, so this is all they need.
function say_text( id, text )
  clientcon.sendevent( struct{ todo := "speech", arg := text, id := id } );
  return 1;
endfunction

// The serial the pre-AOS listing gives the entry of a spell. Nothing is behind
// it - the entries stand for bits in the book, not for items - so the core
// counts down from the top of the item range to keep them clear of real ones.
function old_book_serial( spellnumber )
  return 0x7FFFFFFF - spellnumber;
endfunction

// The record spelleffect.src leaves, or an error once the deadline passes.
function wait_for_cast( timeout := CAST_TIMEOUT )
  var deadline := ReadMillisecondClock() + timeout;
  while ( ReadMillisecondClock() < deadline )
    var cast := GetGlobalProperty( "#TestSpellCast" );
    if ( cast )
      return cast;
    endif
    Sleepms( CAST_POLL );
  endwhile
  return ret_error( $"no spell script ran within {timeout}ms" );
endfunction

// That no spell ran, which only a quiet period can show. This is the whole cost
// of a case that refuses a cast, so it is kept to the shared window.
function no_cast_for( ms := QUIET_SECS * 1000 )
  var deadline := ReadMillisecondClock() + ms;
  while ( ReadMillisecondClock() < deadline )
    var cast := GetGlobalProperty( "#TestSpellCast" );
    if ( cast )
      return ret_error( $"spell {cast.spellid} ran when nothing should have" );
    endif
    Sleepms( CAST_POLL );
  endwhile
  return 1;
endfunction

// Casts a spell that then sits in its script holding the caster's script slot,
// and answers once it is known to be running. Only for reaching the branch that
// refuses a second cast - let go of it with spell_release() in the same case.
function spell_hold( id, spellid )
  SetGlobalProperty( "#TestSpellHold", 1 );
  cast_spell( id, spellid );
  var cast := wait_for_cast();
  if ( !cast )
    SetGlobalProperty( "#TestSpellHold", 0 );
    return cast;
  endif
  return cast;
endfunction

// Lets the held script finish and waits until the caster is free again, so the
// next case does not open on a character that is still busy.
function spell_release()
  EraseGlobalProperty( "#TestSpellHold" );
  var deadline := ReadMillisecondClock() + CAST_TIMEOUT;
  while ( ReadMillisecondClock() < deadline )
    // the same thing do_cast reads to refuse a second cast, so this waits for
    // exactly the state the hold was put there to produce
    if ( !char.active_skill )
      return 1;
    endif
    Sleepms( CAST_POLL );
  endwhile
  return ret_error( "the held spell script never finished" );
endfunction

// Waits for the spellbook contents packet, 0xBF sub 0x1B, which is what an AOS
// client is sent when a book of a school it may open is double clicked.
function wait_for_book( id, timeout := 10 )
  var ev := waitForClient( id, { EVT_SPELLBOOK }, timeout );
  if ( !ev )
    return ret_error( $"client {id} was sent no spellbook within {timeout}s" );
  endif
  return ev;
endfunction

// That no such packet arrives, for the cases where the core answers with a
// refusal or with nothing at all.
function no_book_for( id, timeout := QUIET_SECS )
  var deadline := ReadMillisecondClock() + timeout * 1000;
  while ( ReadMillisecondClock() < deadline )
    var ev := Wait_For_Event( POLL_INTERVAL );
    if ( !ev or ev.type != EVT_SPELLBOOK or CInt( ev.id ) != id )
      continue;
    endif
    return ret_error( $"client {id} was sent the contents of book 0x{ev.serial:x}" );
  endwhile
  return 1;
endfunction

// Whether a spell's bit is set in the eight content bytes the packet carries.
// The bits run from the school's first spell in the low bit of byte 0, so the
// place a spell has in its own school is what indexes them - 1 for magery spell
// 1 and for necromancy spell 101 alike.
function book_has_bit( contents, spellnumber )
  var idx := CInt( ( spellnumber - 1 ) / 8 );
  var bit := 1 << ( ( spellnumber - 1 ) % 8 );
  return CInt( contents[idx + 1] ) & bit ? 1 : 0;
endfunction
