/*
History
=======


Notes
=======

*/

#ifndef BINARYSCROBJ_H
#define BINARYSCROBJ_H

#include "../bscript/bobject.h"
#include "../clib/rawtypes.h"

class BinFile
{
public:
    BinFile();
    BinFile( const string& filename, ios::openmode mode );
    ~BinFile();

    bool Open( const string& filename, ios::openmode mode );
    void Close();

    template<class T>
        bool Read( T& val )
        {
            return ReadBuffer( &val, sizeof(T) );
        }

    template<class T>
        bool Read( T* arr, unsigned count )
        {
            return ReadBuffer( arr, sizeof(T) * count );
        }

	template<class T>
        bool Write( T& val )
        {
            return WriteBuffer( &val, sizeof(T) );
        }

	bool WriteString(const char* chr, unsigned len);
	bool Seek( fstream::pos_type abs_offset, ios::seekdir origin );
    fstream::pos_type FileSize(Executor& exec);
	fstream::pos_type Tell();
	int Peek();
	void Flush();
	bool IsOpen();


private:
    bool ReadBuffer( void* buffer, streamsize length );
	bool WriteBuffer( void* buffer, streamsize length );

    fstream _file;
};

class BBinaryfile : public BObjectImp
{
public:
    BBinaryfile();
	BBinaryfile(string filename, unsigned short mode, bool _bigendian);
	~BBinaryfile();
	virtual BObjectRef get_member( const char* membername );
    virtual BObjectRef get_member_id( const int id ); //id test
    virtual BObjectImp* call_method( const char* methodname, Executor& ex );
    virtual BObjectImp* call_method_id( const int id, Executor& ex );
    virtual BObjectImp* copy() const;
    virtual std::string getStringRep() const;
	virtual unsigned long sizeEstimate() const { return 0; }
    virtual const char* typeOf() const { return "BinaryFile"; }
	virtual bool isTrue() const;
	virtual bool isEqual(const BObjectImp& objimp) const;

private:
	mutable BinFile file;
	string _filename;
	ios::openmode openmode;
	bool bigendian;
};


#endif