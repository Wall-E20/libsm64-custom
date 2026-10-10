#ifndef BEHAVIOR_DATA_H
#define BEHAVIOR_DATA_H

#include "types.h"

/*
 * Behavior script table.
 *
 * Definitions live in data/behavior_data.c. Adding an actor means adding its
 * script here and there -- nothing else needs to change: the interpreter in
 * game/behavior_script_min.c dispatches CALL_NATIVE to the bhv_* functions by
 * pointer, and actorMgr reads the object list recorded by BEGIN() for collision
 * filtering.
 */
/* BehaviorScript is a scalar (uintptr_t) in this fork and upstream, so these
 * declare pointers to the first command word. data/behavior_data.c defines them as
 * `const BehaviorScript bhvGoomba[] = {...}`, which decays to this same type. */
extern const BehaviorScript bhvGoomba[];
extern const BehaviorScript bhvGoombaTripletSpawner[];

#endif // BEHAVIOR_DATA_H
