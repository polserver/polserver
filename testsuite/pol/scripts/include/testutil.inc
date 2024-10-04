use uo;

const IGNORE_TEST := 255; // only valid for setup or programs

function ret_error( str )
  return error{ errortext := str };
endfunction

function ret_error_not_equal( actual, expected, message )
  if ( actual != expected )
    return ret_error( $"{message}" );
  endif

  return 1;
endfunction

function createAccountWithChar( accname, psw )
  var a := FindAccount( accname );
  if ( !a )
    a := CreateAccount( accname, psw, 1 );
    if ( !a )
      return ret_error( "Failed to create account" );
    endif
    a.addcharacter( 0 );
  endif
  var char := a.getcharacter( 1 );
  if ( !char )
    return ret_error( "Could not find char at slot 1" );
  endif
  return char;
endfunction

var __resources := array{};

function dprint( unused what )
endfunction

function destroy_multi_resource( multi )
  dprint( $"cleanup multi 0x{multi.serial:x}" );
  return DestroyMulti( multi );
endfunction

function destroy_item_resource( item )
  dprint( $"cleanup item 0x{item.serial:x}" );
  return DestroyItem( item );
endfunction

function destroy_npc_resource( npc )
  dprint( $"cleanup npc {npc.name} 0x{npc.serial:x}" );
  var x := npc.x, y := npc.y, z := npc.z, serial := npc.serial;
  var res := npc.kill();
  if ( !res )
    return res;
  endif

  var corpses := ListItemsNearLocationOfType( x, y, z, 0, 0x2006 );
  foreach corpse in corpses
    if ( serial == corpse.ownerserial )
      return DestroyItem( corpse );
    endif
  endforeach

  return ret_error( $"Could not find corpse for NPC 0x{serial:x} at ({x}, {y}, {z}) (corpses: {corpses.size()})" );
endfunction

function new_item_resource( x, y, z, objtype, amount := 1, realm := _DEFAULT_REALM )
  var resource := CreateItemAtLocation( x, y, z, objtype, amount, realm );
  if ( !resource )
    return resource;
  endif
  __resources.append( array{ resource, @destroy_item_resource } );
  return resource;
endfunction

function new_multi_resource( x, y, z, objtype, flags := 0, realm := _DEFAULT_REALM )
  var resource := CreateMultiAtLocation( x, y, z, objtype, flags, realm );
  if ( !resource )
    return resource;
  endif
  __resources.append( array{ resource, @destroy_multi_resource } );
  return resource;
endfunction

function new_npc_resource( template, x, y, z, override_properties := 0, realm := _DEFAULT_REALM,
                           forcelocation := 0 )
  var resource := CreateNPCFromTemplate( template, x, y, z, override_properties, realm,
                                         forcelocation );
  if ( !resource )
    return resource;
  endif
  __resources.append( array{ resource, @destroy_npc_resource } );
  return resource;
endfunction

function add_resource( resource, func )
  if ( !resource )
    return resource;
  endif
  return func( resource );
endfunction

function cleanup_resources()
  while ( __resources.size() )
    var resource_entry := __resources[__resources.size()];
    var resource := resource_entry[1];
    var cleanup_fn := resource_entry[2];

    dprint( $"cleanup 0x{resource.serial:x} ({TypeOf( resource )}):" );
    var res := cleanup_fn.call( resource );
    dprint( $" - {res}" );
    __resources.shrink( __resources.size() - 1 );
  endwhile
endfunction

function with_resources( resourced_func )
  var res := resourced_func();

  cleanup_resources();

  return res;
endfunction

function invoke_if( result, fn )
  if ( result )
    return fn();
  else
    return result;
  endif
endfunction
