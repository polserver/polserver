// packtst.inc

// test_pack( obj, string_representation )

function test_pack( obj, exrep )
  print( "Testing Pack for: " + obj );
  print( "Expected representation: " + exrep );

  var acrep := pack( obj );
  print( "Result representation:   " + acrep );
  if ( exrep == acrep )
    print( "Successful!" );
  else
    print( "Failure!" );
    return 0;
  endif

  var unp := unpack( acrep );
  print( "Unpacked: " + unp );
  if ( unp == obj )
    print( "Unpack matches!" );
  else
    print( "Unpack doesn't match!" );
    if ( pack( unp ) == acrep )
      print( "repack does match, however." );
    else
      print( "repack is " + pack( unp ) );
    endif
    return 0;
  endif

  return 1;
endfunction
