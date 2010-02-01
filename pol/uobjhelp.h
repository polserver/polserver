/*
History
=======


Notes
=======

*/

#ifndef UOBJHELP_H
#define UOBJHELP_H


class Client;
class UObject;
class PropertyList;

class UObjectHelper
{
public:
    static void ShowProperties( Client* client, PropertyList& proplist );
    static void ShowProperties( Client* client, UObject* obj );
};

#endif
