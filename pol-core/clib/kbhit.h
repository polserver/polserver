#ifndef _WIN32

#ifndef KBHIT_H
#define KBHIT_H

#include <termios.h>

namespace Pol {
  namespace Clib {
    class KeyboardHook
    {
    public:
      KeyboardHook();
      ~KeyboardHook();
      int kbhit();
      int getch();

    private:
      struct termios initial_settings, new_settings;
      int peek_character;
    };
  }
}

#endif 

#endif
