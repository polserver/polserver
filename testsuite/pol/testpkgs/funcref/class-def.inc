class A()
  function A( this )
  endfunction
endclass

class B( A )
  function B( this )
    super();
  endfunction
endclass

class C( A )
  function C( this )
    super();
  endfunction
endclass

class D( B, C )
  function D( this )
    super();
  endfunction
endclass

class E( B, C )
  function E( this )
    super();
  endfunction
endclass
