#include <stdio.h>

int main( int argc, char *argv[] )
{
    int dif = 0;

    if (argc != 3)
    {
        printf( "Usage: bcomp file1 file2\n" );
        return 1;
    }

    FILE *fa = fopen( argv[1], "rb" );
    if (!fa)
    {
        printf( "File '%s' does not exist.\n", argv[1] );
        dif = 1;
    }

    FILE *fb = fopen( argv[2], "rb" );
    if (!fb)
    {
        printf( "File '%s' does not exist.\n", argv[2] );
        dif = 1;
    }

    size_t address=0;
    while (!dif)
    {
        char cha, chb;
        size_t resa, resb;
        resa = fread( &cha, 1, 1, fa );
        resb = fread( &chb, 1, 1, fb );
        if (resa != resb)
        {
            printf( "Files %s and %s have different sizes.\n", argv[1], argv[2] );
            dif = 1;
            break;
        }
        if (cha != chb)
        {
            printf( "Files %s and %s are different. Address %X\n", argv[1], argv[2],address );
            dif = 1;
            break;
        }
        if (resa == 0)
            break;
      ++address;
    }

    if (fb) fclose( fb );
    fb = NULL;
    if (fa) fclose( fa );
    fa = NULL;

    return dif;
}
