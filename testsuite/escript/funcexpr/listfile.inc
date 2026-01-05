use file;

function print_listfile( test )
  var SKIP := "SKIP FROM HERE";
  var lines := ReadFile( $"./funcexpr/{test}.lst" );
  if ( !lines )
    return print( lines );
  endif
  // skip non-PC lines
  lines := lines.filter( @( e ) { return e[1] != "#"; } );

  var self := lines.findIndex( @( e ) {
    return e.find( "SKIP FROM HERE" );
  } ) - 3;

  var first_find := false;
  var self_end := lines.findIndex( @( e ) {
    var f := e.find( "SKIP END" );
    if ( f )
      if ( !first_find )
        first_find := true;
        return 0;
      else
        return f;
      endif
    endif
    return 0;
  } ) + 4;
  if ( self_end < lines.size() )
    lines := lines[1, self] + lines[self_end, lines.size()];
  else
    lines.shrink( self );
  endif
  var prefix := "";
  foreach line in lines
    print( $"{prefix}{line}" );
    // Uses interstrings to make these instructions not indented
    if ( line[$"{" "}{"create-functor"}"] )
      prefix := $"{prefix}    ";
    elseif ( line[$"{" "}{"return"}"] )
      prefix := prefix[1, prefix.length() - 4] ?: "";
    endif
  endforeach
  SKIP := "SKIP END";
endfunction

