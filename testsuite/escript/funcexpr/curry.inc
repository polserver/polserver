function curry( f ) // curry(f) does the currying transform
  return function ( a ) {
    return function ( b ) {
      // Change this to `f(a, b)` when expressions-as-callee feature lands.
      return f.call( a, b );
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
