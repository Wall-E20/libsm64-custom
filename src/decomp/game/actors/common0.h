#ifndef COMMON0_H
#define COMMON0_H

/* Shim for n64decomp's actors/common0.h, which data/behavior_data.c includes.
 *
 * Upstream this holds the shared constants for actors/goomba, actors/coin,
 * actors/star and friends: the GOOMBA_BP_* behavior-parameter masks, the COIN_*
 * set, and so on.
 *
 * The goomba subset already lives in src/object_constants_min.h alongside the
 * OBJ_FLAG_* and OBJ_MOVE_* set the actor port needed, because goomba.inc.c needs
 * the same values. Forwarding keeps a single definition instead of two that drift.
 *
 * TODO(actors): when coin/star behavior scripts are added, move their constants
 * out of object_constants_min.h into a real per-actor header as upstream does.
 */

#include "../../../object_constants_min.h"

#endif // COMMON0_H
