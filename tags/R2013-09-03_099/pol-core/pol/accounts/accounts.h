/*
History
=======
2005/05/24 Shinigami: added delete_account
2005/05/25 Shinigami: added duplicate_account
2009/08/06 MuadDib:   Removed PasswordOnlyHash support


Notes
=======

*/

#ifndef ACCOUNTS_H
#define ACCOUNTS_H

Account* create_new_account( const std::string& acctname, const std::string& password, bool enabled );
Account* duplicate_account( const std::string& oldacctname, const std::string& newacctname );
Account* find_account( const char* acctname );
int delete_account( const char* acctname );
void write_account_data();
extern bool accounts_txt_dirty;
#endif
