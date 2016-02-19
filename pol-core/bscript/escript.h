/** @file
 *
 * @par History
 */


#ifndef H_ESCRIPT_H
#define H_ESCRIPT_H

/*
    GOTO/GOSUB support removed 9/19/1998 Syz
    break statement and correct local handling makes them troublesome.
    their need is dubious since loops can be labelled, and
    break/continue can take a label.
*/

#define ESCRIPT_INCLUDE_GOTO 0
namespace Pol
{
namespace Bscript
{
}
}

#endif  // H_ESCRIPT_H
