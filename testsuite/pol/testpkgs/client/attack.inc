// Attack requests from a test client. Needs testutil and communication.

// Sends an attack request for the given serial and waits for the server to
// highlight the expected object.
function send_attack( clientcon, serial, expected )
  Clear_Event_Queue();
  clientcon.sendevent( struct{ todo := "attack", arg := serial, id := 0 } );
  while ( 1 )
    var ev := waitForClient( 0, { EVT_ATTACK } );
    if ( !ev )
      return ret_error( $"No highlight of 0x{expected:x} received: {ev}" );
    endif
    if ( ev.serial == expected )
      return ev;
    endif
  endwhile
endfunction
