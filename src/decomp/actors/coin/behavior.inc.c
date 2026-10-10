// coin.inc.c
#include "../../game/behavior_script.h"

static struct ObjectHitbox sYellowCoinHitbox = {
    /* interactType:      */ INTERACT_COIN,
    /* downOffset:        */ 0,
    /* damageOrCoinValue: */ 1,
    /* health:            */ 0,
    /* numLootCoins:      */ 0,
    /* radius:            */ 100,
    /* height:            */ 64,
    /* hurtboxRadius:     */ 0,
    /* hurtboxHeight:     */ 0,
};

uint32_t bhv_coin_sparkles_init(void) {
    if (o->oInteractStatus & INT_STATUS_INTERACTED
        && !(o->oInteractStatus & INT_STATUS_TOUCHED_BOB_OMB)) {
        //spawn_object(o, MODEL_SPARKLES, bhvGoldenCoinSparkles);
        obj_mark_for_deletion(o);
    }

    o->oInteractStatus = 0;
}

void bhv_yellow_coin_init(void) {
    //cur_obj_set_behavior(bhvYellowCoin);
    obj_set_hitbox(o, &sYellowCoinHitbox);
    bhv_init_room();
    cur_obj_update_floor_height();

    if (500.0f < absf(o->oPosY - o->oFloorHeight)) {
        //cur_obj_set_model(MODEL_YELLOW_COIN_NO_SHADOW);
    }

    if (o->oFloorHeight < FLOOR_LOWER_LIMIT_MISC) {
        obj_mark_for_deletion(o);
    }
}

void bhv_yellow_coin_loop(void) {
    bhv_coin_sparkles_init();
    o->oAnimState++;
}

void bhv_temp_coin_loop(void) {
    o->oAnimState++;

    if (cur_obj_wait_then_blink(200, 20)) {
        obj_mark_for_deletion(o);
    }

    bhv_coin_sparkles_init();
}

void bhv_spawned_coin_init(void) {
    o->oVelY = random_float() * 10.0f + 30.0f + o->oCoinBaseVelY;
    o->oForwardVel = random_float() * 10.0f;
    o->oMoveAngleYaw = random_u16();

    //cur_obj_set_behavior(bhvYellowCoin);
    obj_set_hitbox(o, &sYellowCoinHitbox);
    cur_obj_become_intangible();
}

void bhv_spawned_coin_loop(void) {
    struct SM64SurfaceCollisionData *floor;

    cur_obj_update_floor_and_walls();
    cur_obj_if_hit_wall_bounce_away();
    cur_obj_move_standard(-62);

    if ((floor = o->oFloor) != NULL) {
        if (o->oMoveFlags & OBJ_MOVE_ON_GROUND) {
            o->oSubAction = 1;
        }
        if (o->oSubAction == 1) {
            o->oBounciness = 0;
            if (floor->normal.y < 0.9) {
                s16 sp1A = atan2s(floor->normal.z, floor->normal.x);
                cur_obj_rotate_yaw_toward(sp1A, 0x400);
            }
        }
    }

    if (o->oTimer == 0) {
        cur_obj_play_sound_2(SOUND_GENERAL_COIN_SPURT);
    }

    if (o->oVelY < 0.0f) {
        cur_obj_become_tangible();
    }

    if (o->oMoveFlags & OBJ_MOVE_LANDED) {
#ifndef VERSION_JP
        if (o->oMoveFlags & (OBJ_MOVE_ABOVE_DEATH_BARRIER | OBJ_MOVE_ABOVE_LAVA))
#else
        if (o->oMoveFlags & OBJ_MOVE_ABOVE_LAVA)
#endif
        {
            obj_mark_for_deletion(o);
        }
    }

    if (o->oMoveFlags & OBJ_MOVE_BOUNCE) {
#ifndef VERSION_JP
        if (o->oCoinNumBounceSoundPlayed < 5) {
#endif
            cur_obj_play_sound_2(SOUND_GENERAL_COIN_DROP);
#ifndef VERSION_JP
        }
        o->oCoinNumBounceSoundPlayed++;
#endif
    }

    if (cur_obj_wait_then_blink(400, 20)) {
        obj_mark_for_deletion(o);
    }

    bhv_coin_sparkles_init();
}
/*
void bhv_coin_formation_spawn_loop(void) {
    if (o->oTimer == 0) {
        //cur_obj_set_behavior(bhvYellowCoin);
        obj_set_hitbox(o, &sYellowCoinHitbox);
        bhv_init_room();

        if (o->oCoinOnGround) {
            o->oPosY += 300.0f;
            cur_obj_update_floor_height();

            if (o->oPosY < o->oFloorHeight || o->oFloorHeight < FLOOR_LOWER_LIMIT_MISC) {
                obj_mark_for_deletion(o);
            } else {
                o->oPosY = o->oFloorHeight;
            }
        } else {
            cur_obj_update_floor_height();

            if (absf(o->oPosY - o->oFloorHeight) > 250.0f) {
                //cur_obj_set_model(MODEL_YELLOW_COIN_NO_SHADOW);
            }
        }
    } else {
        if (bhv_coin_sparkles_init()) {
            o->parentObj->oCoinCollectedFlags |= bit_shift_left(o->oBhvParams2ndByte);
        }
        o->oAnimState++;
    }

    if (o->parentObj->oAction == COIN_FORMATION_ACT_RESPAWN_COINS) {
        obj_mark_for_deletion(o);
    }
}
*/
static s16 sCoinArrowPositions[][2] = {
    { 0, -150 },
    { 0, -50 },
    { 0, 50 },
    { 0, 150 },
    { -50, 100 },
    { -100, 50 },
    { 50, 100 },
    { 100, 50 },
};
/*
void spawn_coin_in_formation(s32 coinIndex, s32 coinFormationFlags) {
    
    struct Object *coinSpawner;
    Vec3i pos;
    s32 setSpawner = TRUE;
    s32 onGround = TRUE;

    pos[0] = pos[1] = pos[2] = 0;

    switch (coinFormationFlags & COIN_FORMATION_BP_FLAG_MASK) {
        case COIN_FORMATION_BP_LINE_HORIZONTAL:
            pos[2] = 160 * (coinIndex - 2);
            if (coinIndex > 4) {
                setSpawner = FALSE;
            }
            break;

        case COIN_FORMATION_BP_LINE_VERTICAL:
            onGround = FALSE;
            pos[1] = 160 * coinIndex * 0.8; // 128 * coinIndex
            if (coinIndex > 4) {
                setSpawner = FALSE;
            }
            break;

        case COIN_FORMATION_BP_RING_HORIZONTAL:
            pos[0] = sins(coinIndex << 13) * 300.0f;
            pos[2] = coss(coinIndex << 13) * 300.0f;
            break;

        case COIN_FORMATION_BP_RING_VERTICAL:
            onGround = FALSE;
            pos[0] = coss(coinIndex << 13) * 200.0f;
            pos[1] = sins(coinIndex << 13) * 200.0f + 200.0f;
            break;

        case COIN_FORMATION_BP_ARROW:
            pos[0] = sCoinArrowPositions[coinIndex][0];
            pos[2] = sCoinArrowPositions[coinIndex][1];
            break;
    }

    if (coinFormationFlags & COIN_FORMATION_BP_FLAG_FLYING) {
        onGround = FALSE;
    }

    if (setSpawner) {
        coinSpawner = spawn_object_relative(coinIndex, pos[0], pos[1], pos[2], o,
                                            MODEL_YELLOW_COIN, bhvCoinFormationSpawn);
        coinSpawner->oCoinOnGround = onGround;
    }
    
}

void bhv_coin_formation_init(void) {
    o->oCoinCollectedFlags = (o->oBhvParams >> 8) & 0xFF;
}

void bhv_coin_formation_loop(void) {
    s32 coinIndex;

    switch (o->oAction) {
        case COIN_FORMATION_ACT_SPAWN_COINS:
            if (o->oDistanceToMario < 2000.0f) {
                for (coinIndex = 0; coinIndex <= 7; coinIndex++) {
                    if (!(o->oCoinCollectedFlags & (1 << coinIndex))) {
                        spawn_coin_in_formation(coinIndex, o->oBhvParams2ndByte);
                    }
                }
                o->oAction++;
            }
            break;

        case COIN_FORMATION_ACT_IDLE:
            if (o->oDistanceToMario > 2100.0f) {
                o->oAction++;
            }
            break;

        case COIN_FORMATION_ACT_RESPAWN_COINS:
            o->oAction = COIN_FORMATION_ACT_SPAWN_COINS;
            break;
    }

    set_object_respawn_info_bits(o, o->oCoinCollectedFlags & 0xFF);
}
*/
void coin_inside_boo_act_1(void) {
    cur_obj_update_floor_and_walls();
    cur_obj_if_hit_wall_bounce_away();

    if (o->oMoveFlags & OBJ_MOVE_BOUNCE) {
        cur_obj_play_sound_2(SOUND_GENERAL_COIN_DROP);
    }

    if (o->oTimer > 90 || (o->oMoveFlags & OBJ_MOVE_LANDED)) {
        obj_set_hitbox(o, &sYellowCoinHitbox);
        cur_obj_become_tangible();
        //cur_obj_set_behavior(bhvYellowCoin);
    }

    cur_obj_move_standard(-30);
    bhv_coin_sparkles_init();

    //if (cur_obj_has_model(MODEL_BLUE_COIN)) {
    //    o->oDamageOrCoinValue = 5;
    //}

    if (cur_obj_wait_then_blink(400, 20)) {
        obj_mark_for_deletion(o);
    }
}

void coin_inside_boo_act_0(void) {
    /*
    s16 sp26;
    f32 sp20;
    struct Object *parent = o->parentObj;

    cur_obj_become_intangible();

    if (o->oTimer == 0 && gCurrLevelNum == LEVEL_BBH) {
        cur_obj_set_model(MODEL_BLUE_COIN);
        cur_obj_scale(0.7f);
    }

    obj_copy_pos(o, parent);

    if (parent->oBooDeathStatus == BOO_DEATH_STATUS_DYING) {
        o->oAction = 1;
        sp26 = gMarioObject->oMoveAngleYaw;
        sp20 = 3.0f;
        o->oVelX = sins(sp26) * sp20;
        o->oVelZ = coss(sp26) * sp20;
        o->oVelY = 35.0f;
    }
    */
}

void (*sCoinInsideBooActions[])(void) = {
    coin_inside_boo_act_0,
    coin_inside_boo_act_1,
};

void bhv_coin_inside_boo_loop(void) {
    //cur_obj_call_action_function(sCoinInsideBooActions);
}

void bhv_coin_sparkles_loop(void) {
    cur_obj_scale(0.6f);
}

void bhv_golden_coin_sparkles_loop(void) {
    /*
    struct Object *sparkles;
    UNUSED u8 filler[4];
    f32 sp24 = 30.0f;

    sparkles = spawn_object(o, MODEL_SPARKLES, bhvCoinSparkles);
    sparkles->oPosX += random_float() * sp24 - sp24 / 2;
    sparkles->oPosZ += random_float() * sp24 - sp24 / 2;
    */
}






// moving_coin.inc.c

static struct ObjectHitbox sMovingYellowCoinHitbox = {
    /* interactType:      */ INTERACT_COIN,
    /* downOffset:        */ 0,
    /* damageOrCoinValue: */ 1,
    /* health:            */ 0,
    /* numLootCoins:      */ 0,
    /* radius:            */ 100,
    /* height:            */ 64,
    /* hurtboxRadius:     */ 0,
    /* hurtboxHeight:     */ 0,
};

static struct ObjectHitbox sMovingBlueCoinHitbox = {
    /* interactType:      */ INTERACT_COIN,
    /* downOffset:        */ 0,
    /* damageOrCoinValue: */ 5,
    /* health:            */ 0,
    /* numLootCoins:      */ 0,
    /* radius:            */ 100,
    /* height:            */ 64,
    /* hurtboxRadius:     */ 0,
    /* hurtboxHeight:     */ 0,
};

s32 coin_step(s16 *collisionFlagsPtr) {
    /*
    *collisionFlagsPtr = object_step();

    obj_check_floor_death(*collisionFlagsPtr, sObjFloor);

    if ((*collisionFlagsPtr & OBJ_COL_FLAG_GROUNDED)
        && !(*collisionFlagsPtr & OBJ_COL_FLAG_NO_Y_VEL)) {
        cur_obj_play_sound_2(SOUND_GENERAL_COIN_DROP);
        return TRUE;
    }

    return FALSE;
    */
}

void moving_coin_flicker(void) {
    s16 collisionFlags;

    coin_step(&collisionFlags);
    obj_flicker_and_disappear(o, 0);
}

void coin_collected(void) {
    //spawn_object(o, MODEL_SPARKLES, bhvGoldenCoinSparkles);
    o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
}

void bhv_moving_yellow_coin_init(void) {
    o->oGravity = 3.0f;
    o->oFriction = 1.0f;
    o->oBuoyancy = 1.5f;

    obj_set_hitbox(o, &sMovingYellowCoinHitbox);
}

void bhv_moving_yellow_coin_loop(void) {
    s16 collisionFlags;

    switch (o->oAction) {
        case MOV_YCOIN_ACT_IDLE:
            coin_step(&collisionFlags);

            if (o->oTimer < 10) {
                cur_obj_become_intangible();
            } else {
                cur_obj_become_tangible();
            }

            if (o->oTimer > 300) {
                o->oAction = MOV_YCOIN_ACT_BLINKING;
            }
            break;

        case MOV_YCOIN_ACT_BLINKING:
            moving_coin_flicker();
            break;

        case MOV_YCOIN_ACT_LAVA_DEATH:
            o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
            break;

        case MOV_YCOIN_ACT_DEATH_PLANE_DEATH:
            o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
            break;
    }

    if (o->oInteractStatus & INT_STATUS_INTERACTED) {
        coin_collected();
        o->oInteractStatus = 0;
    }
}

void bhv_moving_blue_coin_init(void) {
    o->oGravity = 5.0f;
    o->oFriction = 1.0f;
    o->oBuoyancy = 1.5f;

    obj_set_hitbox(o, &sMovingBlueCoinHitbox);
}

void bhv_moving_blue_coin_loop(void) {
    s16 collisionFlags;

    switch (o->oAction) {
        case MOV_BCOIN_ACT_STILL:
            if (is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 1500)) {
                o->oAction = MOV_BCOIN_ACT_MOVING;
            }
            break;

        case MOV_BCOIN_ACT_MOVING:
            collisionFlags = object_step();

            if (collisionFlags & OBJ_COL_FLAG_GROUNDED) {
                o->oForwardVel += 25.0f;
                if (!(collisionFlags & OBJ_COL_FLAG_NO_Y_VEL)) {
                    cur_obj_play_sound_2(SOUND_GENERAL_COIN_DROP);
                }
            } else {
                o->oForwardVel *= 0.98;
            }

            if (o->oForwardVel > 75.0) {
                o->oForwardVel = 75.0f;
            }

            obj_flicker_and_disappear(o, 600);
            break;
    }

    if (o->oInteractStatus & INT_STATUS_INTERACTED) {
        coin_collected();
        o->oInteractStatus = 0;
    }
}

void bhv_blue_coin_sliding_jumping_init(void) {
    o->oGravity = 3.0;
    o->oFriction = 0.98;
    o->oBuoyancy = 1.5;

    obj_set_hitbox(o, &sMovingBlueCoinHitbox);
}

void blue_coin_sliding_away_from_mario(void) {
    s16 collisionFlags;

    o->oForwardVel = 15.0;
    o->oMoveAngleYaw = o->oAngleToMario + 0x8000;

    if (coin_step(&collisionFlags)) {
        o->oVelY += 18.0f;
    }

    if (collisionFlags & OBJ_COL_FLAG_HIT_WALL) {
        o->oAction = 3;
    }

    if (!is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 1000)) {
        o->oAction = 2;
    }
}

void blue_coin_sliding_slow_down(void) {
    s16 collisionFlags;

    coin_step(&collisionFlags);

    if (is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 500) == TRUE) {
        o->oAction = 1;
    }

    if (o->oTimer > 150) {
        o->oAction = 3;
    }
}

void bhv_blue_coin_sliding_loop(void) {
    s16 collisionFlags;

    switch (o->oAction) {
        case 0:
            if (is_point_within_radius_of_mario(o->oPosX, o->oPosY, o->oPosZ, 500) == TRUE) {
                o->oAction = 1;
            }

            set_object_visibility(o, 3000);
            break;

        case 1:
            blue_coin_sliding_away_from_mario();
            break;

        case 2:
            blue_coin_sliding_slow_down();
            set_object_visibility(o, 3000);
            break;

        case 3:
            coin_step(&collisionFlags);
            if (o->oTimer > 60) {
                o->oAction = 4;
            }
            break;

        case 4:
            moving_coin_flicker();
            break;

        case 100:
            o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
            break;

        case 101:
            o->activeFlags = ACTIVE_FLAG_DEACTIVATED;
            break;
    }

    if (o->oInteractStatus & INT_STATUS_INTERACTED) {
        coin_collected();
        o->oInteractStatus = 0;
    }
}

void bhv_blue_coin_jumping_loop(void) {
    s16 collisionFlags;

    switch (o->oAction) {
        case 0:
            if (o->oTimer == 0) {
                cur_obj_become_intangible();
                o->oVelY = 50.0;
            }

            object_step();

            if (o->oTimer == 15) {
                cur_obj_become_tangible();
                o->oAction = 1;
            }
            break;

        case 1:
            blue_coin_sliding_away_from_mario();
            break;

        case 2:
            blue_coin_sliding_slow_down();
            set_object_visibility(o, 3000);
            break;

        case 3:
            coin_step(&collisionFlags);
            if (o->oTimer > 60) {
                o->oAction = 4;
            }
            break;

        case 4:
            moving_coin_flicker();
            break;
    }

    if (o->oInteractStatus & INT_STATUS_INTERACTED) {
        coin_collected();
        o->oInteractStatus = 0;
    }
}
