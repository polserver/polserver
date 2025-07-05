use file;

// expects that this is the only function call and performed at the end
// prints the content of the lst file until function call
function print_listfile( test )
  var lines := ReadFile( $"./opt/{test}.lst" );
  if ( !lines )
    return print( lines );
  endif
  var self := lines.findIndex( @( e ) { return e.find( "jsr userfunc" ); } );
  if ( self )
    lines.shrink( self - 3 ); // remove also makelocal for param pass
  endif
  print( lines.reduce( @( acc, curr ) { return acc + curr + "\n"; }, "" ) );
endfunction
