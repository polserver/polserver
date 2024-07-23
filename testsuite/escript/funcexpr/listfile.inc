use file;

function print_listfile( test )
    print( "\n".join( ReadFile( $"./funcexpr/{test}.lst" ) ) );
endfunction
