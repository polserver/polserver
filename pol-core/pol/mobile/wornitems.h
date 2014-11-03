#ifndef MOBILE_WORNITEMS_H
#define MOBILE_WORNITEMS_H

#include "../containr.h"

namespace Pol {
    namespace Core {

        class WornItemsContainer : public UContainer
        {
            typedef UContainer base;
        public:
            WornItemsContainer();
            virtual ~WornItemsContainer() {};
            virtual size_t estimatedSize() const;

            virtual Bscript::BObjectImp* make_ref();
            virtual Mobile::Character* get_chr_owner();
            Mobile::Character* chr_owner;

            virtual UObject* owner();
            virtual const UObject* owner() const;
            virtual UObject* self_as_owner();
            virtual const UObject* self_as_owner() const;

            virtual void for_each_item(void(*f)(Items::Item* item, void* a), void* arg);

            Items::Item* GetItemOnLayer(unsigned idx) const;
            void PutItemOnLayer(Item* item);
            void RemoveItemFromLayer(Item* item);

            virtual bool saveonexit() const;
            virtual void saveonexit(bool newvalue);

            void print(Clib::StreamWriter& sw_pc, Clib::StreamWriter& sw_equip) const;
        };

        inline Items::Item *WornItemsContainer::GetItemOnLayer(unsigned idx) const
        {
            if (Items::valid_equip_layer(idx))
                return ITEM_ELEM_PTR(contents_[idx]);

            return NULL;
        }

		inline Mobile::Character *WornItemsContainer::get_chr_owner()
        {
            return chr_owner;
        }

    }
}

#endif
