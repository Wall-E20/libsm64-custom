#include <PR/ultratypes.h>
#include "sm64.h"
#include "decomp/shim.h"
#include "decomp/global_state.h"
#include "decomp/include/object_fields.h"
#include "object_constants_min.h"
#include "object_helpers_min.h"
#include "decomp/game/behavior_actions.h"
#include "decomp/game/camera.h"
#include "decomp/game/behavior_script.h"
#include "decomp/engine/geo_layout.h"
#include "decomp/engine/math_util.h"
#include "decomp/engine/surface_collision.h"
#include "decomp/game/object_stuff.h"
#include "decomp/game/rendering_graph_node.h"
#include "decomp/game/interaction.h"
#include "decomp/game/mario.h"
#include "decomp/game/mario_misc.h"
#include "memory.h"
#include "decomp/game/platform_displacement.h"
#include "decomp/game/save_file.h"
#include "decomp/include/seq_ids.h"
#include "play_sound.h"
#include "actorMgr.h"

static Vec3f gVec3fZero = { 0.0f, 0.0f, 0.0f };
static Vec3s gVec3sZero = { 0, 0, 0 };
static Vec3f gVec3fOne = { 1.0f, 1.0f, 1.0f };

#define o (gCurrentObject)


static s32 cur_obj_move_xz(f32 steepSlopeNormalY, s32 careAboutEdgesAndSteepSlopes);
static f32 cur_obj_move_y_and_get_water_level(f32 gravity, f32 buoyancy);

void spawn_mist_particles(void);
void spawn_mist_particles_with_sound(u32 soundMagic);


f32 absf(f32 x) {
    if (x >= 0) {
        return x;
    } else {
        return -x;
    }
}

void obj_set_speed_to_zero(void) {
    o->oForwardVel = o->oVelY = 0.0f;
}

void shelled_koopa_attack_handler(UNUSED s32 attackType) {
}

void wiggler_jumped_on_attack_handler(void) {
}

void cur_obj_extend_animation_if_at_end(void) {
    s32 animFrame = o->header.gfx.animInfo.animFrame;
    s32 nearLoopEnd = o->header.gfx.animInfo.curAnim->loopEnd - 2;

    if (animFrame == nearLoopEnd) {
        o->header.gfx.animInfo.animFrame--;
    }
}


s32 random_sign(void) {
    if (random_u16() >= 0x7FFF) {
        return 1;
    } else {
        return -1;
    }
}



/**
 * Orients an object with the given normals, typically the surface under the object.
 */
void obj_orient_graph(struct Object *obj, f32 normalX, f32 normalY, f32 normalZ) {
    Vec3f objVisualPosition, surfaceNormals;

    Mat4 *throwMatrix;

    // Passes on orienting certain objects that shouldn't be oriented, like boulders.
    if (!sOrientObjWithFloor) {
        return;
    }

    // Passes on orienting billboard objects, i.e. coins, trees, etc.
    if (obj->header.gfx.node.flags & GRAPH_RENDER_BILLBOARD) {
        return;
    }

    throwMatrix = alloc_display_list(sizeof(*throwMatrix));
    // If out of memory, fail to try orienting the object.
    if (throwMatrix == NULL) {
        return;
    }

    objVisualPosition[0] = obj->oPosX;
    objVisualPosition[1] = obj->oPosY + obj->oGraphYOffset;
    objVisualPosition[2] = obj->oPosZ;

    surfaceNormals[0] = normalX;
    surfaceNormals[1] = normalY;
    surfaceNormals[2] = normalZ;

    mtxf_align_terrain_normal(*throwMatrix, surfaceNormals, objVisualPosition, obj->oFaceAngleYaw);
    obj->header.gfx.throwMatrix = throwMatrix;
}

/**
 * Determines an object's forward speed multiplier.
 */
void calc_obj_friction(f32 *objFriction, f32 floor_nY) {
    if (floor_nY < 0.2 && o->oFriction < 0.9999) {
        *objFriction = 0;
    } else {
        *objFriction = o->oFriction;
    }
}


/**
 * Updates an objects speed for gravity and updates Y position.
 */
void calc_new_obj_vel_and_pos_y(struct SM64SurfaceCollisionData *objFloor, f32 objFloorY, f32 objVelX, f32 objVelZ) {
    f32 floor_nX = objFloor->normal.x;
    f32 floor_nY = objFloor->normal.y;
    f32 floor_nZ = objFloor->normal.z;
    f32 objFriction;

    // Caps vertical speed with a "terminal velocity".
    o->oVelY -= o->oGravity;
    if (o->oVelY > 75.0) {
        o->oVelY = 75.0;
    }
    if (o->oVelY < -75.0) {
        o->oVelY = -75.0;
    }

    o->oPosY += o->oVelY;

    //Snap the object up to the floor.
    if (o->oPosY < objFloorY) {
        o->oPosY = objFloorY;

        // Bounces an object if the ground is hit fast enough.
        if (o->oVelY < -17.5) {
            o->oVelY = -(o->oVelY / 2);
        } else {
            o->oVelY = 0;
        }
    }

    //! (Obj Position Crash) If you got an object with height past 2^31, the game would crash.
    if ((s32) o->oPosY >= (s32) objFloorY && (s32) o->oPosY < (s32) objFloorY + 37) {
        obj_orient_graph(o, floor_nX, floor_nY, floor_nZ);

        // Adds horizontal component of gravity for horizontal speed.
        objVelX += floor_nX * (floor_nX * floor_nX + floor_nZ * floor_nZ)
                   / (floor_nX * floor_nX + floor_nY * floor_nY + floor_nZ * floor_nZ) * o->oGravity
                   * 2;
        objVelZ += floor_nZ * (floor_nX * floor_nX + floor_nZ * floor_nZ)
                   / (floor_nX * floor_nX + floor_nY * floor_nY + floor_nZ * floor_nZ) * o->oGravity
                   * 2;

        if (objVelX < 0.000001 && objVelX > -0.000001) {
            objVelX = 0;
        }
        if (objVelZ < 0.000001 && objVelZ > -0.000001) {
            objVelZ = 0;
        }

        if (objVelX != 0 || objVelZ != 0) {
            o->oMoveAngleYaw = atan2s(objVelZ, objVelX);
        }

        calc_obj_friction(&objFriction, floor_nY);
        o->oForwardVel = sqrtf(objVelX * objVelX + objVelZ * objVelZ) * objFriction;
    }
}

void calc_new_obj_vel_and_pos_y_underwater(struct SM64SurfaceCollisionData *objFloor, f32 floorY, f32 objVelX, f32 objVelZ,
                                    f32 waterY) {
    f32 floor_nX = objFloor->normal.x;
    f32 floor_nY = objFloor->normal.y;
    f32 floor_nZ = objFloor->normal.z;

    f32 netYAccel = (1.0f - o->oBuoyancy) * (-1.0f * o->oGravity);
    o->oVelY -= netYAccel;

    // Caps vertical speed with a "terminal velocity".
    if (o->oVelY > 75.0) {
        o->oVelY = 75.0;
    }
    if (o->oVelY < -75.0) {
        o->oVelY = -75.0;
    }

    o->oPosY += o->oVelY;

    //Snap the object up to the floor.
    if (o->oPosY < floorY) {
        o->oPosY = floorY;

        // Bounces an object if the ground is hit fast enough.
        if (o->oVelY < -17.5) {
            o->oVelY = -(o->oVelY / 2);
        } else {
            o->oVelY = 0;
        }
    }

    // If moving fast near the surface of the water, flip vertical speed? To emulate skipping?
    if (o->oForwardVel > 12.5 && (waterY + 30.0f) > o->oPosY && (waterY - 30.0f) < o->oPosY) {
        o->oVelY = -o->oVelY;
    }

    if ((s32) o->oPosY >= (s32) floorY && (s32) o->oPosY < (s32) floorY + 37) {
        obj_orient_graph(o, floor_nX, floor_nY, floor_nZ);

        // Adds horizontal component of gravity for horizontal speed.
        objVelX += floor_nX * (floor_nX * floor_nX + floor_nZ * floor_nZ)
                   / (floor_nX * floor_nX + floor_nY * floor_nY + floor_nZ * floor_nZ) * netYAccel * 2;
        objVelZ += floor_nZ * (floor_nX * floor_nX + floor_nZ * floor_nZ)
                   / (floor_nX * floor_nX + floor_nY * floor_nY + floor_nZ * floor_nZ) * netYAccel * 2;
    }

    if (objVelX < 0.000001 && objVelX > -0.000001) {
        objVelX = 0;
    }
    if (objVelZ < 0.000001 && objVelZ > -0.000001) {
        objVelZ = 0;
    }

    if (o->oVelY < 0.000001 && o->oVelY > -0.000001) {
        o->oVelY = 0;
    }

    if (objVelX != 0 || objVelZ != 0) {
        o->oMoveAngleYaw = atan2s(objVelZ, objVelX);
    }

    // Decreases both vertical velocity and forward velocity. Likely so that skips above
    // don't loop infinitely.
    o->oForwardVel = sqrtf(objVelX * objVelX + objVelZ * objVelZ) * 0.8;
    o->oVelY *= 0.8;
}

/**
 * Updates an objects position from oForwardVel and oMoveAngleYaw.
 */
void obj_update_pos_vel_xz(void) {
    f32 xVel = o->oForwardVel * sins(o->oMoveAngleYaw);
    f32 zVel = o->oForwardVel * coss(o->oMoveAngleYaw);

    o->oPosX += xVel;
    o->oPosZ += zVel;
}




void turn_obj_away_from_surface(f32 velX, f32 velZ, f32 nX, UNUSED f32 nY, f32 nZ, f32 *objYawX,
                            f32 *objYawZ) {
    *objYawX = (nZ * nZ - nX * nX) * velX / (nX * nX + nZ * nZ)
               - 2 * velZ * (nX * nZ) / (nX * nX + nZ * nZ);

    *objYawZ = (nX * nX - nZ * nZ) * velZ / (nX * nX + nZ * nZ)
               - 2 * velX * (nX * nZ) / (nX * nX + nZ * nZ);
}

/**
 * Finds any wall collisions, applies them, and turns away from the surface.
 */
s8 obj_find_wall(f32 objNewX, f32 objY, f32 objNewZ, f32 objVelX, f32 objVelZ) {
    struct SM64WallCollisionData hitbox;
    f32 wall_nX, wall_nY, wall_nZ, objVelXCopy, objVelZCopy, objYawX, objYawZ;

    hitbox.x = objNewX;
    hitbox.y = objY;
    hitbox.z = objNewZ;
    hitbox.offsetY = o->hitboxHeight / 2;
    hitbox.radius = o->hitboxRadius;

    if (find_wall_collisions(&hitbox) != 0) {
        o->oPosX = hitbox.x;
        o->oPosY = hitbox.y;
        o->oPosZ = hitbox.z;

        wall_nX = hitbox.walls[0]->normal.x;
        wall_nY = hitbox.walls[0]->normal.y;
        wall_nZ = hitbox.walls[0]->normal.z;

        objVelXCopy = objVelX;
        objVelZCopy = objVelZ;

        // Turns away from the first wall only.
        turn_obj_away_from_surface(objVelXCopy, objVelZCopy, wall_nX, wall_nY, wall_nZ, &objYawX, &objYawZ);

        o->oMoveAngleYaw = atan2s(objYawZ, objYawX);
        return FALSE;
    }

    return TRUE;
}


/**
 * Turns an object away from steep floors, similarly to walls.
 */
s8 turn_obj_away_from_steep_floor(struct SM64SurfaceCollisionData *objFloor, f32 floorY, f32 objVelX, f32 objVelZ) {
    f32 floor_nX, floor_nY, floor_nZ, objVelXCopy, objVelZCopy, objYawX, objYawZ;

    if (objFloor == NULL) {
        //! (OOB Object Crash) TRUNC overflow exception after 36 minutes
        o->oMoveAngleYaw += 32767.999200000002; /* ¯\_(ツ)_/¯ */
        return FALSE;
    }

    floor_nX = objFloor->normal.x;
    floor_nY = objFloor->normal.y;
    floor_nZ = objFloor->normal.z;

    // If the floor is steep and we are below it (i.e. walking into it), turn away from the floor.
    if (floor_nY < 0.5 && floorY > o->oPosY) {
        objVelXCopy = objVelX;
        objVelZCopy = objVelZ;
        turn_obj_away_from_surface(objVelXCopy, objVelZCopy, floor_nX, floor_nY, floor_nZ, &objYawX,
                               &objYawZ);
        o->oMoveAngleYaw = atan2s(objYawZ, objYawX);
        return FALSE;
    }

    return TRUE;
}


s16 object_step(void) {
    f32 objX = o->oPosX;
    f32 objY = o->oPosY;
    f32 objZ = o->oPosZ;

    f32 floorY;
    f32 waterY = FLOOR_LOWER_LIMIT_MISC;

    f32 objVelX = o->oForwardVel * sins(o->oMoveAngleYaw);
    f32 objVelZ = o->oForwardVel * coss(o->oMoveAngleYaw);

    s16 collisionFlags = 0;

    // Find any wall collisions, receive the push, and set the flag.
    if (obj_find_wall(objX + objVelX, objY, objZ + objVelZ, objVelX, objVelZ) == 0) {
        collisionFlags += OBJ_COL_FLAG_HIT_WALL;
    }

    floorY = find_floor(objX + objVelX, objY, objZ + objVelZ, &sObjFloor);
    if (turn_obj_away_from_steep_floor(sObjFloor, floorY, objVelX, objVelZ) == 1) {
        waterY = find_water_level(objX + objVelX, objZ + objVelZ);
        if (waterY > objY) {
            calc_new_obj_vel_and_pos_y_underwater(sObjFloor, floorY, objVelX, objVelZ, waterY);
            collisionFlags += OBJ_COL_FLAG_UNDERWATER;
        } else {
            calc_new_obj_vel_and_pos_y(sObjFloor, floorY, objVelX, objVelZ);
        }
    } else {
        // Treat any awkward floors similar to a wall.
        collisionFlags +=
            ((collisionFlags & OBJ_COL_FLAG_HIT_WALL) ^ OBJ_COL_FLAG_HIT_WALL);
    }

    obj_update_pos_vel_xz();
    if ((s32) o->oPosY == (s32) floorY) {
        collisionFlags += OBJ_COL_FLAG_GROUNDED;
    }

    if ((s32) o->oVelY == 0) {
        collisionFlags += OBJ_COL_FLAG_NO_Y_VEL;
    }

    // Generate a splash if in water.
    //obj_splash((s32) waterY, (s32) o->oPosY);
    return collisionFlags;
}


void set_object_respawn_info_bits(struct Object *obj, u8 bits) {
    u32 *info32;
    u16 *info16;

    switch (obj->respawnInfoType) {
        case RESPAWN_INFO_TYPE_32:
            info32 = (u32 *) obj->respawnInfo;
            *info32 |= bits << 8;
            break;

        case RESPAWN_INFO_TYPE_16:
            info16 = (u16 *) obj->respawnInfo;
            *info16 |= bits << 8;
            break;
    }
}


s16 abs_angle_diff(s16 x0, s16 x1) {
    s16 diff = x1 - x0;

    if (diff == -0x8000) {
        diff = -0x7FFF;
    }

    if (diff < 0) {
        diff = -diff;
    }

    return diff;
}

static void apply_drag_to_value(f32 *value, f32 dragStrength) {
    f32 decel;

    if (*value != 0) {
        //! Can overshoot if |*value| > 1/(dragStrength * 0.0001)
        decel = (*value) * (*value) * (dragStrength * 0.0001L);

        if (*value > 0) {
            *value -= decel;
            if (*value < 0.001L) {
                *value = 0;
            }
        } else {
            *value += decel;
            if (*value > -0.001L) {
                *value = 0;
            }
        }
    }
}

static s32 approach_f32_ptr(f32 *px, f32 target, f32 delta) {
    if (*px > target) {
        delta = -delta;
    }

    *px += delta;

    if ((*px - target) * delta >= 0) {
        *px = target;
        return TRUE;
    }
    return FALSE;
}

s16 approach_s16_symmetric(s16 value, s16 target, s16 increment) {
    s16 dist = target - value;

    if (dist >= 0) {
        if (dist > increment) {
            value += increment;
        } else {
            value = target;
        }
    } else {
        if (dist < -increment) {
            value -= increment;
        } else {
            value = target;
        }
    }

    return value;
}

static s32 clear_move_flag(u32 *bitSet, s32 flag) {
    if (*bitSet & flag) {
        *bitSet &= flag ^ 0xFFFFFFFF;
        return TRUE;
    } else {
        return FALSE;
    }
}

Gfx *geo_update_projectile_pos_from_parent(s32 callContext, UNUSED struct GraphNode *node, Mat4 mtx) {
    if (callContext == GEO_CONTEXT_RENDER) {
        Mat4 sp20;
        struct Object *obj = (struct Object *) gCurGraphNodeObject; // TODO: change global type to Object pointer
        if (obj->prevObj) {
            create_transformation_from_matrices(sp20, mtx, *gCurGraphNodeCamera->matrixPtr);
            obj_update_pos_from_parent_transformation(sp20, obj->prevObj);
            obj_set_gfx_pos_from_pos(obj->prevObj);
        }
    }

    return NULL;
}


void cur_obj_apply_drag_xz(f32 dragStrength) {
    apply_drag_to_value(&o->oVelX, dragStrength);
    apply_drag_to_value(&o->oVelZ, dragStrength);
}

void cur_obj_become_intangible(void) {
    // When the timer is negative, the object is intangible and the timer
    // doesn't count down
    o->oIntangibleTimer = -1;
}

void cur_obj_become_tangible(void) {
    o->oIntangibleTimer = 0;
}

f32 dist_between_objects(struct Object *obj1, struct Object *obj2) {
    f32 dx = obj1->oPosX - obj2->oPosX;
    f32 dy = obj1->oPosY - obj2->oPosY;
    f32 dz = obj1->oPosZ - obj2->oPosZ;

    return sqrtf(dx * dx + dy * dy + dz * dz);
}

s32 cur_obj_check_anim_frame_in_range(s32 startFrame, s32 rangeLength) {
    s32 animFrame = o->header.gfx.animInfo.animFrame;

    if (animFrame >= startFrame && animFrame < startFrame + rangeLength) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void cur_obj_compute_vel_xz(void) {
    o->oVelX = o->oForwardVel * sins(o->oMoveAngleYaw);
    o->oVelZ = o->oForwardVel * coss(o->oMoveAngleYaw);
}

static s32 cur_obj_detect_steep_floor(s16 steepAngleDegrees) {
    struct SM64SurfaceCollisionData *intendedFloor;
    f32 intendedX, intendedFloorHeight, intendedZ;
    f32 deltaFloorHeight;
    f32 steepNormalY = coss((s16)(steepAngleDegrees * (0x10000 / 360)));

    if (o->oForwardVel != 0.0f) {
        intendedX = o->oPosX + o->oVelX;
        intendedZ = o->oPosZ + o->oVelZ;
        intendedFloorHeight = find_floor(intendedX, o->oPosY, intendedZ, &intendedFloor);
        deltaFloorHeight = intendedFloorHeight - o->oFloorHeight;

        if (intendedFloorHeight < FLOOR_LOWER_LIMIT_MISC) {
            o->oWallAngle = o->oMoveAngleYaw + 0x8000;
            return 2;
        } else if (intendedFloor->normal.y < steepNormalY && deltaFloorHeight > 0
                   && intendedFloorHeight > o->oPosY) {
            o->oWallAngle = atan2s(intendedFloor->normal.z, intendedFloor->normal.x);
            return 1;
        } else {
            return 0;
        }
    }

    return 0;
}
void cur_obj_update_floor_height(void) {
    struct SM64SurfaceCollisionData *floor;
    o->oFloorHeight = find_floor(o->oPosX, o->oPosY, o->oPosZ, &floor);
}
void cur_obj_if_hit_wall_bounce_away(void) {
    if (o->oMoveFlags & OBJ_MOVE_HIT_WALL) {
        o->oMoveAngleYaw = o->oWallAngle;
    }
}

void cur_obj_hide(void) {
    o->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
}

void cur_obj_init_animation_with_accel_and_sound(s32 animIndex, f32 accel) {
    struct Animation **anims = o->oAnimations;
    s32 animAccel = (s32)(accel * 65536.0f);
    geo_obj_init_animation_accel(&o->header.gfx, &anims[animIndex], animAccel);
    o->oSoundStateID = animIndex;
}

void cur_obj_move_standard(s16 steepSlopeAngleDegrees) {
    f32 gravity = o->oGravity;
    f32 bounciness = o->oBounciness;
    f32 buoyancy = o->oBuoyancy;
    f32 dragStrength = o->oDragStrength;
    f32 steepSlopeNormalY;
    s32 careAboutEdgesAndSteepSlopes = FALSE;
    s32 negativeSpeed = FALSE;

    //! Because some objects allow these active flags to be set but don't
    //  avoid updating when they are, we end up with "partial" updates, where
    //  an object's internal state will be updated, but it doesn't move.
    //  This allows numerous glitches and is typically referred to as
    //  deactivation (though this term has a different meaning in the code).
    //  Objects that do this will be marked with //PARTIAL_UPDATE.
    if (!(o->activeFlags & (ACTIVE_FLAG_FAR_AWAY | ACTIVE_FLAG_IN_DIFFERENT_ROOM))) {
        if (steepSlopeAngleDegrees < 0) {
            // clang-format off
            careAboutEdgesAndSteepSlopes = TRUE; steepSlopeAngleDegrees = -steepSlopeAngleDegrees;
            // clang-format on
        }

        steepSlopeNormalY = coss(steepSlopeAngleDegrees * (0x10000 / 360));

        cur_obj_compute_vel_xz();
        cur_obj_apply_drag_xz(dragStrength);

        cur_obj_move_xz(steepSlopeNormalY, careAboutEdgesAndSteepSlopes);
        cur_obj_move_y(gravity, bounciness, buoyancy);

        if (o->oForwardVel < 0.0f) {
            negativeSpeed = TRUE;
        }
        o->oForwardVel = sqrtf(sqr(o->oVelX) + sqr(o->oVelZ));
        if (negativeSpeed == TRUE) {
            o->oForwardVel = -o->oForwardVel;
        }
    }
}

static void cur_obj_move_update_ground_air_flags(UNUSED f32 gravity, f32 bounciness) {
    o->oMoveFlags &= ~OBJ_MOVE_BOUNCE;

    if (o->oPosY < o->oFloorHeight) {
        // On the first frame that we touch the ground, set OBJ_MOVE_LANDED.
        // On subsequent frames, set OBJ_MOVE_ON_GROUND
        if (!(o->oMoveFlags & OBJ_MOVE_ON_GROUND)) {
            if (clear_move_flag(&o->oMoveFlags, OBJ_MOVE_LANDED)) {
                o->oMoveFlags |= OBJ_MOVE_ON_GROUND;
            } else {
                o->oMoveFlags |= OBJ_MOVE_LANDED;
            }
        }

        o->oPosY = o->oFloorHeight;

        if (o->oVelY < 0.0f) {
            o->oVelY *= bounciness;
        }

        if (o->oVelY > 5.0f) {
            //! This overestimates since velY could be > 5 here
            //! without bounce (e.g. jump into misa).
            o->oMoveFlags |= OBJ_MOVE_BOUNCE;
        }
    } else {
        o->oMoveFlags &= ~OBJ_MOVE_LANDED;
        if (clear_move_flag(&o->oMoveFlags, OBJ_MOVE_ON_GROUND)) {
            o->oMoveFlags |= OBJ_MOVE_LEFT_GROUND;
        }
    }

    o->oMoveFlags &= ~OBJ_MOVE_MASK_IN_WATER;
}

static void cur_obj_move_update_underwater_flags(void) {
    f32 decelY = (f32)(sqrtf(o->oVelY * o->oVelY) * (o->oDragStrength * 7.0f)) / 100.0L;

    if (o->oVelY > 0) {
        o->oVelY -= decelY;
    } else {
        o->oVelY += decelY;
    }

    if (o->oPosY < o->oFloorHeight) {
        o->oPosY = o->oFloorHeight;
        o->oMoveFlags |= OBJ_MOVE_UNDERWATER_ON_GROUND;
    } else {
        o->oMoveFlags |= OBJ_MOVE_UNDERWATER_OFF_GROUND;
    }
}

static s32 cur_obj_move_xz(f32 steepSlopeNormalY, s32 careAboutEdgesAndSteepSlopes) {
    struct SM64SurfaceCollisionData *intendedFloor;

    f32 intendedX = o->oPosX + o->oVelX;
    f32 intendedZ = o->oPosZ + o->oVelZ;

    f32 intendedFloorHeight = find_floor(intendedX, o->oPosY, intendedZ, &intendedFloor);
    f32 deltaFloorHeight = intendedFloorHeight - o->oFloorHeight;

    UNUSED u8 filler[4];
    UNUSED f32 ny;

    o->oMoveFlags &= ~OBJ_MOVE_HIT_EDGE;

    if (o->oRoom != -1 && intendedFloor != NULL) {
        if (intendedFloor->room != 0 && o->oRoom != intendedFloor->room && intendedFloor->room != 18) {
            // Don't leave native room
            return FALSE;
        }
    }

    if (intendedFloorHeight < FLOOR_LOWER_LIMIT_MISC) {
        // Don't move into OoB
        o->oMoveFlags |= OBJ_MOVE_HIT_EDGE;
        return FALSE;
    } else if (deltaFloorHeight < 5.0f) {
        if (!careAboutEdgesAndSteepSlopes) {
            // If we don't care about edges or steep slopes, okay to move
            o->oPosX = intendedX;
            o->oPosZ = intendedZ;
            return TRUE;
        } else if (deltaFloorHeight < -50.0f && (o->oMoveFlags & OBJ_MOVE_ON_GROUND)) {
            // Don't walk off an edge
            o->oMoveFlags |= OBJ_MOVE_HIT_EDGE;
            return FALSE;
        } else if (intendedFloor->normal.y > steepSlopeNormalY) {
            // Allow movement onto a slope, provided it's not too steep
            o->oPosX = intendedX;
            o->oPosZ = intendedZ;
            return TRUE;
        } else {
            // We are likely trying to move onto a steep downward slope
            o->oMoveFlags |= OBJ_MOVE_HIT_EDGE;
            return FALSE;
        }
    } else if ((ny = intendedFloor->normal.y) > steepSlopeNormalY || o->oPosY > intendedFloorHeight) {
        // Allow movement upward, provided either:
        // - The target floor is flat enough (e.g. walking up stairs)
        // - We are above the target floor (most likely in the air)
        o->oPosX = intendedX;
        o->oPosZ = intendedZ;
        //! Returning FALSE but moving anyway (not exploitable; return value is
        //  never used)
    }

    // We are likely trying to move onto a steep upward slope
    return FALSE;
}

/*
 * Flag-driven per-frame steps, verbatim from upstream behavior_script.c's
 * cur_obj_update(). Upstream the behaviour script sets these oFlags bits and
 * cur_obj_update() acts on them after running the behaviour; this fork has no
 * behaviour interpreter, so actorMgr calls these directly.
 */

/* obj_set_face_angle_to_move_angle(), object_helpers.c upstream */
void obj_set_face_angle_to_move_angle(struct Object *obj) {
    obj->oFaceAnglePitch = obj->oMoveAnglePitch;
    obj->oFaceAngleYaw = obj->oMoveAngleYaw;
    obj->oFaceAngleRoll = obj->oMoveAngleRoll;
}

/*
 * obj_update_gfx_pos_and_angle(), behavior_script.c upstream.
 *
 * This is what makes an actor visibly turn. Behaviours steer oMoveAngleYaw
 * (cur_obj_rotate_yaw_toward), but rendering reads header.gfx.angle, so without
 * this the model never rotates no matter how correctly the behaviour walks.
 */
void obj_update_gfx_pos_and_angle(struct Object *obj) {
    obj->header.gfx.pos[0] = obj->oPosX;
    obj->header.gfx.pos[1] = obj->oPosY + obj->oGraphYOffset;
    obj->header.gfx.pos[2] = obj->oPosZ;

    obj->header.gfx.angle[0] = obj->oFaceAnglePitch & 0xFFFF;
    obj->header.gfx.angle[1] = obj->oFaceAngleYaw & 0xFFFF;
    obj->header.gfx.angle[2] = obj->oFaceAngleRoll & 0xFFFF;
}

/* cur_obj_move_xz_using_fvel_and_yaw(), object_helpers.c upstream */
void cur_obj_move_xz_using_fvel_and_yaw(void) {
    o->oVelX = o->oForwardVel * sins(o->oMoveAngleYaw);
    o->oVelZ = o->oForwardVel * coss(o->oMoveAngleYaw);

    o->oPosX += o->oVelX;
    o->oPosZ += o->oVelZ;
}

/* cur_obj_move_y_with_terminal_vel(), object_helpers.c upstream */
void cur_obj_move_y_with_terminal_vel(void) {
    if (o->oVelY < -70.0f) {
        o->oVelY = -70.0f;
    }

    o->oPosY += o->oVelY;
}

void cur_obj_move_y(f32 gravity, f32 bounciness, f32 buoyancy) {
    f32 waterLevel;

    o->oMoveFlags &= ~OBJ_MOVE_LEFT_GROUND;

    if (o->oMoveFlags & OBJ_MOVE_AT_WATER_SURFACE) {
        if (o->oVelY > 5.0f) {
            o->oMoveFlags &= ~OBJ_MOVE_MASK_IN_WATER;
            o->oMoveFlags |= OBJ_MOVE_LEAVING_WATER;
        }
    }

    if (!(o->oMoveFlags & OBJ_MOVE_MASK_IN_WATER)) {
        waterLevel = cur_obj_move_y_and_get_water_level(gravity, 0.0f);
        if (o->oPosY > waterLevel) {
            //! We only handle floor collision if the object does not enter
            //  water. This allows e.g. coins to clip through floors if they
            //  enter water on the same frame.
            cur_obj_move_update_ground_air_flags(gravity, bounciness);
        } else {
            o->oMoveFlags |= OBJ_MOVE_ENTERED_WATER;
            o->oMoveFlags &= ~OBJ_MOVE_MASK_ON_GROUND;
        }
    } else {
        o->oMoveFlags &= ~OBJ_MOVE_ENTERED_WATER;

        waterLevel = cur_obj_move_y_and_get_water_level(gravity, buoyancy);
        if (o->oPosY < waterLevel) {
            cur_obj_move_update_underwater_flags();
        } else {
            if (o->oPosY < o->oFloorHeight) {
                o->oPosY = o->oFloorHeight;
                o->oMoveFlags &= ~OBJ_MOVE_MASK_IN_WATER;
            } else {
                o->oPosY = waterLevel;
                o->oVelY = 0.0f;
                o->oMoveFlags &= ~(OBJ_MOVE_UNDERWATER_OFF_GROUND | OBJ_MOVE_UNDERWATER_ON_GROUND);
                o->oMoveFlags |= OBJ_MOVE_AT_WATER_SURFACE;
            }
        }
    }

    if (o->oMoveFlags & (OBJ_MOVE_MASK_ON_GROUND | OBJ_MOVE_AT_WATER_SURFACE
        | OBJ_MOVE_UNDERWATER_OFF_GROUND)) {
        o->oMoveFlags &= ~OBJ_MOVE_IN_AIR;
    } else {
        o->oMoveFlags |= OBJ_MOVE_IN_AIR;
    }
}

static f32 cur_obj_move_y_and_get_water_level(f32 gravity, f32 buoyancy) {
    f32 waterLevel;

    o->oVelY += gravity + buoyancy;
    if (o->oVelY < -78.0f) {
        o->oVelY = -78.0f;
    }

    o->oPosY += o->oVelY;
    if (o->activeFlags & ACTIVE_FLAG_UNK10) {
        waterLevel = FLOOR_LOWER_LIMIT;
    } else {
        waterLevel = find_water_level(o->oPosX, o->oPosZ);
    }

    return waterLevel;
}

s32 cur_obj_play_sound_at_anim_range(s8 arg0, s8 arg1, u32 sound) {
    s32 val04;

    if ((val04 = o->header.gfx.animInfo.animAccel / 0x10000) <= 0) {
        val04 = 1;
    }

    if (cur_obj_check_anim_frame_in_range(arg0, val04) || cur_obj_check_anim_frame_in_range(arg1, val04)) {
        cur_obj_play_sound_2(sound);
        return TRUE;
    }

    return FALSE;
}

s16 cur_obj_reflect_move_angle_off_wall(void) {
    s16 angle = o->oWallAngle - ((s16) o->oMoveAngleYaw - (s16) o->oWallAngle) + 0x8000;
    return angle;
}

s32 cur_obj_resolve_wall_collisions(void) {
    s32 numCollisions;
    struct SM64SurfaceCollisionData *wall;
    struct SM64WallCollisionData collisionData;

    f32 offsetY = 10.0f;
    f32 radius = o->oWallHitboxRadius;

    if (radius > 0.1L) {
        collisionData.offsetY = offsetY;
        collisionData.radius = radius;
        collisionData.x = (s16) o->oPosX;
        collisionData.y = (s16) o->oPosY;
        collisionData.z = (s16) o->oPosZ;

        numCollisions = find_wall_collisions(&collisionData);
        if (numCollisions != 0) {
            o->oPosX = collisionData.x;
            o->oPosY = collisionData.y;
            o->oPosZ = collisionData.z;
            wall = collisionData.walls[collisionData.numWalls - 1];

            o->oWallAngle = atan2s(wall->normal.z, wall->normal.x);
            if (abs_angle_diff(o->oWallAngle, o->oMoveAngleYaw) > 0x4000) {
                return TRUE;
            } else {
                return FALSE;
            }
        }
    }

    return FALSE;
}

s32 cur_obj_rotate_yaw_toward(s16 target, s16 increment) {
    s16 startYaw;

    startYaw = (s16) o->oMoveAngleYaw;
    o->oMoveAngleYaw = approach_s16_symmetric(o->oMoveAngleYaw, target, increment);

    if ((o->oAngleVelYaw = (s16)((s16) o->oMoveAngleYaw - startYaw)) == 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void cur_obj_scale(f32 scale) {
    o->header.gfx.scale[0] = scale;
    o->header.gfx.scale[1] = scale;
    o->header.gfx.scale[2] = scale;
}

static void cur_obj_update_floor(void) {
    struct SM64SurfaceCollisionData *floor = cur_obj_update_floor_height_and_get_floor();
    o->oFloor = floor;

    if (floor != NULL) {
        if (floor->type == SURFACE_BURNING) {
            o->oMoveFlags |= OBJ_MOVE_ABOVE_LAVA;
        }
#ifndef VERSION_JP
        else if (floor->type == SURFACE_DEATH_PLANE) {
            //! This misses SURFACE_VERTICAL_WIND (and maybe SURFACE_WARP)
            o->oMoveFlags |= OBJ_MOVE_ABOVE_DEATH_BARRIER;
        }
#endif

        o->oFloorType = floor->type;
        o->oFloorRoom = floor->room;
    } else {
        o->oFloorType = 0;
        o->oFloorRoom = 0;
    }
}

static void cur_obj_update_floor_and_resolve_wall_collisions(s16 steepSlopeDegrees) {
#ifdef VERSION_JP
    o->oMoveFlags &= ~OBJ_MOVE_ABOVE_LAVA;
#else
    o->oMoveFlags &= ~(OBJ_MOVE_ABOVE_LAVA | OBJ_MOVE_ABOVE_DEATH_BARRIER);
#endif

    if (o->activeFlags & (ACTIVE_FLAG_FAR_AWAY | ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
        cur_obj_update_floor();
        o->oMoveFlags &= ~(OBJ_MOVE_HIT_WALL | OBJ_MOVE_MASK_IN_WATER);

        if (o->oPosY > o->oFloorHeight) {
            o->oMoveFlags |= OBJ_MOVE_IN_AIR;
        }
    } else {
        o->oMoveFlags &= ~OBJ_MOVE_HIT_WALL;
        if (cur_obj_resolve_wall_collisions()) {
            o->oMoveFlags |= OBJ_MOVE_HIT_WALL;
        }

        cur_obj_update_floor();

        if (o->oPosY > o->oFloorHeight) {
            o->oMoveFlags |= OBJ_MOVE_IN_AIR;
        }

        if (cur_obj_detect_steep_floor(steepSlopeDegrees)) {
            o->oMoveFlags |= OBJ_MOVE_HIT_WALL;
        }
    }
}

void cur_obj_update_floor_and_walls(void) {
    cur_obj_update_floor_and_resolve_wall_collisions(60);
}

struct SM64SurfaceCollisionData *cur_obj_update_floor_height_and_get_floor(void) {
    struct SM64SurfaceCollisionData *floor;
    o->oFloorHeight = find_floor(o->oPosX, o->oPosY, o->oPosZ, &floor);
    return floor;
}

static void obj_act_knockback(UNUSED f32 baseScale) {
    cur_obj_update_floor_and_walls();

    if (o->header.gfx.animInfo.curAnim != NULL) {
        cur_obj_extend_animation_if_at_end();
    }

    //! Dies immediately if above lava
    if ((o->oMoveFlags
         & (OBJ_MOVE_MASK_ON_GROUND | OBJ_MOVE_MASK_IN_WATER | OBJ_MOVE_HIT_WALL | OBJ_MOVE_ABOVE_LAVA))
        || (o->oAction == OBJ_ACT_VERTICAL_KNOCKBACK && o->oTimer >= 9)) {
        obj_die_if_health_non_positive();
    }

    cur_obj_move_standard(-78);
}

static void obj_act_squished(f32 baseScale) {
    f32 targetScaleY = baseScale * 0.3f;

    cur_obj_update_floor_and_walls();

    if (o->header.gfx.animInfo.curAnim != NULL) {
        cur_obj_extend_animation_if_at_end();
    }

    if (approach_f32_ptr(&o->header.gfx.scale[1], targetScaleY, baseScale * 0.14f)) {
        o->header.gfx.scale[0] = o->header.gfx.scale[2] = baseScale * 2.0f - o->header.gfx.scale[1];

        if (o->oTimer > 15) {
            obj_die_if_health_non_positive();
        }
    }

    o->oForwardVel = 0.0f;
    cur_obj_move_standard(-78);
}


s32 obj_bounce_off_walls_edges_objects(s32 *targetYaw) {
    if (o->oMoveFlags & OBJ_MOVE_HIT_WALL) {
        *targetYaw = cur_obj_reflect_move_angle_off_wall();
    } else if (o->oMoveFlags & OBJ_MOVE_HIT_EDGE) {
        *targetYaw = (s16)(o->oMoveAngleYaw + 0x8000);
    } else if (!obj_resolve_object_collisions(targetYaw)) {
        return FALSE;
    }

    return TRUE;
}

void obj_copy_angle(struct Object *dst, struct Object *src) {
    dst->oMoveAnglePitch = src->oMoveAnglePitch;
    dst->oMoveAngleYaw = src->oMoveAngleYaw;
    dst->oMoveAngleRoll = src->oMoveAngleRoll;

    dst->oFaceAnglePitch = src->oFaceAnglePitch;
    dst->oFaceAngleYaw = src->oFaceAngleYaw;
    dst->oFaceAngleRoll = src->oFaceAngleRoll;
}

void obj_copy_pos(struct Object *dst, struct Object *src) {
    dst->oPosX = src->oPosX;
    dst->oPosY = src->oPosY;
    dst->oPosZ = src->oPosZ;
}

void obj_copy_pos_and_angle(struct Object *dst, struct Object *src) {
    obj_copy_pos(dst, src);
    obj_copy_angle(dst, src);
}

static s32 obj_die_if_above_lava_and_health_non_positive(void) {
    if (o->oMoveFlags & OBJ_MOVE_UNDERWATER_ON_GROUND) {
        if (o->oGravity + o->oBuoyancy > 0.0f
            || find_water_level(o->oPosX, o->oPosZ) - o->oPosY < 150.0f) {
            return FALSE;
        }
    } else if (!(o->oMoveFlags & OBJ_MOVE_ABOVE_LAVA)) {
        if (o->oMoveFlags & OBJ_MOVE_ENTERED_WATER) {
            if (o->oWallHitboxRadius < 200.0f) {
                cur_obj_play_sound_2(SOUND_OBJ_DIVING_INTO_WATER);
            } else {
                cur_obj_play_sound_2(SOUND_OBJ_DIVING_IN_WATER);
            }
        }
        return FALSE;
    }

    obj_die_if_health_non_positive();
    return TRUE;
}

void obj_die_if_health_non_positive(void) {
    if (o->oHealth <= 0) {
        if (o->oDeathSound == 0) {
            spawn_mist_particles_with_sound(SOUND_OBJ_DEFAULT_DEATH);
        } else if (o->oDeathSound > 0) {
            spawn_mist_particles_with_sound(o->oDeathSound);
        } else {
            spawn_mist_particles();
        }

        if (o->oNumLootCoins < 0) {
        } else {
            obj_spawn_loot_yellow_coins(o, o->oNumLootCoins, 20.0f);
        }
        obj_spawn_loot_yellow_coins(o, o->oNumLootCoins, 20.0f);

        if (o->oHealth < 0) {
            cur_obj_hide();
            cur_obj_become_intangible();
        } else {
            obj_mark_for_deletion(o);
        }
    }
}

s32 obj_forward_vel_approach(f32 target, f32 delta) {
    return approach_f32_ptr(&o->oForwardVel, target, delta);
}

s32 obj_handle_attacks(struct ObjectHitbox *hitbox, s32 attackedMarioAction,
                              u8 *attackHandlers) {
    s32 attackType;

    obj_set_hitbox(o, hitbox);

    //! Die immediately if above lava
    if (obj_die_if_above_lava_and_health_non_positive()) {
        return 1;
    } else if (o->oInteractStatus & INT_STATUS_INTERACTED) {
        if (o->oInteractStatus & INT_STATUS_ATTACKED_MARIO) {
            if (o->oAction != attackedMarioAction) {
                o->oAction = attackedMarioAction;
                o->oTimer = 0;
            }
        } else {
            attackType = o->oInteractStatus & INT_STATUS_ATTACK_MASK;

            switch (attackHandlers[attackType - 1]) {
                case ATTACK_HANDLER_NOP:
                    break;

                case ATTACK_HANDLER_DIE_IF_HEALTH_NON_POSITIVE:
                    obj_die_if_health_non_positive();
                    break;

                case ATTACK_HANDLER_KNOCKBACK:
                    obj_set_knockback_action(attackType);
                    break;

                case ATTACK_HANDLER_SQUISHED:
                    obj_set_squished_action();
                    break;

                case ATTACK_HANDLER_SPECIAL_KOOPA_LOSE_SHELL:
                    shelled_koopa_attack_handler(attackType);
                    break;

                case ATTACK_HANDLER_SET_SPEED_TO_ZERO:
                    obj_set_speed_to_zero();
                    break;

                case ATTACK_HANDLER_SPECIAL_WIGGLER_JUMPED_ON:
                    wiggler_jumped_on_attack_handler();
                    break;

                case ATTACK_HANDLER_SPECIAL_HUGE_GOOMBA_WEAKLY_ATTACKED:
                    huge_goomba_weakly_attacked();
                    break;

                case ATTACK_HANDLER_SQUISHED_WITH_BLUE_COIN:
                    o->oNumLootCoins = -1;
                    obj_set_squished_action();
                    break;
            }

            o->oInteractStatus = 0;
            return attackType;
        }
    }

    o->oInteractStatus = 0;
    return 0;
}


void obj_mark_for_deletion(struct Object *obj) {

    obj->activeFlags = ACTIVE_FLAG_DEACTIVATED;
}

s16 obj_random_fixed_turn(s16 delta) {
    return o->oMoveAngleYaw + (s16) random_sign() * delta;
}

s32 obj_resolve_collisions_and_turn(s16 targetYaw, s16 turnSpeed) {
    obj_resolve_object_collisions(NULL);

    if (cur_obj_rotate_yaw_toward(targetYaw, turnSpeed)) {
        return FALSE;
    } else {
        return TRUE;
    }
}

s32 obj_resolve_object_collisions(s32 *targetYaw) {
    struct Object *otherObject;
    f32 dx;
    f32 dz;
    s16 angle;
    f32 radius;
    f32 otherRadius;
    f32 relativeRadius;
    f32 newCenterX;
    f32 newCenterZ;

    if (o->numCollidedObjs != 0) {
        otherObject = o->collidedObjs[0];
        if (otherObject != gMarioObject) {
            //! If one object moves after collisions are detected and this code
            //  runs, the objects can move toward each other (transport cloning)

            dx = otherObject->oPosX - o->oPosX;
            dz = otherObject->oPosZ - o->oPosZ;
            angle = atan2s(dx, dz); //! This should be atan2s(dz, dx)

            radius = o->hitboxRadius;
            otherRadius = otherObject->hitboxRadius;
            relativeRadius = radius / (radius + otherRadius);

            newCenterX = o->oPosX + dx * relativeRadius;
            newCenterZ = o->oPosZ + dz * relativeRadius;

            o->oPosX = newCenterX - radius * coss(angle);
            o->oPosZ = newCenterZ - radius * sins(angle);

            otherObject->oPosX = newCenterX + otherRadius * coss(angle);
            otherObject->oPosZ = newCenterZ + otherRadius * sins(angle);

            if (targetYaw != NULL && abs_angle_diff(o->oMoveAngleYaw, angle) < 0x4000) {
                // Bounce off object (or it would, if the above atan2s bug
                // were fixed)
                *targetYaw = (s16)(angle - o->oMoveAngleYaw + angle + 0x8000);
                return TRUE;
            }
        }
    }

    return FALSE;
}


void obj_set_hitbox(struct Object *obj, struct ObjectHitbox *hitbox) {
    if (!(obj->oFlags & OBJ_FLAG_30)) {
        obj->oFlags |= OBJ_FLAG_30;

        obj->oInteractType = hitbox->interactType;
        obj->oDamageOrCoinValue = hitbox->damageOrCoinValue;
        obj->oHealth = hitbox->health;
        obj->oNumLootCoins = hitbox->numLootCoins;

        cur_obj_become_tangible();
    }

    obj->hitboxRadius = obj->header.gfx.scale[0] * hitbox->radius;
    obj->hitboxHeight = obj->header.gfx.scale[1] * hitbox->height;
    obj->hurtboxRadius = obj->header.gfx.scale[0] * hitbox->hurtboxRadius;
    obj->hurtboxHeight = obj->header.gfx.scale[1] * hitbox->hurtboxHeight;
    obj->hitboxDownOffset = obj->header.gfx.scale[1] * hitbox->downOffset;
}

void obj_set_knockback_action(s32 attackType) {
    switch (attackType) {
        case ATTACK_KICK_OR_TRIP:
        case ATTACK_FAST_ATTACK:
            o->oAction = OBJ_ACT_VERTICAL_KNOCKBACK;
            o->oForwardVel = 20.0f;
            o->oVelY = 50.0f;
            break;

        default:
            o->oAction = OBJ_ACT_HORIZONTAL_KNOCKBACK;
            o->oForwardVel = 50.0f;
            o->oVelY = 30.0f;
            break;
    }

    o->oFlags &= ~OBJ_FLAG_SET_FACE_YAW_TO_MOVE_YAW;
    o->oMoveAngleYaw = obj_angle_to_object(gMarioObject, o);
}

void obj_set_squished_action(void) {
    cur_obj_play_sound_2(SOUND_OBJ_STOMPED);
    o->oAction = OBJ_ACT_SQUISHED;
}

void obj_spawn_loot_coins(struct Object *obj, s32 numCoins, f32 baseVelY,
                                    const BehaviorScript *coinBehavior,
                                    s16 posJitter, s16 model) {
    /*
    s32 i;
    f32 spawnHeight;
    struct SM64SurfaceCollisionData *floor;
    struct Object *coin;

    spawnHeight = find_floor(obj->oPosX, obj->oPosY, obj->oPosZ, &floor);
    if (obj->oPosY - spawnHeight > 100.0f) {
        spawnHeight = obj->oPosY;
    }

    for (i = 0; i < numCoins; i++) {
        if (obj->oNumLootCoins <= 0) {
            break;
        }

        obj->oNumLootCoins--;

        coin = spawn_object(obj, model, coinBehavior);
        obj_translate_xz_random(coin, posJitter);
        coin->oPosY = spawnHeight;
        coin->oCoinBaseVelY = baseVelY;
    }
    */
}

void obj_spawn_loot_yellow_coins(struct Object *obj, s32 numCoins, f32 baseVelY) {
    //obj_spawn_loot_coins(obj, numCoins, baseVelY, bhvSingleCoinGetsSpawned, 0, MODEL_YELLOW_COIN);
}

void obj_translate_xz_random(struct Object *obj, f32 rangeLength) {
    obj->oPosX += random_float() * rangeLength - rangeLength * 0.5f;
    obj->oPosZ += random_float() * rangeLength - rangeLength * 0.5f;
}

void obj_update_blinking(s32 *blinkTimer, s16 baseCycleLength, s16 cycleLengthRange,
                                s16 blinkLength) {
    if (*blinkTimer != 0) {
        (*blinkTimer)--;
    } else {
        *blinkTimer = random_linear_offset(baseCycleLength, cycleLengthRange);
    }

    if (*blinkTimer > blinkLength) {
        o->oAnimState = 0;
    } else {
        o->oAnimState = 1;
    }
}


s32 obj_update_standard_actions(f32 scale) {
    if (o->oAction < 100) {
        return TRUE;
    } else {
        cur_obj_become_intangible();

        switch (o->oAction) {
            case OBJ_ACT_HORIZONTAL_KNOCKBACK:
            case OBJ_ACT_VERTICAL_KNOCKBACK:
                obj_act_knockback(scale);
                break;

            case OBJ_ACT_SQUISHED:
                obj_act_squished(scale);
                break;
        }

        return FALSE;
    }
}



void bhv_init_room(void) {
    //struct SM64SurfaceCollisionData *floor;
    //f32 floorHeight;
    /*
    if (is_item_in_array(gCurrLevelNum, sLevelsWithRooms)) {
        floorHeight = find_floor(o->oPosX, o->oPosY, o->oPosZ, &floor);

        if (floor != NULL) {
            if (floor->room != 0) {
                o->oRoom = floor->room;
            } else {
                // Floor probably belongs to a platform object. Try looking
                // underneath it
                find_floor(o->oPosX, floorHeight - 100.0f, o->oPosZ, &floor);
                if (floor != NULL) {
                    //! Technically possible that the room could still be 0 here
                    o->oRoom = floor->room;
                }
            }
        }
    } else {
        o->oRoom = -1;
    }
    */
   o->oRoom = 0;
}

s32 cur_obj_wait_then_blink(s32 timeUntilBlinking, s32 numBlinks) {
    s32 done = false;
    s32 timeBlinking;

    if (o->oTimer >= timeUntilBlinking) {
        if ((timeBlinking = o->oTimer - timeUntilBlinking) % 2 != 0) {
            o->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
            if (timeBlinking / 2 > numBlinks) {
                done = true;
            }
        } else {
            o->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
        }
    }

    return done;
}

s8 obj_flicker_and_disappear(struct Object *obj, s16 lifeSpan) {
    if (obj->oTimer < lifeSpan) {
        return FALSE;
    }

    if (obj->oTimer < lifeSpan + 40) {
        if (obj->oTimer % 2 != 0) {
            obj->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
        } else {
            obj->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
        }
    } else {
        obj->activeFlags = ACTIVE_FLAG_DEACTIVATED;
        return TRUE;
    }

    return FALSE;
}
s8 is_point_within_radius_of_mario(f32 x, f32 y, f32 z, s32 dist) {
    f32 mGfxX = gMarioObject->header.gfx.pos[0];
    f32 mGfxY = gMarioObject->header.gfx.pos[1];
    f32 mGfxZ = gMarioObject->header.gfx.pos[2];

    if ((x - mGfxX) * (x - mGfxX) + (y - mGfxY) * (y - mGfxY) + (z - mGfxZ) * (z - mGfxZ)
        < (f32)(dist * dist)) {
        return TRUE;
    }

    return FALSE;
}

void set_object_visibility(struct Object *obj, s32 dist) {
    f32 objX = obj->oPosX;
    f32 objY = obj->oPosY;
    f32 objZ = obj->oPosZ;

    if (is_point_within_radius_of_mario(objX, objY, objZ, dist) == TRUE) {
        obj->header.gfx.node.flags &= ~GRAPH_RENDER_INVISIBLE;
    } else {
        obj->header.gfx.node.flags |= GRAPH_RENDER_INVISIBLE;
    }
}

s16 random_linear_offset(s16 base, s16 range) {
    return base + (s16)(range * random_float());
}

void spawn_mist_particles(void) {
    //spawn_mist_particles_variable(0, 0, 46.0f);
}

void spawn_mist_particles_with_sound(u32 soundMagic) {
    //spawn_mist_particles_variable(0, 0, 46.0f);
    create_sound_spawner(soundMagic);
}

struct Object *spawn_object_at_origin(struct Object *parent, UNUSED s32 unusedArg, u32 model,
                                      const BehaviorScript *behavior) {
    struct Object *obj;
    const BehaviorScript *behaviorAddr;

    behaviorAddr = segmented_to_virtual(behavior);
    obj = create_object_bhv(behaviorAddr);

    obj->parentObj = parent;
    //obj->header.gfx.areaIndex = parent->header.gfx.areaIndex;
    //obj->header.gfx.activeAreaIndex = parent->header.gfx.areaIndex;


    geo_obj_init((struct GraphNodeObject *) &obj->header.gfx, NULL, gVec3fZero,
                 gVec3sZero);
    obj->unused2 = putObjectInActorPool(obj);
    return obj;
}

struct Object *spawn_object(struct Object *parent, s32 model, const BehaviorScript *behavior) {
    struct Object *obj = spawn_object_at_origin(parent, 0, model, behavior);

    obj_copy_pos_and_angle(obj, parent);
    
    return obj;
}

void treat_far_home_as_mario(f32 threshold) {
    f32 dx = o->oHomeX - o->oPosX;
    f32 dy = o->oHomeY - o->oPosY;
    f32 dz = o->oHomeZ - o->oPosZ;
    f32 distance = sqrtf(dx * dx + dy * dy + dz * dz);

    if (distance > threshold) {
        o->oAngleToMario = atan2s(dz, dx);
        o->oDistanceToMario = 25000.0f;
    } else {
        dx = o->oHomeX - gMarioObject->oPosX;
        dy = o->oHomeY - gMarioObject->oPosY;
        dz = o->oHomeZ - gMarioObject->oPosZ;
        distance = sqrtf(dx * dx + dy * dy + dz * dz);

        if (distance > threshold) {
            o->oDistanceToMario = 20000.0f;
        }
    }
}
s16 obj_angle_to_object(struct Object *obj1, struct Object *obj2) {
    f32 z1, x1, z2, x2;
    s16 angle;

    z1 = obj1->oPosZ; z2 = obj2->oPosZ; // ordering of instructions..
    x1 = obj1->oPosX; x2 = obj2->oPosX;

    angle = atan2s(z2 - z1, x2 - x1);
    return angle;
}

s32 obj_is_rendering_enabled(void) {
    if (o->header.gfx.node.flags & GRAPH_RENDER_ACTIVE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

void wiggler_jumped_on_attack_handler(void);

s32 cur_obj_get_int(u8 field) {
    return ((struct Object *)gCurrentObject)->rawData.asS32[field];
}

void cur_obj_set_int(u8 field, s32 value) {
    ((struct Object *)gCurrentObject)->rawData.asS32[field] = value;
}

void cur_obj_or_int(u8 field, s32 value) {
    ((struct Object *)gCurrentObject)->rawData.asS32[field] |= value;
}

void cur_obj_and_int(u8 field, s32 value) {
    ((struct Object *)gCurrentObject)->rawData.asS32[field] &= value;
}

void cur_obj_add_int(u8 field, s32 value) {
    ((struct Object *)gCurrentObject)->rawData.asS32[field] += value;
}

f32 cur_obj_get_float(u8 field) {
    return ((struct Object *)gCurrentObject)->rawData.asF32[field];
}

void cur_obj_set_float(u8 field, f32 value) {
    ((struct Object *)gCurrentObject)->rawData.asF32[field] = value;
}

void cur_obj_add_float(u8 field, f32 value) {
    ((struct Object *)gCurrentObject)->rawData.asF32[field] += value;
}

void cur_obj_set_vptr(u8 field, void *value) {
#if IS_64_BIT
    ((struct Object *)gCurrentObject)->ptrData.asVoidPtr[field] = value;
#else
    ((struct Object *)gCurrentObject)->rawData.asVoidPtr[field] = value;
#endif
}

void obj_and_int(struct Object *obj, u8 field, s32 value) {
    obj->rawData.asS32[field] &= value;
}
