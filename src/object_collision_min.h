#pragma once

#include "decomp/include/types.h"
#include "decomp/game/object_list_processor.h"   /* for enum ObjectList */

#ifdef __cplusplus
extern "C" {
#endif

struct Object;

s32 detect_object_hitbox_overlap(struct Object *a, struct Object *b);
s32 detect_object_hurtbox_overlap(struct Object *a, struct Object *b);
s32 check_object_collision_pair(struct Object *a, struct Object *b);
void clear_object_collision(enum ObjectList objType);
void detect_object_collisions(void);
#ifdef __cplusplus
}
#endif