/** @file
 *
 * @par History
 */


#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef NDEBUG
#define OPT_DEBUG 0
#else
#define OPT_DEBUG 1
#endif

/* was defaulted to OPT_DEBUG */
#ifndef OPT_LOG_CLIENT_DATA
#define OPT_LOG_CLIENT_DATA		0
#endif
namespace Pol {
  namespace Core {
  }
}


#endif
