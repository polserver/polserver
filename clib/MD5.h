/*
History
=======

Notes
=======

*/

#ifndef MD5_H
#define MD5_H

bool MD5_Encrypt(const string& in, string& out);
bool MD5_Compare(const string& a, const string& b);
void MD5_Cleanup();

#endif
