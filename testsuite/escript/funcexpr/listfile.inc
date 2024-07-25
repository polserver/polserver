use file;

function print_listfile( test )
    var lines := ReadFile( $"./funcexpr/{test}.lst" );
    if ( !lines )
        return print( lines );
    endif

    var prefix := "";
    foreach line in lines
        print( $"{prefix}{line}" );
        // Uses interstrings to make these instructions not indented
        if (line[$"{" "}{"create-functor"}"])
            prefix := $"{prefix}    ";
        elseif (line[$"{" "}{"return"}"])
            prefix := prefix[1, prefix.length() - 4] ?: "";
        endif
    endforeach
endfunction
