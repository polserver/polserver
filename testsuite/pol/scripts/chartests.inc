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
    return;
  endif

  var backpack:=CreateItemAtLocation(0,0,0,0xE75);
  var res:=EquipItem(char,backpack);
  if (!res)
    success:=0;
    syslog("Failed to equip backpack "+backpack+" "+res);
    return;
  endif
  
  backpack.damage_cold_mod:=10;
  if (char.damage_cold != 10)
    syslog("failed to apply equipped aos prop");
    success:=0;
    return;
  endif
  char.damage_cold_mod+=10;
  if (char.damage_cold != 20)
    syslog("failed to apply aos prop mod");
    success:=0;
    return;
  endif


  return success;
endfunction
