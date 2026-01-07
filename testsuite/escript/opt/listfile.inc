use file;
use math;

// expects that this is the only function call and performed at the end
// prints the content of the lst file until function call
function print_listfile( test )
  var lines := ReadFile( $"./opt/{test}.lst" );
  if ( !lines )
    return print( lines );
  endif
  // skip non-PC lines
  lines := lines.filter( @( e ) { return e[1] != "#"; } );
  var self := lines.findIndex( @( e ) { return e.find( "jsr userfunc" ); } );
  if ( self )
    lines.shrink( self - 3 ); // remove also makelocal for param pass
  endif
  lines := lines.filter( @( e ) { return !e.find( "(consume)" ); } );
  // remove variable verbose info
  lines := lines.map( @( e ) {
    var i := e.find( "len=" ) ?: e.find( "offset=" );
    return i ? e[1, i - 2] : e;
  } );
  print( lines.reduce( @( acc, curr ) { return acc + curr + "\n"; }, "" ) );
endfunction

