// History
//   2006/10/07 Shinigami: FreeBSD fix - changed __linux__ to __unix__

// this file only used in Linux build
#ifdef LINUX

#include "kbhit.h"
#include <unistd.h>  // read()

namespace Pol::Clib
{
KeyboardHook::KeyboardHook()
{
  tcgetattr( 0, &initial_settings );
  new_settings = initial_settings;
  new_settings.c_lflag &= ~ICANON;
  new_settings.c_lflag &= ~ECHO;
  new_settings.c_lflag |= ISIG;  // enable signals, like ctrl-c
  new_settings.c_cc[VMIN] = 1;
  new_settings.c_cc[VTIME] = 0;
  tcsetattr( 0, TCSANOW, &new_settings );
  peek_character = -1;
}

KeyboardHook::~KeyboardHook()
{
  tcsetattr( 0, TCSANOW, &initial_settings );
}

int KeyboardHook::kbhit()
{
  if ( peek_character != -1 )
    return 1;

  new_settings.c_cc[VMIN] = 0;
  tcsetattr( 0, TCSANOW, &new_settings );
  unsigned char ch;
  int nread = read( 0, &ch, 1 );
  new_settings.c_cc[VMIN] = 1;
  tcsetattr( 0, TCSANOW, &new_settings );

  if ( nread == 1 )
  {
    peek_character = ch;
    return 1;
  }
  return 0;
}

int KeyboardHook::getch()
{
  char ch;

  if ( peek_character != -1 )
  {
    ch = peek_character;
    peek_character = -1;
  }
  else
  {
    int numread = read( 0, &ch, 1 );
    if ( numread != 1 )
      throw;
  }

  return ch;
}
}  // namespace Pol::Clib


#endif
