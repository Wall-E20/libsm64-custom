
#include "../shim.h"
#include "../global_state.h"
#include "../include/types.h"
#include "../include/sm64.h"
#include "../include/object_fields.h"
#include "../../object_constants_min.h"
#include "../../object_helpers_min.h"
#include "behavior_script.h"
#include "object_list_processor.h"
#include "object_stuff.h"   /* ACTIVE_FLAG_DEACTIVATED */
#include "../engine/graph_node.h"
#include "../engine/math_util.h"
#include "../engine/surface_collision.h"
#include "memory.h"

#include <stdio.h>

#define BHV_CMD_WORD(index) ((u32)(uintptr_t)gCurBhvCommand[index])
#define BHV_CMD_PTR(index)  ((uintptr_t)gCurBhvCommand[index])

#define BHV_CMD_GET_1ST_U8(index)  (u8)((BHV_CMD_WORD(index) >> 24) & 0xFF)
#define BHV_CMD_GET_2ND_U8(index)  (u8)((BHV_CMD_WORD(index) >> 16) & 0xFF)
#define BHV_CMD_GET_3RD_U8(index)  (u8)((BHV_CMD_WORD(index) >> 8) & 0xFF)
#define BHV_CMD_GET_4TH_U8(index)  (u8)((BHV_CMD_WORD(index)) & 0xFF)

#define BHV_CMD_GET_1ST_S16(index) (s16)(BHV_CMD_WORD(index) >> 16)
#define BHV_CMD_GET_2ND_S16(index) (s16)(BHV_CMD_WORD(index) & 0xFFFF)

#define BHV_CMD_GET_U32(index)     BHV_CMD_WORD(index)
#define BHV_CMD_GET_VPTR(index)    (void *)BHV_CMD_PTR(index)

#define BHV_CMD_GET_ADDR_OF_CMD(index) (uintptr_t)(&gCurBhvCommand[index])



static void cur_obj_bhv_stack_push(uintptr_t bhvAddr) {
    gCurrentObject->bhvStack[gCurrentObject->bhvStackIndex] = bhvAddr;
    gCurrentObject->bhvStackIndex++;
}

static uintptr_t cur_obj_bhv_stack_pop(void) {
    uintptr_t bhvAddr;
    gCurrentObject->bhvStackIndex--;
    bhvAddr = gCurrentObject->bhvStack[gCurrentObject->bhvStackIndex];
    return bhvAddr;
}


static s32 bhv_cmd_unsupported(u8 opcode, const char *name) {
    printf("behavior script: %s (0x%02X) is not supported by this fork; "
           "script halted\n", name, opcode);
    return BHV_PROC_BREAK;
}



static s32 bhv_cmd_hide(void) {
    cur_obj_hide();
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_disable_rendering(void) {
    gCurrentObject->header.gfx.node.flags &= ~GRAPH_RENDER_ACTIVE;
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_billboard(void) {
    gCurrentObject->header.gfx.node.flags |= GRAPH_RENDER_BILLBOARD;
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_set_model(void) {
    s32 modelID = BHV_CMD_GET_2ND_S16(0);
    gCurrentObject->oModelID = modelID;
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_spawn_child(void) {
    return bhv_cmd_unsupported(0x1C, "SPAWN_CHILD");
}
static s32 bhv_cmd_spawn_obj(void) {
    return bhv_cmd_unsupported(0x2C, "SPAWN_OBJ");
}
static s32 bhv_cmd_spawn_child_with_param(void) {
    return bhv_cmd_unsupported(0x29, "SPAWN_CHILD_WITH_PARAM");
}
static s32 bhv_cmd_spawn_water_droplet(void) {
    return bhv_cmd_unsupported(0x37, "SPAWN_WATER_DROPLET");
}


static s32 bhv_cmd_deactivate(void) {
    gCurrentObject->activeFlags = ACTIVE_FLAG_DEACTIVATED;
    return BHV_PROC_BREAK;
}

static s32 bhv_cmd_break(void) {
    return BHV_PROC_BREAK;
}

static s32 bhv_cmd_break_unused(void) {
    return BHV_PROC_BREAK;
}

static s32 bhv_cmd_call(void) {
    gCurBhvCommand++;
    cur_obj_bhv_stack_push(BHV_CMD_GET_ADDR_OF_CMD(1));
    gCurBhvCommand = (const BehaviorScript *)BHV_CMD_GET_VPTR(0);
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_return(void) {
    gCurBhvCommand = (const BehaviorScript *)cur_obj_bhv_stack_pop();
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_delay(void) {
    s16 num = BHV_CMD_GET_2ND_S16(0);
    if (gCurrentObject->bhvDelayTimer < num - 1) {
        gCurrentObject->bhvDelayTimer++;
    } else {
        gCurrentObject->bhvDelayTimer = 0;
        gCurBhvCommand++;
    }
    return BHV_PROC_BREAK;
}

static s32 bhv_cmd_delay_var(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s32 num = cur_obj_get_int(field);
    if (gCurrentObject->bhvDelayTimer < num - 1) {
        gCurrentObject->bhvDelayTimer++;
    } else {
        gCurrentObject->bhvDelayTimer = 0;
        gCurBhvCommand++;
    }
    return BHV_PROC_BREAK;
}

static s32 bhv_cmd_goto(void) {
    gCurBhvCommand++;
    gCurBhvCommand = (const BehaviorScript *)BHV_CMD_GET_VPTR(0);
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_begin_repeat_unused(void) {
    s32 count = BHV_CMD_GET_2ND_U8(0);
    cur_obj_bhv_stack_push(BHV_CMD_GET_ADDR_OF_CMD(1));
    cur_obj_bhv_stack_push(count);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_begin_repeat(void) {
    s32 count = BHV_CMD_GET_2ND_S16(0);
    cur_obj_bhv_stack_push(BHV_CMD_GET_ADDR_OF_CMD(1));
    cur_obj_bhv_stack_push(count);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_end_repeat(void) {
    u32 count = cur_obj_bhv_stack_pop();
    count--;
    if (count != 0) {
        gCurBhvCommand = (const BehaviorScript *)cur_obj_bhv_stack_pop();
        cur_obj_bhv_stack_push(BHV_CMD_GET_ADDR_OF_CMD(0));
        cur_obj_bhv_stack_push(count);
    } else {
        cur_obj_bhv_stack_pop();
        gCurBhvCommand++;
    }
    return BHV_PROC_BREAK;
}

static s32 bhv_cmd_end_repeat_continue(void) {
    u32 count = cur_obj_bhv_stack_pop();
    count--;
    if (count != 0) {
        gCurBhvCommand = (const BehaviorScript *)cur_obj_bhv_stack_pop();
        cur_obj_bhv_stack_push(BHV_CMD_GET_ADDR_OF_CMD(0));
        cur_obj_bhv_stack_push(count);
    } else {
        cur_obj_bhv_stack_pop();
        gCurBhvCommand++;
    }
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_begin_loop(void) {
    cur_obj_bhv_stack_push(BHV_CMD_GET_ADDR_OF_CMD(1));
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_end_loop(void) {
    gCurBhvCommand = (const BehaviorScript *)cur_obj_bhv_stack_pop();
    cur_obj_bhv_stack_push(BHV_CMD_GET_ADDR_OF_CMD(0));
    return BHV_PROC_BREAK;
}


typedef void (*NativeBhvFunc)(void);
static s32 bhv_cmd_call_native(void) {
    NativeBhvFunc behaviorFunc = BHV_CMD_GET_VPTR(1);
    behaviorFunc();
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_set_float(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    f32 value = BHV_CMD_GET_2ND_S16(0);
    cur_obj_set_float(field, value);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_set_int(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s16 value = BHV_CMD_GET_2ND_S16(0);
    cur_obj_set_int(field, value);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_set_int_unused(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s32 value = BHV_CMD_GET_2ND_S16(1);
    cur_obj_set_int(field, value);
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_set_random_float(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    f32 min = BHV_CMD_GET_2ND_S16(0);
    f32 range = BHV_CMD_GET_1ST_S16(1);
    cur_obj_set_float(field, (range * random_float()) + min);
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_set_random_int(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s32 min = BHV_CMD_GET_2ND_S16(0);
    s32 range = BHV_CMD_GET_1ST_S16(1);
    cur_obj_set_int(field, (s32)(range * random_float()) + min);
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_set_int_rand_rshift(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s32 min = BHV_CMD_GET_2ND_S16(0);
    s32 rshift = BHV_CMD_GET_1ST_S16(1);
    cur_obj_set_int(field, (random_u16() >> rshift) + min);
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_add_random_float(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    f32 min = BHV_CMD_GET_2ND_S16(0);
    f32 range = BHV_CMD_GET_1ST_S16(1);
    cur_obj_set_float(field, cur_obj_get_float(field) + min + (range * random_float()));
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_add_int_rand_rshift(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s32 min = BHV_CMD_GET_2ND_S16(0);
    s32 rshift = BHV_CMD_GET_1ST_S16(1);
    s32 rnd = random_u16();
    cur_obj_set_int(field, (cur_obj_get_int(field) + min) + (rnd >> rshift));
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_add_float(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    f32 value = BHV_CMD_GET_2ND_S16(0);
    cur_obj_add_float(field, value);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_add_int(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s16 value = BHV_CMD_GET_2ND_S16(0);
    cur_obj_add_int(field, value);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_or_int(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s32 value = BHV_CMD_GET_2ND_S16(0);
    value &= 0xFFFF;
    cur_obj_or_int(field, value);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_bit_clear(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s32 value = BHV_CMD_GET_2ND_S16(0);
    value = (value & 0xFFFF) ^ 0xFFFF;
    cur_obj_and_int(field, value);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_load_animations(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    cur_obj_set_vptr(field, BHV_CMD_GET_VPTR(1));
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_animate(void) {
    s32 animIndex = BHV_CMD_GET_2ND_U8(0);
    struct Animation **animations = gCurrentObject->oAnimations;
    geo_obj_init_animation(&gCurrentObject->header.gfx, &animations[animIndex]);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_drop_to_floor(void) {
    f32 x = gCurrentObject->oPosX;
    f32 y = gCurrentObject->oPosY;
    f32 z = gCurrentObject->oPosZ;
    f32 floor = find_floor_height(x, y + 200.0f, z);
    gCurrentObject->oPosY = floor;
    gCurrentObject->oMoveFlags |= OBJ_MOVE_ON_GROUND;
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_nop_1(void) {
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}
static s32 bhv_cmd_nop_2(void) {
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}
static s32 bhv_cmd_nop_3(void) {
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_sum_float(void) {
    u32 fieldDst = BHV_CMD_GET_2ND_U8(0);
    u32 fieldSrc1 = BHV_CMD_GET_3RD_U8(0);
    u32 fieldSrc2 = BHV_CMD_GET_4TH_U8(0);
    cur_obj_set_float(fieldDst, cur_obj_get_float(fieldSrc1) + cur_obj_get_float(fieldSrc2));
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_sum_int(void) {
    u32 fieldDst = BHV_CMD_GET_2ND_U8(0);
    u32 fieldSrc1 = BHV_CMD_GET_3RD_U8(0);
    u32 fieldSrc2 = BHV_CMD_GET_4TH_U8(0);
    cur_obj_set_int(fieldDst, cur_obj_get_int(fieldSrc1) + cur_obj_get_int(fieldSrc2));
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_set_hitbox(void) {
    s16 radius = BHV_CMD_GET_1ST_S16(1);
    s16 height = BHV_CMD_GET_2ND_S16(1);
    gCurrentObject->hitboxRadius = radius;
    gCurrentObject->hitboxHeight = height;
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_set_hurtbox(void) {
    s16 radius = BHV_CMD_GET_1ST_S16(1);
    s16 height = BHV_CMD_GET_2ND_S16(1);
    gCurrentObject->hurtboxRadius = radius;
    gCurrentObject->hurtboxHeight = height;
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}

static s32 bhv_cmd_set_hitbox_with_offset(void) {
    s16 radius = BHV_CMD_GET_1ST_S16(1);
    s16 height = BHV_CMD_GET_2ND_S16(1);
    s16 downOffset = BHV_CMD_GET_1ST_S16(2);
    gCurrentObject->hitboxRadius = radius;
    gCurrentObject->hitboxHeight = height;
    gCurrentObject->hitboxDownOffset = downOffset;
    gCurBhvCommand += 3;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_nop_4(void) {
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_begin(void) {
    gCurrentObject->oObjList = BHV_CMD_GET_2ND_U8(0);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_load_collision_data(void) {
    gCurrentObject->collisionData = (u32 *)BHV_CMD_GET_VPTR(1);
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_set_home(void) {
    gCurrentObject->oHomeX = gCurrentObject->oPosX;
    gCurrentObject->oHomeY = gCurrentObject->oPosY;
    gCurrentObject->oHomeZ = gCurrentObject->oPosZ;
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_set_interact_type(void) {
    gCurrentObject->oInteractType = BHV_CMD_GET_U32(1);
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_set_interact_subtype(void) {
    gCurrentObject->oInteractionSubtype = BHV_CMD_GET_U32(1);
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_scale(void) {
    s16 percent = BHV_CMD_GET_2ND_S16(0);
    cur_obj_scale(percent / 100.0f);
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_set_obj_physics(void) {
    gCurrentObject->oWallHitboxRadius = BHV_CMD_GET_1ST_S16(1);
    gCurrentObject->oGravity = BHV_CMD_GET_2ND_S16(1) / 100.0f;
    gCurrentObject->oBounciness = BHV_CMD_GET_1ST_S16(2) / 100.0f;
    gCurrentObject->oDragStrength = BHV_CMD_GET_2ND_S16(2) / 100.0f;
    gCurrentObject->oFriction = BHV_CMD_GET_1ST_S16(3) / 100.0f;
    gCurrentObject->oBuoyancy = BHV_CMD_GET_2ND_S16(3) / 100.0f;
    gCurBhvCommand += 5;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_parent_bit_clear(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s32 value = BHV_CMD_GET_U32(1);
    value = value ^ 0xFFFFFFFF;
    if (gCurrentObject->parentObj != NULL) {
        obj_and_int(gCurrentObject->parentObj, field, value);
    }
    gCurBhvCommand += 2;
    return BHV_PROC_CONTINUE;
}


static s32 bhv_cmd_animate_texture(void) {
    u8 field = BHV_CMD_GET_2ND_U8(0);
    s16 rate = BHV_CMD_GET_2ND_S16(0);
    if (rate != 0 && (gGlobalTimer % rate) == 0) {
        cur_obj_add_int(field, 1);
    }
    gCurBhvCommand++;
    return BHV_PROC_CONTINUE;
}


typedef s32 (*BhvCommandProc)(void);


static BhvCommandProc BehaviorCmdTable[] = {
    bhv_cmd_begin,                    /* 0x00 */
    bhv_cmd_delay,                    /* 0x01 */
    bhv_cmd_call,                     /* 0x02 */
    bhv_cmd_return,                   /* 0x03 */
    bhv_cmd_goto,                     /* 0x04 */
    bhv_cmd_begin_repeat,             /* 0x05 */
    bhv_cmd_end_repeat,               /* 0x06 */
    bhv_cmd_end_repeat_continue,      /* 0x07 */
    bhv_cmd_begin_loop,               /* 0x08 */
    bhv_cmd_end_loop,                 /* 0x09 */
    bhv_cmd_break,                    /* 0x0A */
    bhv_cmd_break_unused,             /* 0x0B */
    bhv_cmd_call_native,              /* 0x0C */
    bhv_cmd_add_float,                /* 0x0D */
    bhv_cmd_set_float,                /* 0x0E */
    bhv_cmd_add_int,                  /* 0x0F */
    bhv_cmd_set_int,                  /* 0x10 */
    bhv_cmd_or_int,                   /* 0x11 */
    bhv_cmd_bit_clear,                /* 0x12 */
    bhv_cmd_set_int_rand_rshift,      /* 0x13 */
    bhv_cmd_set_random_float,         /* 0x14 */
    bhv_cmd_set_random_int,           /* 0x15 */
    bhv_cmd_add_random_float,         /* 0x16 */
    bhv_cmd_add_int_rand_rshift,      /* 0x17 */
    bhv_cmd_nop_1,                    /* 0x18 */
    bhv_cmd_nop_2,                    /* 0x19 */
    bhv_cmd_nop_3,                    /* 0x1A */
    bhv_cmd_set_model,                /* 0x1B */
    bhv_cmd_spawn_child,              /* 0x1C */
    bhv_cmd_deactivate,               /* 0x1D */
    bhv_cmd_drop_to_floor,            /* 0x1E */
    bhv_cmd_sum_float,                /* 0x1F */
    bhv_cmd_sum_int,                  /* 0x20 */
    bhv_cmd_billboard,                /* 0x21 */
    bhv_cmd_hide,                     /* 0x22 */
    bhv_cmd_set_hitbox,               /* 0x23 */
    bhv_cmd_nop_4,                    /* 0x24 */
    bhv_cmd_delay_var,                /* 0x25 */
    bhv_cmd_begin_repeat_unused,      /* 0x26 */
    bhv_cmd_load_animations,          /* 0x27 */
    bhv_cmd_animate,                  /* 0x28 */
    bhv_cmd_spawn_child_with_param,   /* 0x29 */
    bhv_cmd_load_collision_data,      /* 0x2A */
    bhv_cmd_set_hitbox_with_offset,   /* 0x2B */
    bhv_cmd_spawn_obj,                /* 0x2C */
    bhv_cmd_set_home,                 /* 0x2D */
    bhv_cmd_set_hurtbox,              /* 0x2E */
    bhv_cmd_set_interact_type,        /* 0x2F */
    bhv_cmd_set_obj_physics,          /* 0x30 */
    bhv_cmd_set_interact_subtype,     /* 0x31 */
    bhv_cmd_scale,                    /* 0x32 */
    bhv_cmd_parent_bit_clear,         /* 0x33 */
    bhv_cmd_animate_texture,          /* 0x34 */
    bhv_cmd_disable_rendering,        /* 0x35 */
    bhv_cmd_set_int_unused,           /* 0x36 */
    bhv_cmd_spawn_water_droplet,      /* 0x37 */
};

#define BEHAVIOR_CMD_TABLE_LEN ((s32)(sizeof(BehaviorCmdTable) / sizeof(BehaviorCmdTable[0])))


void sm64_behavior_run_script(struct Object *obj) {
    s16 objFlags;
    BhvCommandProc bhvCmdProc;
    s32 bhvProcResult;

    gCurrentObject = obj;

    objFlags = obj->oFlags;

    if (objFlags & OBJ_FLAG_COMPUTE_DIST_TO_MARIO) {
        obj->oDistanceToMario = dist_between_objects(obj, gMarioObject);
    }
    if (objFlags & OBJ_FLAG_COMPUTE_ANGLE_TO_MARIO) {
        obj->oAngleToMario = obj_angle_to_object(obj, gMarioObject);
    }

    gCurBhvCommand = obj->curBhvCommand;

    do {
        s32 opcode = (s32)(BHV_CMD_WORD(0) >> 24) & 0xFF;
        if (opcode >= BEHAVIOR_CMD_TABLE_LEN) {
            printf("behavior script: bad opcode 0x%02X; script halted\n", opcode);
            return;
        }
        bhvCmdProc = BehaviorCmdTable[opcode];
        bhvProcResult = bhvCmdProc();
    } while (bhvProcResult == BHV_PROC_CONTINUE);

    obj->curBhvCommand = gCurBhvCommand;
}


void sm64_behavior_start_script(struct Object *obj, const BehaviorScript *script) {
    obj->behavior = script;
    obj->curBhvCommand = script;
    obj->bhvStackIndex = 0;
    obj->bhvDelayTimer = 0;
}
