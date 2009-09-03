/*
History
=======

Notes
=======

*/

#ifndef PLIB_REALMDESCRIPTOR_H
#define PLIB_REALMDESCRIPTOR_H

class ConfigElem;

class RealmDescriptor
{
public:
    static RealmDescriptor Load( const string& realm_name, const string& realm_path="" );
	RealmDescriptor();
    // default copy ctor and operator= are okay.

    const string name;
	const string file_path;
    const unsigned width;
    const unsigned height;
    const unsigned uomapid;         // map[n].mul, need to be able to tell the client this.
    const bool uodif;               // use the *dif files?
	const unsigned num_map_patches;
	const unsigned num_static_patches;
	const unsigned season;
    const string mapserver_type;    // "memory" or "file"

    string path( const string& filename ) const;
	bool operator==(const RealmDescriptor& rdesc) const
	{
		return (name == rdesc.name && uomapid == rdesc.uomapid);
	}
	RealmDescriptor & operator=( const RealmDescriptor & ) { return *this; }
private:
    RealmDescriptor( const string& realm_name, const string& realm_path, ConfigElem& elem );
};

#endif
