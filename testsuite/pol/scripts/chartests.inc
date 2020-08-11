use uo;
use os;

function account_test()
  syslog("account_test..");
  var success:=1;

  var a:=CreateAccount("te st","pass",1);
  if (a)
    success:=0;
    syslog("Could create account with whitespace: "+a);
  endif

  a:=CreateAccount("test","pass",1);
  if (!a)
    success:=0;
    syslog("Could not create account: "+a);
  endif
  return success;
endfunction

function char_test()
  syslog("char_test..");
  var success:=1;
  var a:=FindAccount("test");
  if (!a)
    a:=CreateAccount("test","pass",1);
    syslog("need to create test account");
  endif
  a.addcharacter(0);
  var char:=a.getcharacter(1);
  if (!char)
    success:=0;
    syslog("Could not find char at slot 1");
  endif
  return success;
endfunction
