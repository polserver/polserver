/*
History
=======


Notes
=======

*/

#ifndef __UVARS_H
#define __UVARS_H

#include "reftypes.h"

#include <vector>

namespace Pol {
  namespace Items {
	class UWeapon;
    class Item;
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
	

	typedef std::vector<AccountRef>           AccountsVector;
	//typedef vector<Character*>         Characters;
    typedef std::vector<Network::Client*>            Clients;
	class ItemsVector : public std::vector<Items::Item*> {};
    typedef std::vector<ServerDescription*> Servers;
    typedef std::vector<StartingLocation*>  StartingLocations;

    extern AccountsVector accounts;
	//extern Characters characters;
	//extern Characters offline_characters;
	extern Clients clients;
	//extern Clients pending_clients; // these are Clients with ready==false
	extern Servers servers;
	extern StartingLocations startlocations;
	extern Items::UWeapon* wrestling_weapon;
  }
}
#endif
