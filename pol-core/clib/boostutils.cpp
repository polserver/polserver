
#include "boostutils.h"
namespace Pol {
  namespace boost_utils {
#ifdef DEBUG_FLYWEIGHT
    std::vector<bucket_query*> debug_flyweight_queries;
#endif

    /**
     * Flyweight automatic initialization is not thread safe.
     * This takes care of manually initializing the used string types.
     * Must be called only once, in single thread mode
     *
     * @url http://www.boost.org/doc/libs/1_50_0/libs/flyweight/doc/tutorial/technical.html
     * @author Bodom, 12-25-2015
     */
    bool flystring_init()
    {
      return
        cprop_name_flystring::init() &&
        cprop_value_flystring::init() &&
        cfg_key_flystring::init() &&
        object_name_flystring::init() &&
        script_name_flystring::init() &&
        npctemplate_name_flystring::init() &&
        function_name_flystring::init();
    }

  }
}
