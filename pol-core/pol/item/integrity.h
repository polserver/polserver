/** @file
 *
 * Does every item agree with the registry that holds it?
 *
 * An item's Location names exactly one home, and each home keeps its own list. Those are two
 * representations of one fact, and nothing reconciles them at write time -- relocate() maintains
 * both, but a caller that reaches around it, or a registry that forgets to unlink, leaves them
 * disagreeing with no symptom until something much later reads the wrong one.
 *
 * This walks both directions and reports what it finds. It never aborts: the point is to enumerate
 * a whole world's worth of disagreement in one pass, not to stop at the first.
 */

#ifndef ITEM_INTEGRITY_H
#define ITEM_INTEGRITY_H

namespace Pol::Items
{
struct IntegrityReport
{
  unsigned checks = 0;      ///< location/registry pairings examined, counting both directions
  unsigned violations = 0;  ///< disagreements found, each already logged

  bool ok() const { return violations == 0; }
};

/**
 * Check that every item's Location and every registry's contents describe the same world.
 *
 * @warning Requires PolLock, and reads a great deal of the world: this is a diagnostic, not
 *          something to put on a timer.
 */
IntegrityReport check_item_integrity();
}  // namespace Pol::Items

#endif
