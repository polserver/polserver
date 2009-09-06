/*
History
=======

Notes
=======

*/

#include "../../clib/stl_inc.h"

#include "../module/cfgmod.h"
#include "../module/osmod.h"
#include "../scrsched.h"
#include "../uoemod.h"
#include "../uoexec.h"

#include "charactr.h"




bool Character::start_skill_script( EScriptProgram* prog )
{
    return start_script( prog, true );
}

