/*
History
=======

2005/03/01 Shinigami: added MAX_NUMER_REALMS for use in MSGBF_SUB18_ENABLE_MAP_DIFFS
2009/08/25 Shinigami: STLport-5.2.1 fix: shadowrealm_count definition fixed
2009/12/02 Turley:    added TerMur - Tomi

Notes
=======

*/


#ifndef REALMS_H
#define REALMS_H
namespace Pol {
  namespace Plib {
	class Realm;
  }
  namespace Core {

	extern Plib::Realm* main_realm;
	extern vector<Plib::Realm*>* Realms;
	extern std::map<int, Plib::Realm*> shadowrealms_by_id;
	extern unsigned int baserealm_count;
	extern unsigned int shadowrealm_count;

	Plib::Realm* find_realm( const std::string& name );
	void add_realm( const std::string& name, Plib::Realm* base );
	bool defined_realm( const std::string& name );
	void remove_realm( const std::string& name );
#define ENABLE_OLD_MAPCODE 0

	// Support up to 5 Maps: Britannia, Britannia_alt, Ilshenar, Malas, Tokuno, TerMur
#define MAX_NUMER_REALMS 6
  }
}
#endif
