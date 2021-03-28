use os;
use attributes;
use vitals;

//see pyuo/brain.py method typestr()
const EVT_INIT := "init";
const EVT_HP_CHANGED := "hp_changed";
const EVT_MANA_CHANGED := "mana_changed";
const EVT_STAM_CHANGED := "stam_changed";
const EVT_SPEECH := "speech";
const EVT_NOTORIETY := "notoriety";
const EVT_MOVED := "moved";
const EVT_NEW_MOBILE := "new_mobile";
const EVT_NEW_ITEM := "new_item";
const EVT_REMOVED_OBJ := "removed_obj";
const EVT_LIST_OBJS := "list_objs";
const EVT_OPEN_BACKPACK := "open_bp";
const EVT_TARGET := "target";

function clientTestActive()
  var testclient:=GetEnvironmentVariable("POLCORE_TESTCLIENT")=="TRUE";
  if (!testclient)
    return 0;
  endif
  return 1;
endfunction

// get client connection auxiliary scriptref and transfers ownership, meaning client events get send to the calling script
function getClientConnection()
  var res:=GetGlobalProperty("#clientcon");
  var proc := GetProcess(res);
  proc.sendevent(struct{ownership:=GetProcess()});
  return proc;
endfunction

// wait for events of given types and of given client id
function waitForClient(id, types)
  while (1)
    var ev:=Wait_For_Event(10);
    if (!ev)
      return ret_error("no signal received");
    endif
    if (ev.id != id || !(ev.type in types))
      continue;
    endif
    return ev;
  endwhile
endfunction

function waitForClients(types)
  // wait for events of given types of any client
  while (1)
    var ev:=Wait_For_Event(10);
    if (!ev)
      return ret_error("no signal received");
    endif
    if (!(ev.type in types))
      continue;
    endif
    return ev;
  endwhile
endfunction
