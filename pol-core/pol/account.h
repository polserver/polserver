/*
History
=======
2005/05/25 Shinigami: added void writeto( ConfigElem& elem )
2005/05/25 Shinigami: added getnextfreeslot()
2009/08/06 MuadDib:   Removed PasswordOnlyHash support

Notes
=======

*/

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "ucfg.h"
#include "proplist.h"
#include "reftypes.h"

#include "clib/strset.h"
#include "clib/refptr.h"

class Character;
class Client;
class ConfigElem;

class Account : public ref_counted
{
public:
	explicit Account( ConfigElem& elem );
	~Account();

    const char *name() const;
    const string password() const;
	const string passwordhash() const;
	const string uo_expansion() const;
    bool enabled() const;
    bool banned() const;

	int numchars() const;
    int getnextfreeslot() const;
	
	Character* active_character; 
	
    Character* get_character( int index );
    void set_character( int index, Character* chr );
    void clear_character( int index );

    void readfrom( ConfigElem& elem );
    void writeto( ostream& os );
    void writeto( ConfigElem& elem ) const;

    string default_privlist() const;
    unsigned char default_cmdlevel() const;

	void set_password(string newpass) {password_ = newpass;};
	void set_passwordhash(string newpass) {passwordhash_ = newpass;};

    friend class AccountObjImp;

private:
    vector<CharacterRef> characters_;
    string name_;
    string password_;
	string passwordhash_;
	string uo_expansion_;
    bool enabled_;
    bool banned_;
    PropertyList props_;
    StringSet default_privs_;
    unsigned char default_cmdlevel_;

    StringSet options_;
};

Account *find_account( const char *acctname );

#endif
