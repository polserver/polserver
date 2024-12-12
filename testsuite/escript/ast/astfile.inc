use file;

function print_astfile( test )
  var lines := ReadFile( $"./ast/{test}.ast" );
  if ( !lines )
    return print( lines );
  endif
  print( "\n".join( lines ) );
endfunction
