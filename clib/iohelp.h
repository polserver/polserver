/*
History
=======

Notes
=======

*/

#ifndef CLIB_IOHELP_H
#define CLIB_IOHELP_H

void open_file( fstream& ofs, string& filename, ios::openmode mode );
void open_file( ofstream& ofs, string& filename, ios::openmode mode );
void open_file( ifstream& ofs, string& filename, ios::openmode mode );

#endif
