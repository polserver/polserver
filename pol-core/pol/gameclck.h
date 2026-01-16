/** @file
 *
 * @par History
 */


#ifndef GAMECLCK_H
#define GAMECLCK_H


namespace Pol::Core
{
using gameclock_t = unsigned int;

/// The functions below deal with reading and updating the gameclock.
/// The state is protected by a mutex.

/// Starts the game clock based on the global cprop "gameclock".
void start_gameclock();

/// Stops the game clock and saves the current value into "gameclock"
void stop_gameclock();

/// Saves the current value into the global cprop "gameclock"
void update_gameclock();

/// Reads the current value of the game clock.
gameclock_t read_gameclock();
}  // namespace Pol::Core

#endif
