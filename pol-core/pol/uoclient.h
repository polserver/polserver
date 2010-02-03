/*
History
=======
2010/02/03 Turley:    MethodScript support for mobiles


Notes
=======

*/

#ifndef UOCLIENT_H
#define UOCLIENT_H

#include "crypt/cryptkey.h"

class ExportScript;

class UoClientGeneral
{
public:
    void check( std::string& var, const char* tag, const char* deflt );

public:
	~UoClientGeneral();

    class Mapping
    {
    public:
        bool any;
        std::string name;
        unsigned id;
    };

    Mapping strength;
    Mapping intelligence;
    Mapping dexterity;

    Mapping hits;
    Mapping stamina;
    Mapping mana;
	unsigned int maxskills; //dave changed 3/15/03, support configurable max skillid
	ExportScript* method_script;
};

extern UoClientGeneral uoclient_general;

class UoClientProtocol
{
public:
    bool EnableFlowControlPackets;
};
extern UoClientProtocol uoclient_protocol;

class ConfigElem;
class UoClientListener
{
public:
    UoClientListener( ConfigElem& elem );

    unsigned short port;
    TCryptInfo encryption;
	bool aosresist;
};

typedef vector< UoClientListener > UoClientListeners;

extern UoClientListeners uoclient_listeners;

#endif
