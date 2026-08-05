// Shared fixture for the spell tests: the values config/spells.cfg and
// ../../config/circles.cfg were written with, and the two things every file
// needs - a caster carrying a backpack, and a way to see that a spell script
// really ran.
use os;
use uo;

include "testutil";

// open flat land, clear of the other packages' fixtures
const SPELL_X := 70;
const SPELL_Y := 110;

// the spells of config/spells.cfg
const SPELL_CIRCLE := 1;
const SPELL_PARAMS := 2;
const SPELL_REAGENTS := 3;
const SPELL_NOSCRIPT := 4;
const SPELL_SEVEN := 7;
const SPELL_NECRO := 101;
const SPELL_MYSTICISM := 678;

// an id inside the table with nothing behind it
const SPELL_UNDEFINED := 5;
// one past the highest id defined, so one past the end of the table
const SPELL_PAST_END := SPELL_MYSTICISM + 1;

// what the two circles carry
const CIRCLE1_DIFFICULTY := 10;
const CIRCLE1_MANA := 4;
const CIRCLE2_DIFFICULTY := 20;
const CIRCLE2_MANA := 6;

// what spell 2 spells out for itself instead of naming a circle
const PARAMS_DIFFICULTY := 33;
const PARAMS_MANA := 9;

// the itemdesc names of this package
const BOOK_MAGIC := "TestBookMagic";
const BOOK_NECRO := "TestBookNecro";
const BOOK_MYSTICISM := "TestBookMysticism";
const BOOK_BUSHIDO := "TestBookBushido";
const SCROLL_SEVEN := "TestScrollSeven";
const SCROLL_ONE := "TestScrollOne";
const SCROLL_EIGHT := "TestScrollEight";
const SCROLL_NECRO := "TestScrollNecro";
const REAGENT_ASH := "TestReagentAsh";
const REAGENT_MOSS := "TestReagentMoss";
const PLAIN_ITEM := "TestSpellPlainItem";

// a spell script is scheduled, not run inline, so its record has to be waited
// for rather than read
const CAST_TIMEOUT := 5000;
const CAST_POLL := 50;

// An npc with a backpack. Not invulnerable to spells - nothing here damages
// it - but passive, and without an AI script, since a spell script is started
// attached and a character can only carry one of those.
function make_caster( resmgn )
  var npc := resmgn.CreateNPCFromTemplate( ":testspell:spell_npc", SPELL_X, SPELL_Y, 0 );
  if ( !npc )
    return ret_error( $"could not create the npc: {npc}" );
  endif

  var backpack := resmgn.CreateItemAtLocation( SPELL_X, SPELL_Y, 0, 0xE75 );
  if ( !backpack )
    return ret_error( $"could not create the backpack: {backpack}" );
  endif

  var res := EquipItem( npc, backpack );
  if ( !res )
    return ret_error( $"could not equip the backpack: {res}" );
  endif

  return npc;
endfunction

function make_book( resmgn, name )
  var book := resmgn.CreateItemAtLocation( SPELL_X, SPELL_Y, 0, name );
  if ( !book )
    return ret_error( $"could not create {name}: {book}" );
  endif
  return book;
endfunction

function forget_cast()
  EraseGlobalProperty( "#TestSpellCast" );
  return 1;
endfunction

// the record spelleffect.src leaves, or an error once the deadline passes
function wait_for_cast()
  var deadline := ReadMillisecondClock() + CAST_TIMEOUT;
  while ( ReadMillisecondClock() < deadline )
    var cast := GetGlobalProperty( "#TestSpellCast" );
    if ( cast )
      return cast;
    endif
    Sleepms( CAST_POLL );
  endwhile
  return ret_error( $"no spell script ran within {CAST_TIMEOUT}ms" );
endfunction

// that nothing ran, which only a full quiet period can show
function no_cast_for( ms )
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
