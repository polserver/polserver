/*
History
=======
2009/09/03 MuadDib:   Relocation of account related cpp/h


Notes
=======

*/

#include "../clib/stl_inc.h"

#include "../clib/endian.h"
#include "../clib/passert.h"
#include "../clib/logfile.h"

#include "accounts/account.h"
#include "objecthash.h"
#include "polcfg.h"
#include "ufunc.h"
#include "uobject.h"

ObjectHash::ObjectHash() :
    hash(),
    reap_iterator( hash.end() )
{};

ObjectHash::~ObjectHash()
{};

bool ObjectHash::Insert(UObject* obj)
{
	OH_iterator itr = hash.find(obj->serial);
	if(itr != hash.end())
	{
		if( config.loglevel >= 5)
			Log("ObjectHash insert failed for object serial %x. (duplicate serial?)\n", obj->serial);
		//cout << "ObjectHash insert failed for object serial " << hex << obj->serial << ". (duplicate serial?)" << endl;
		return false;
	}
	hash.insert(hash.end(), make_pair(obj->serial,UObjectRef(obj)) );
	return true;
}

bool ObjectHash::Remove(u32 serial)
{
//	unsigned int num_erased = hash.erase( serial );
//	return (num_erased>0); 
    return true;
}


UObject* ObjectHash::Find(u32 serial)
{
	OH_iterator itr = hash.find(serial);
	if(itr != hash.end())
		return (itr->second).get();
	else
		return NULL;
}

u32 ObjectHash::GetNextUnusedItemSerial()
{
	u32 curr = GetCurrentItemSerialNumber();
	u32 tempserial = curr +1;

	for(;;)
	{
		//recycle time. when we roll over the max serial, start from the beginning. now the find() below may take some time to get the next unused.
		if(tempserial > ITEMSERIAL_END)
			tempserial = ITEMSERIAL_START;

		if (hash.find(tempserial) != hash.end())
		{
			tempserial++;
			continue;
		}
		else if (dirty_deleted.count(tempserial))
        {
			tempserial++;
            continue;
        }
        else if (clean_deleted.count(tempserial))
        {
   			tempserial++;
            continue;
        }
        else
        {
            break;
        }
	}
	return tempserial;
};

u32 ObjectHash::GetNextUnusedCharSerial()
{
	u32 curr = GetCurrentCharSerialNumber();
	u32 tempserial = curr +1;

    for(;;)
	{
		//recycle time. when we roll over the max serial, start from the beginning. now the find() below may take some time to get the next unused.
		if(tempserial > CHARACTERSERIAL_END)
			tempserial = CHARACTERSERIAL_START;

		if (hash.find(tempserial) != hash.end())
		{
			tempserial++;
			continue;
		}
		else if (dirty_deleted.find(tempserial) != dirty_deleted.end())
        {
			tempserial++;
            continue;
        }
        else if (clean_deleted.find(tempserial) != clean_deleted.end())
        {
   			tempserial++;
            continue;
        }
        else
        {
            break;
        }
	}
	return tempserial;
};

void ObjectHash::PrintContents( std::ostream& os ) const
{
	OH_const_iterator itr, itrend;
	os << "Object Count: " << hash.size() <<endl;
	for(itr = hash.begin(), itrend=hash.end(); itr != itrend; ++itr)
	{
		itr->second->printOn(os);
	}

}

void ObjectHash::Reap()
{
    // this is called every 2 seconds (approximately)
    // iterate through enough of the object hash that it will be swept entirely once every 30 minutes
    // 2 minutes = 120 seconds = 60 reap calls per sweep
    // 30 minutes = 1800 seconds = 900 reap calls per sweep
    
    // first, figure out how many objects to check:
    hs::size_type count = hash.size();
    if (count == 0)
        return;
    hs::size_type count_this = count / 60;
    if (count_this < 1)
        count_this = 1;
    if (reap_iterator == hash.end())
        reap_iterator = hash.begin();

    while (count_this--)
    {
        UObject* obj = (*reap_iterator).second.get();
        
        // We want the objecthash to be the holder of the last reference to an
        // object when it is deleted - hence the ref_counted_count() check.
        if (obj->orphan() && obj->ref_counted_count() == 1)
        {
            dirty_deleted.insert( cfBEu32(obj->serial_ext) );
            hash.erase( reap_iterator++ );
        }
		else
			++reap_iterator;

        if (reap_iterator == hash.end())
        {
            reap_iterator = hash.begin();
            if (reap_iterator == hash.end())
                break;
        }
    }
}

void ObjectHash::Clear()
{
    bool any;
    do
    {
        any = false;
        unsigned skipped = 0;
        for( OH_iterator itr = hash.begin(), itrend=hash.end(); itr != itrend; )
        {
            UObject* obj = (*itr).second.get();

            if (obj->orphan() && obj->ref_counted_count() == 1)
            {
                hash.erase( itr++ );
                any = true;
            }
            else
            {
                ++skipped;
				++itr;
            }
        }
    } while (any );
    if (!hash.empty())
    {
        cout << "Leftover objects in objecthash: " << hash.size() << endl;
        
        // the hash will be cleared after main() exits, with other statics.
        // this usually causes assertion failures and crashes.
        // creating a copy of the internal hash will ensure no refcounts reach zero.
        cout << "Leaking a copy of the objecthash in order to avoid a crash." << endl;
        new hs( hash );
    }
//    hash.clear();
}

#include "mobile/charactr.h"
void ObjectHash::ClearCharacterAccountReferences()
{
	for(OH_const_iterator itr = hash.begin(), itrend=hash.end(); itr != itrend; ++itr)
	{
        UObject* obj = (*itr).second.get();
        if (!obj->orphan() && obj->ismobile())
        {
            Character* chr = static_cast<Character*>(obj);
            //if (!chr->logged_in)
            //{
                chr->acct.clear(); //dave added 9/27/03, accounts and player characters have a mutual 
                                   // reference that prevents them getting cleaned up
                                   // properly. So clear the reference now.
                chr->destroy();
            //}
        }
	}
}

ObjectHash::hs::const_iterator ObjectHash::begin() const
{
    return hash.begin();
}

ObjectHash::hs::const_iterator ObjectHash::end() const
{
    return hash.end();
}

ObjectHash::ds::const_iterator ObjectHash::dirty_deleted_begin() const
{
    return dirty_deleted.begin();
}

ObjectHash::ds::const_iterator ObjectHash::dirty_deleted_end() const
{
    return dirty_deleted.end();
}

void ObjectHash::ClearDeleted()
{
    dirty_deleted.clear();
    clean_deleted.clear();
}

void ObjectHash::CleanDeleted()
{
    clean_deleted.insert( dirty_deleted.begin(), dirty_deleted.end() );
    dirty_deleted.clear();
}

void ObjectHash::RegisterCleanDeletedSerial( u32 serial )
{
    clean_deleted.insert( serial );
}

ObjectHash objecthash;
