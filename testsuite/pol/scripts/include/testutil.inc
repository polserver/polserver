use uo;

const IGNORE_TEST:=255; // only valid for setup or programs

function ret_error(str)
  return error{errortext:=str};
endfunction

function ret_error_not_equal( actual, expected, message )
  if ( actual != expected )
	  return ret_error( $"{message}" );
  endif

  return 1;
endfunction

function createAccountWithChar(accname, psw)
  var a:=FindAccount(accname);
  if (!a)
    a:=CreateAccount(accname,psw,1);
    if (!a)
      return ret_error("Failed to create account");
    endif
    a.addcharacter(0);
  endif
  var char:=a.getcharacter(1);
  if (!char)
    return ret_error("Could not find char at slot 1");
  endif
  return char;
endfunction
