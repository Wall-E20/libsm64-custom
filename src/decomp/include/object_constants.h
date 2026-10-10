#ifndef OBJECT_CONSTANTS_H
#define OBJECT_CONSTANTS_H

/*
 * Shim for n64decomp's include/object_constants.h, which data/behavior_data.c
 * includes.
 *
 * Upstream this header is ~2000 lines of constants for every object in the game.
 * Only the handful that appear in the behavior scripts are needed here, and those
 * already live in src/object_constants_min.h (the pruned set the actor port
 * needed). Re-declaring them here would risk the two drifting apart, so this
 * forwards instead.
 *
 * enum ObjectList is not here: it is an enum, not a macro, and lives in
 * game/object_list_processor.h, which behavior_data.c includes separately.
 */

#include "../../object_constants_min.h"

#endif // OBJECT_CONSTANTS_H
