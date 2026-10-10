#ifndef BEHAVIOR_SCRIPT_H
#define BEHAVIOR_SCRIPT_H

#include "../include/PR/ultratypes.h"
#include "../include/types.h"

#define BHV_PROC_CONTINUE 0
#define BHV_PROC_BREAK    1

u16 random_u16(void);
float random_float(void);
s32 random_sign(void);
void sm64_behavior_start_script(struct Object *obj, const BehaviorScript *script);
void sm64_behavior_run_script(struct Object *obj);

#endif // BEHAVIOR_SCRIPT_H
