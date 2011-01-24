/*
History
=======


Notes
=======

*/

#ifndef CLIFACE_H
#define CLIFACE_H

#include <vector>

class Attribute;
class Client;
class Character;
class Vital;
class UOSkill;

struct ClientVitalUpdaters
{
public:
    ClientVitalUpdaters();

    void (*my_vital_changed)(Client* client, Character* me, const Vital* vital);
    void (*others_vital_changed)(Client* client, Character* him, const Vital* vital);
};

struct ClientAttributeUpdaters
{
public:
    ClientAttributeUpdaters();

    void (*my_attr_changed)(Client* client, Character* me, const Attribute* attr);
    
    const UOSkill* pUOSkill;
};

class ClientInterface
{
public:
    virtual ~ClientInterface() {}

    void register_client( Client* client );
    void deregister_client( Client* client );

    static void tell_vital_changed( Character* who, const Vital* vital );
    static void tell_attribute_changed( Character* who, const Attribute* attr );

protected:
    virtual void Initialize();

    virtual void bcast_vital_changed( Character* who, const Vital* vital ) const = 0;

    std::vector<ClientVitalUpdaters> vital_updaters;
    std::vector<ClientAttributeUpdaters> attribute_updaters;

    std::vector<Client*> clients;

    friend void send_uo_skill( Client* client, Character* me, const Attribute* attr );

};

class UOClientInterface : public ClientInterface
{
public:
    void Initialize();

protected:
    friend class ClientInterface;
    virtual void bcast_vital_changed( Character* who, const Vital* vital ) const;
};
extern UOClientInterface uo_client_interface;

#endif
