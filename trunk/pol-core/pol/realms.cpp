/*
History
=======

2005/03/01 Shinigami: added MAX_NUMER_REALMS check to prevent core crash (see MSGBF_SUB18_ENABLE_MAP_DIFFS)
2008/12/17 MuadDub:   Added check when loading Realms for no realms existing via int counter.
2009/08/25 Shinigami: STLport-5.2.1 fix: shadowrealm_count definition fixed

Notes
=======

*/

#include "clib/stl_inc.h"
#include "clib/dirlist.h"
#include "clib/passert.h"
#include "clib/strutil.h"
#include "plib/realm.h"
#include "uofile.h"
#include "los.h"
#include "polcfg.h"
#include "realms.h"

Realm* main_realm = NULL;
vector<Realm*>* Realms = new vector<Realm*>();
std::map<int,Realm*> shadowrealms_by_id;
unsigned int baserealm_count=0;
unsigned int shadowrealm_count=0;

bool load_realms()
{
	//string realm_dir = "realm/";
	Realm* temprealm;
	int realm_counter = 0;
	for( DirList dl( config.realm_data_path.c_str() ); !dl.at_end(); dl.next() )
	{
		string realm_name = dl.name();
		if (realm_name[0] == '.') 
			continue;

		passert_r( Realms->size() < MAX_NUMER_REALMS,
			"You can't use more than " + decint(MAX_NUMER_REALMS) + " realms");

		cout << "Loading Realm " << realm_name << "." << endl;

		temprealm = new Realm( realm_name, config.realm_data_path+realm_name );
		Realms->push_back( temprealm );
		++realm_counter;

		//To-Fix - Nasty kludge assuming 'britannia' is the default realm
		//May want to make this configurable in later core releases.
		if( realm_name == string("britannia") )
			main_realm = temprealm;
	}
//	main_realm = new DummyRealm();
	baserealm_count = realm_counter;
	shadowrealm_count = 0;
	if (realm_counter > 0)
		return true;
	else
		return false;
}

Realm* find_realm(const string name)
{
	vector<Realm*>::iterator itr;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		if( (*itr)->name() == name )
			return *itr;
	}
	return NULL;
}

bool defined_realm(const string name)
{
	vector<Realm*>::iterator itr;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		if( (*itr)->name() == name )
			return true;
	}
	return false;
}

void add_realm(const string name, Realm* base)
{
	Realm* r = new Realm(name,base);
	r->shadowid = ++shadowrealm_count;
	shadowrealms_by_id[r->shadowid]=r;
	Realms->push_back( r );
}

void remove_realm(const string name)
{
	vector<Realm*>::iterator itr;
	for(itr = Realms->begin(); itr != Realms->end(); ++itr)
	{
		if( (*itr)->name() == name )
		{
			shadowrealms_by_id[(*itr)->shadowid] = NULL; // used inside the decaythread
			delete *itr;
			Realms->erase(itr);
			break;
		}
	}
}

#include "plib/mapserver.h"

UMulti* uo_find_supporting_multi( MultiList& mvec, int z );

#if ENABLE_OLD_MAPCODE

class DummyRealm : public Realm
{
public:
	DummyRealm() {}

	virtual bool walkheight( const Character* chr, int x, int y, int oldz,
				int* newz, UMulti** pmulti, Item** pwalkon )
	{
		return ::uo_walkheight( chr, x, y, oldz, newz, pmulti, pwalkon );
	}
	bool walkheight(int x, int y, int oldz,
				int* newz,
				UMulti** pmulti, Item** pwalkon,
				bool doors_block = true,
				MOVEMODE movemode = MOVEMODE_LAND )
	{
		return ::uo_walkheight( x, y, oldz, newz, pmulti, pwalkon, doors_block, movemode );
	}


	bool dropheight( unsigned short x,
					 unsigned short y,
					 int chrz,
					 int* newz,
					 UMulti** pmulti )
	{
		return ::uo_dropheight( x, y, chrz, newz, pmulti );
	}

	bool has_los( const LosObj& att, const LosObj& tgt )
	{
		return ::uo_has_los( att, tgt );
	}
	bool has_los( const UObject& iatt, const UObject& itgt )
	{
		return ::uo_has_los( iatt, itgt );
	}
	bool has_los( const Character& iatt, const UObject& itgt )
	{
		return ::uo_has_los( iatt, itgt );
	}
	bool passable( unsigned short x, unsigned short y )
	{
		return ::uo_passable( x, y );
	}
	UMulti* find_supporting_multi( unsigned short x, unsigned short y, int z )
	{
		return ::uo_find_supporting_multi( x, y, z );
	}
	UMulti* find_supporting_multi( MultiList& mvec, int z )
	{
		return ::uo_find_supporting_multi( mvec, z );
	}
};

#endif


