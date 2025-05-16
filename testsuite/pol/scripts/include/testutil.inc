use file;
use uo;
use os;
use polsys;

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
    a.set_uo_expansion("AOS");
    a.addcharacter( 0 );
  endif
  var char := a.getcharacter( 1 );
  if ( !char )
    return ret_error( "Could not find char at slot 1" );
  endif
  return char;
endfunction

function dprint( unused what )
// print(what);
endfunction

class ResourceManager()
  function ResourceManager( this )
    this.resources := array{};
  endfunction

  function Cleanup( this )
    dprint( $"CLEANUP! {this.resources}" );
    while ( this.resources.size() )
      var resource_entry := this.resources[this.resources.size()];
      var resource := resource_entry[1];
      var cleanup_fn := resource_entry[2];

      dprint( $"cleanup 0x{resource.serial:x} ({TypeOf( resource )}):" );
      var res := cleanup_fn.call( resource );
      dprint( $" - {res}" );
      this.resources.shrink( this.resources.size() - 1 );
    endwhile
    return 1;
  endfunction

  function CreateMultiAtLocation( this, x, y, z, objtype, flags := 0, realm := _DEFAULT_REALM )
    // Module functions are registered in global scope as well
    var resource :=::CreateMultiAtLocation( x, y, z, objtype, flags, realm );
    if ( !resource )
      return resource;
    endif

    this.resources.append( array{ resource, @DestroyMultiResource } );
    dprint( $"created resource resource={resource} this.resources={this.resources}" );
    return resource;
  endfunction

  function DestroyMultiResource( multi )
    dprint( $"cleanup multi 0x{multi.serial:x}" );
    return DestroyMulti( multi );
  endfunction

  function CreateItemAtLocation( this, x, y, z, objtype, amount := 1, realm := _DEFAULT_REALM )
    var resource := uo::CreateItemAtLocation( x, y, z, objtype, amount, realm );
    if ( !resource )
      return resource;
    endif
    this.resources.append( array{ resource, @DestroyItemResource } );
    return resource;
  endfunction

  function Start_Script( this, script_name, byref param := 0 )
    var resource := os::Start_Script( script_name, param );
    if ( !resource )
      return resource;
    endif
    this.resources.append( array{ resource, @DestroyScriptResource } );
    return resource;
  endfunction

  function DestroyScriptResource( script )
    dprint( $"cleanup script {script} {script.state} {script.pid}" );
    // Swallow the "Script has been destroyed" error
    return script.kill() ?: 1;
  endfunction

  function DestroyItemResource( item )
    dprint( $"cleanup item 0x{item.serial:x}" );
    return DestroyItem( item );
  endfunction

  function CreateNPCFromTemplate( this, template, x, y, z, override_properties := 0,
                                  realm := _DEFAULT_REALM, forcelocation := 0 )

    var resource := uo::CreateNPCFromTemplate( template, x, y, z, override_properties, realm,
                                               forcelocation );
    if ( !resource )
      return resource;
    endif
    this.resources.append( array{ resource, @DestroyNpcResource } );
    return resource;
  endfunction

  function DestroyNpcResource( npc )
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

  function AddRealm( this, realm_name, base_realm )
    var res := polsys::AddRealm( realm_name, base_realm );
    if ( !res )
      return res;
    endif

    this.resources.append( array{ realm_name, @DestroyRealmResource } );

    return res;
  endfunction

  function DestroyRealmResource( realm_name )
    return DeleteRealm( realm_name );
  endfunction
endclass

function copy_file( source, destination )
  var file_in := OpenBinaryFile( source, OPENMODE_IN );
  if ( !file_in )
    return file_in;
  endif

  var file_out := OpenBinaryFile( destination, OPENMODE_OUT | OPENMODE_TRUNC );
  if ( !file_out )
    return file_out;
  endif

  var byte, i := 0;

  do
    byte := file_in.getInt8();
    if ( byte == error )
      break;
    endif
    file_out.setInt8( byte );
  dowhile ( 1 );

  file_in.close();
  file_out.close();

  return 1;
endfunction