#ifndef OBJECT_CONSTANTS_MIN_H
#define OBJECT_CONSTANTS_MIN_H

#include "decomp/include/object_fields.h"

#define oBhvParams        oBehParams
#define oBhvParams2ndByte oBehParams2ndByte
static struct SM64SurfaceCollisionData *sObjFloor;
static s8 sOrientObjWithFloor = TRUE;

#define OBJ_COL_FLAG_GROUNDED   (1 << 0)
#define OBJ_COL_FLAG_HIT_WALL   (1 << 1)
#define OBJ_COL_FLAG_UNDERWATER (1 << 2)
#define OBJ_COL_FLAG_NO_Y_VEL   (1 << 3)
#define OBJ_COL_FLAGS_LANDED    (OBJ_COL_FLAG_GROUNDED | OBJ_COL_FLAG_NO_Y_VEL)
//oFlags
#define OBJ_FLAG_UPDATE_GFX_POS_AND_ANGLE         (1 <<  0)
#define OBJ_FLAG_MOVE_XZ_USING_FVEL               (1 <<  1)
#define OBJ_FLAG_MOVE_Y_WITH_TERMINAL_VEL         (1 <<  2)
#define OBJ_FLAG_SET_FACE_YAW_TO_MOVE_YAW         (1 <<  3)
#define OBJ_FLAG_SET_FACE_ANGLE_TO_MOVE_ANGLE     (1 <<  4)
#define OBJ_FLAG_0020                             (1 <<  5)
#define OBJ_FLAG_COMPUTE_DIST_TO_MARIO            (1 <<  6)
#define OBJ_FLAG_ACTIVE_FROM_AFAR                 (1 <<  7)
#define OBJ_FLAG_0100                             (1 <<  8)
#define OBJ_FLAG_TRANSFORM_RELATIVE_TO_PARENT     (1 <<  9)
#define OBJ_FLAG_HOLDABLE                         (1 << 10)
#define OBJ_FLAG_SET_THROW_MATRIX_FROM_TRANSFORM  (1 << 11)
#define OBJ_FLAG_1000                             (1 << 12)
#define OBJ_FLAG_COMPUTE_ANGLE_TO_MARIO           (1 << 13)
#define OBJ_FLAG_PERSISTENT_RESPAWN               (1 << 14)
#define OBJ_FLAG_8000                             (1 << 15)
#define OBJ_FLAG_30                               (1 << 30)

//oMoveFlags
#define OBJ_MOVE_LANDED                (1 <<  0)
#define OBJ_MOVE_ON_GROUND             (1 <<  1)
#define OBJ_MOVE_LEFT_GROUND           (1 <<  2)
#define OBJ_MOVE_ENTERED_WATER         (1 <<  3)
#define OBJ_MOVE_AT_WATER_SURFACE      (1 <<  4)
#define OBJ_MOVE_UNDERWATER_OFF_GROUND (1 <<  5)
#define OBJ_MOVE_UNDERWATER_ON_GROUND  (1 <<  6)
#define OBJ_MOVE_IN_AIR                (1 <<  7)
#define OBJ_MOVE_OUT_SCOPE             (1 <<  8)
#define OBJ_MOVE_HIT_WALL              (1 <<  9)
#define OBJ_MOVE_HIT_EDGE              (1 << 10)
#define OBJ_MOVE_ABOVE_LAVA            (1 << 11)
#define OBJ_MOVE_LEAVING_WATER         (1 << 12)
#define OBJ_MOVE_BOUNCE                (1 << 13)
#define OBJ_MOVE_ABOVE_DEATH_BARRIER   (1 << 14)

#define OBJ_MOVE_MASK_ON_GROUND (OBJ_MOVE_LANDED | OBJ_MOVE_ON_GROUND)
#define OBJ_MOVE_MASK_IN_WATER ( \
    OBJ_MOVE_ENTERED_WATER | \
    OBJ_MOVE_AT_WATER_SURFACE | \
    OBJ_MOVE_UNDERWATER_OFF_GROUND | \
    OBJ_MOVE_UNDERWATER_ON_GROUND)

//oAction
#define OBJ_ACT_HORIZONTAL_KNOCKBACK 100
#define OBJ_ACT_VERTICAL_KNOCKBACK   101
#define OBJ_ACT_SQUISHED             102
    #define MOV_YCOIN_ACT_IDLE              0
    #define MOV_YCOIN_ACT_BLINKING          1
    #define MOV_YCOIN_ACT_LAVA_DEATH        100
    #define MOV_YCOIN_ACT_DEATH_PLANE_DEATH 101
    #define MOV_BCOIN_ACT_STILL  0
    #define MOV_BCOIN_ACT_MOVING 1


#define RESPAWN_INFO_DONT_RESPAWN 0xFF

///attack handlers
#define ATTACK_HANDLER_NOP 0
#define ATTACK_HANDLER_DIE_IF_HEALTH_NON_POSITIVE 1
#define ATTACK_HANDLER_KNOCKBACK 2
#define ATTACK_HANDLER_SQUISHED 3
#define ATTACK_HANDLER_SPECIAL_KOOPA_LOSE_SHELL 4
#define ATTACK_HANDLER_SET_SPEED_TO_ZERO 5
#define ATTACK_HANDLER_SPECIAL_WIGGLER_JUMPED_ON 6
#define ATTACK_HANDLER_SPECIAL_HUGE_GOOMBA_WEAKLY_ATTACKED 7
#define ATTACK_HANDLER_SQUISHED_WITH_BLUE_COIN 8


#define GOOMBA_ACT_WALK           0
#define GOOMBA_ACT_ATTACKED_MARIO 1
#define GOOMBA_ACT_JUMP           2

#define GOOMBA_SIZE_REGULAR                 0
#define GOOMBA_SIZE_HUGE                    1
#define GOOMBA_SIZE_TINY                    2
#define GOOMBA_BP_SIZE_MASK                 (GOOMBA_SIZE_REGULAR | GOOMBA_SIZE_HUGE | GOOMBA_SIZE_TINY)
#define GOOMBA_BP_TRIPLET_RESPAWN_FLAG_MASK (0x000000FF & ~GOOMBA_BP_SIZE_MASK)

#define GOOMBA_TRIPLET_SPAWNER_ACT_UNLOADED 0
#define GOOMBA_TRIPLET_SPAWNER_ACT_LOADED   1


#define GOOMBA_TRIPLET_SPAWNER_BP_EXTRA_GOOMBAS_MASK (0x000000FF & ~GOOMBA_BP_SIZE_MASK)
#define GOOMBA_TRIPLET_SPAWNER_BP_EXTRA_GOOMBAS(num) ((num) << 2)

#endif // OBJECT_CONSTANTS_MIN_H