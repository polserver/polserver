/*
History
=======


Notes
=======

*/


void send_sysmessage(Client *client, const string& text, 
                     unsigned short font = 3, unsigned short color = 0x3B2);
void send_sysmessage(Client *client, const std::wstring& wtext, const char lang[4],
                     unsigned short font = 3, unsigned short color = 0x3B2);
void send_nametext( Client *client, const Character *chr, const string& str );
