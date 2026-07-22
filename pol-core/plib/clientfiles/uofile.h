/** @file
 * Reader API over the raw UO client data files (map/statics/tiledata/verdata),
 * used by uoconvert and uotool -- pol reads converted realm files instead.
 * The reader state and every operation now live on UoClientFiles -- see
 * uoclientfiles.h. uoconvert owns an instance and threads it explicitly;
 * uotool keeps a process-wide one behind its own uofiles() accessor.
 *
 * @par History
 * - 2005/07/05 Shinigami: added uoconvert.cfg option *StaticsPerBlock (hard limit is set to 10000)
 * - 2005/07/16 Shinigami: added uoconvert.cfg flag ShowIllegalGraphicWarning
 * - 2006/04/09 Shinigami: added uoconvert.cfg flag ShowRoofAndPlatformWarning
 */


#ifndef UOFILE_H
#define UOFILE_H

#include "../clidata.h"
#include "../ustruct.h"

#include <span>
#include <vector>


#define MAX_STATICS_PER_BLOCK 10000

#endif
