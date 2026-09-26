// Edits a core config file in place and reloads it, for cases that change a
// setting the core only reads from disk. Needs testutil for ret_error().

use file;
use polsys;

// Rewrites settings of a config file, from a dictionary of key -> value. Lines
// starting with a key are replaced by one appended line. Does not reload. Returns
// the original contents for restore_cfg(), which the case must always call.
function write_cfg_keys( path, keys )
  var original := ReadFile( path );
  if ( original == error )
    return ret_error( $"could not read {path}: {original}" );
  endif

  var rewritten := array{};
  foreach line in ( original )
    var keep := 1;
    foreach key in ( keys.keys() )
      if ( Find( line, key, 1 ) == 1 )
        keep := 0;
        break;
      endif
    endforeach
    if ( keep )
      rewritten.append( line );
    endif
  endforeach
  foreach value in ( keys )
    rewritten.append( $"{_value_iter}={value}" );
  endforeach

  var res := WriteFile( path, rewritten );
  if ( res == error )
    return ret_error( $"could not write {path}: {res}" );
  endif
  return original;
endfunction

// write_cfg_keys() for one key, then ReloadConfiguration.
function set_cfg_key( path, key, value )
  var original := write_cfg_keys( path, dictionary{ key -> value } );
  if ( !original )
    return original;
  endif
  ReloadConfiguration();
  return original;
endfunction

function restore_cfg( path, original )
  WriteFile( path, original );
  ReloadConfiguration();
  return 1;
endfunction
