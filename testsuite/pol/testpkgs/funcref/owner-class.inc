class Owner()
  function Owner( this )
  endfunction

  // Reads a global of the program this class is compiled in, so the value proves which script's
  // globals the method ran against.
  function who( this )
    return owner;
  endfunction
endclass
