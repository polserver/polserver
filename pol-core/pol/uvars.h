/*
History
=======


Notes
=======

*/

#ifndef __UVARS_H
#define __UVARS_H

#include "reftypes.h"
#include "item/item.h"

namespace Pol {
  namespace Items {
	class UWeapon;
  }
  namespace Accounts {
	class Account;
  }
  namespace Mobile {
	class Character;
  }
  namespace Network {
	class Client;
  }
  namespace Multi {
	class UMulti;
  }
  namespace Core {
	class ServerDescription;
	class StartingLocation;
	class UObject;
	

	typedef vector<AccountRef>           AccountsVector;
	//typedef vector<Character*>         Characters;
	typedef vector<Network::Client*>            Clients;
	class ItemsVector : public vector<Items::Item*> {};
	typedef vector<ServerDescription*> Servers;
	typedef vector<StartingLocation*>  StartingLocations;

    extern AccountsVector accounts;
	//extern Characters characters;
	//extern Characters offline_characters;
	extern Clients clients;
	extern Clients pending_clients; // these are Clients with ready==false
	extern Servers servers;
	extern StartingLocations startlocations;
	extern Items::UWeapon* wrestling_weapon;
  }
}
#endif
