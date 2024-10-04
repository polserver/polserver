function Text( b )
  if ( b )
    return "true";
  else
    return "false";
  endif
endfunction

function compareTest( x, y )
  print( "compare: x=" + x + ", y=" + y );

  print( "y <= x? " + Text( y <= x ) );
  print( "y <  x? " + Text( y < x ) );
  print( "y >= x? " + Text( y >= x ) );
  print( "y >  x? " + Text( y > x ) );
  print( "y == x? " + Text( y == x ) );
  print( "y != x? " + Text( y != x ) );

  print( "x <= y? " + Text( x <= y ) );
  print( "x <  y? " + Text( x < y ) );
  print( "x >= y? " + Text( x >= y ) );
  print( "x >  y? " + Text( x > y ) );
  print( "x == y? " + Text( x == y ) );
  print( "x != y? " + Text( x != y ) );

endfunction
