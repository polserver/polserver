/*
History
=======
2011/11/28 MuadDib:   Removed last of uox referencing code.

Notes
=======

*/

int read_data();
int write_data( unsigned int& dirty_writes, unsigned int& clean_writes, long long& elapsed_ms );
void read_account_data();
void read_starting_locations();
void read_gameservers();
