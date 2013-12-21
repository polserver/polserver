/*
History
=======


Notes
=======

*/

#ifndef MKSCROBJ_H
#define MKSCROBJ_H
namespace Pol {
  namespace Items {
	class Item;
  }
  namespace Mobile {
	class Character;
  }
  namespace Bscript {
	class BObjectImp;
  }
  namespace Multi {
	class UBoat;
  }
  namespace Core {
	Bscript::BObjectImp* make_mobileref( Mobile::Character* chr );
	Bscript::BObjectImp* make_itemref( Items::Item* item );
	Bscript::BObjectImp* make_boatref( Multi::UBoat* boat );
  }
}
#endif
