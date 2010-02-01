/*
History
=======


Notes
=======

*/

int read_uox_wsc();
int read_data();
int write_data( unsigned long& dirty_writes, unsigned long& clean_writes, unsigned long& elapsed_ms );
void read_account_data();
void read_starting_locations();
void read_gameservers();
