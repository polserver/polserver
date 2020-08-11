use uo;
use os;

function npc_test()
  syslog("npc_test..");
  var success:=1;

  var npc:=CreateNPCFromTemplate("test",100,100,0);
  print(npc);
  if (!npc)
    success:=0;
    syslog("Could not create NPC: "+npc);
  endif

  while(1)
    sleepms(10);
    if (!npc)
      break;
    elseif (!npc.process)
      syslog(npc.getprop("testerror"));
      success:=0;
      break;
    endif
  endwhile

  return success;
endfunction
