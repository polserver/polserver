/*
History
=======


Notes
=======

*/

#ifndef H_STORAGE_H
#define H_STORAGE_H

#ifdef _MSC_VER
#pragma warning(disable:4786)
#endif

#include "../clib/maputil.h"

class ConfigFile;
class ConfigElem;
class Item;

class StorageArea
{
public:
    StorageArea( string name );
    ~StorageArea();

    Item *find_root_item( const string& name );
    void insert_root_item( Item *item );
    bool delete_root_item( const string& name );

    void print( ostream& os ) const;
    void load_item( ConfigElem& elem );
private:

    string _name;

    // TODO: ref_ptr<Item> ?
    typedef map< string, Item*, ci_cmp_pred > Cont;
    Cont _items; // owns its items.

    friend class StorageAreaImp;
    friend class StorageAreaIterator;
    friend void write_dirty_storage( ofstream& );
};

extern ostream& operator<<( ostream& os, const StorageArea& area );

class Storage
{
public:    
    StorageArea* find_area( const string& name );
    StorageArea* create_area( const string& name );
    StorageArea* create_area( ConfigElem& elem );

    void print( ostream& os ) const;
    void read( ConfigFile& cf );
    void clear();
private:
    // TODO: investigate if this could store objects. Does find() 
    // return object copies, or references?
    typedef map<string, StorageArea*> AreaCont;
    AreaCont areas;

    friend class StorageAreasImp;
    friend class StorageAreasIterator;
    friend void write_dirty_storage( ofstream& );
};
extern ostream& operator<<( ostream& os, const Storage& area );

extern Storage storage;

#endif
