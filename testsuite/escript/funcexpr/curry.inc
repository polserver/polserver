function curry( f ) // curry(f) does the currying transform
  return @( a ) {
    return @( b ) {
      return f( a, b );
    };
  };
endfunction

// usage
function sum( a , b )
  return a + b;
endfunction

function mul( a, b )
  return a * b;
endfunction

function sub( a, b )
  return a - b;
endfunction
