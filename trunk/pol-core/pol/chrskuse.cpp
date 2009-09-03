/*
History
=======

Notes
=======

*/

#include "clib/stl_inc.h"

#include "cfgemod.h"
#include "osemod.h"
#include "scrsched.h"
#include "uoemod.h"
#include "uoexec.h"

#include "charactr.h"




bool Character::start_skill_script( EScriptProgram* prog )
{
    return start_script( prog, true );
}

