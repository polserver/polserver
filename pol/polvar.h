/*
History
=======


Notes
=======

*/

#ifndef POLVAR_H
#define POLVAR_H

class PolVar
{
public:
    unsigned short DataWrittenBy;
};

extern PolVar polvar;
extern unsigned long stored_last_item_serial;
extern unsigned long stored_last_char_serial;
#endif
