/*
History
=======


Notes
=======

*/

int read_uox_wsc();
int read_data();
int write_data( unsigned int& dirty_writes, unsigned int& clean_writes, unsigned int& elapsed_ms );
void read_account_data();
void read_starting_locations();
void read_gameservers();
