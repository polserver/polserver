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

function waitForClient(id, types)
  while (1)
    var ev:=Wait_For_Event(5);
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
  while (1)
    var ev:=Wait_For_Event(5);
    if (!ev)
      return ret_error("no signal received");
    endif
    if (!(ev.type in types))
      continue;
    endif
    return ev;
  endwhile
endfunction
