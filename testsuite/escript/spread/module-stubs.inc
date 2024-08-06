function CreateItemAtLocation( unused x, unused y, unused z, unused objtype, unused amount := 1, unused realm := "britannia" ) return "item"; endfunction
function CreateMultiAtLocation( unused x, unused y, unused z, objtype, unused flags := 0, unused realm := "britannia" )
  return TypeOfInt(objtype) == OT_STRING ? "multi" : error{ errortext := "Invalid parameter type" };
endfunction
function CreateNpcFromTemplate( unused template, unused x, unused y, unused z, unused override_properties := 0, unused realm := "britannia", unused forcelocation := 0) return "npc"; endfunction
function TypeOfInt_( what )
  case ( what )
    "item": return OT_ITEMREF;
    "multi": return OT_MULTIREF;
    "npc": return OT_MOBILEREF;
    default: return OT_UNKNOWN;
  endcase
endfunction

function DisposeItem( unused what... ) print( "DisposeItem" ); endfunction
function DisposeMulti( unused what... ) print( "DisposeMulti" ); endfunction
function DisposeNpc( unused what... ) print( "DisposeNpc" ); endfunction
