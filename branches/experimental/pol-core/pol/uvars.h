/*
History
=======


Notes
=======

*/

#ifndef __UVARS_H
#define __UVARS_H

#include "reftypes.h"

class Account;
class Character;
class Client;
class Item;
class ServerDescription;
class StartingLocation;
class UObject;
class UMulti;
class UWeapon;
class WeaponTemplate;

typedef vector<AccountRef>           Accounts;
//typedef vector<Character*>         Characters;
typedef vector<Client*>            Clients;
class Items : public vector<Item*> {};
typedef vector<ServerDescription*> Servers;
typedef vector<StartingLocation*>  StartingLocations;

extern Accounts accounts;
//extern Characters characters;
//extern Characters offline_characters;
extern Clients clients;
extern Clients pending_clients; // these are Clients with ready==false
extern Servers servers;
extern StartingLocations startlocations;

extern WeaponTemplate* wrestling_template;
extern UWeapon* wrestling_weapon;

#endif
