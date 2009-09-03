/*
History
=======


Notes
=======

*/

#ifndef TIPLIST_H
#define TIPLIST_H

#ifdef _MSC_VER
#pragma warning( disable: 4786 )
#endif

#include <map>
#include <set>
#include <vector>



typedef vector<string> TipFilenames;
extern TipFilenames tipfilenames;

void load_tips();

bool send_tip( class Client* client, const char* tipname );


#endif
