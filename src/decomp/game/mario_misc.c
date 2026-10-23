#include "../include/PR/ultratypes.h"

#include "../include/sm64.h"
#include "area.h"
//#include "audio/external.h"
//#include "behavior_actions.h"
//#include "behavior_data.h"
#include "camera.h"
//#include "dialog_ids.h"
//#include "engine/behavior_script.h"
#include "../engine/graph_node.h"
#include "../engine/math_util.h"
//#include "envfx_snow.h"
//#include "game_init.h"
//#include "goddard/renderer.h"
#include "interaction.h"
#include "level_update.h"
#include "mario_misc.h"
#include "../memory.h"
//#include "object_helpers.h"
//#include "object_list_processor.h"
#include "rendering_graph_node.h"
#include "save_file.h"
//#include "skybox.h"
//#include "sound_init.h"
#include "../shim.h"
#include "../include/mario_animation_ids.h"
#include "../include/object_fields.h"
#include "../include/mario_geo_switch_case_ids.h"
#include "../include/custom_anims/custom_anims.h"

static Vec3s gVec3sZero = {0,0,0};

#define TOAD_STAR_1_REQUIREMENT 12
#define TOAD_STAR_2_REQUIREMENT 25
#define TOAD_STAR_3_REQUIREMENT 35

#define TOAD_STAR_1_DIALOG 90 // DIALOG_082
#define TOAD_STAR_2_DIALOG 91 // DIALOG_076
#define TOAD_STAR_3_DIALOG 92 // DIALOG_083

#define TOAD_STAR_1_DIALOG_AFTER 90 // DIALOG_154
#define TOAD_STAR_2_DIALOG_AFTER 91 // DIALOG_155
#define TOAD_STAR_3_DIALOG_AFTER 92 // DIALOG_156

enum ToadMessageStates {
    TOAD_MESSAGE_FADED,
    TOAD_MESSAGE_OPAQUE,
    TOAD_MESSAGE_OPACIFYING,
    TOAD_MESSAGE_FADING,
    TOAD_MESSAGE_TALKING
};

enum UnlockDoorStarStates {
    UNLOCK_DOOR_STAR_RISING,
    UNLOCK_DOOR_STAR_WAITING,
    UNLOCK_DOOR_STAR_SPAWNING_PARTICLES,
    UNLOCK_DOOR_STAR_DONE
};

static s8 gMiaAnimTableMouth[] = {
    [0x00] = MIA_MOUTH_WATER, [0x01] = MIA_MOUTH_WAAA, [0x02] = MIA_MOUTH_WAAA,
    [0x04] = MIA_MOUTH_U, [0x05] = MIA_MOUTH_WATER, [0x06] = MIA_MOUTH_O,
    [0x07] = MIA_MOUTH_O, [0x08] = MIA_MOUTH_O, [0x0A] = MIA_MOUTH_HAPPY,
    [0x0D] = MIA_MOUTH_WATER, [0x0F] = MIA_MOUTH_O, [0x13] = MIA_MOUTH_HAPPY,
    [0x14] = MIA_MOUTH_HAPPY, [0x19] = MIA_MOUTH_O, [0x1A] = MIA_MOUTH_WATER,
    [0x1B] = MIA_MOUTH_V, [0x1C] = MIA_MOUTH_WATER, [0x24] = MIA_MOUTH_U,
    [0x28] = MIA_MOUTH_V, [0x29] = MIA_MOUTH_WAAA, [0x2C] = MIA_MOUTH_WAAA,
    [0x2D] = MIA_MOUTH_WAAA, [0x33] = MIA_MOUTH_U, [0x34] = MIA_MOUTH_U,
    [0x35] = MIA_MOUTH_O, [0x39] = MIA_MOUTH_O, [0x3A] = MIA_MOUTH_WATER,
    [0x3C] = MIA_MOUTH_WATER, [0x3D] = MIA_MOUTH_WATER, [0x3E] = MIA_MOUTH_O,
    [0x3F] = MIA_MOUTH_WATER, [0x41] = MIA_MOUTH_HAPPY, [0x47] = MIA_MOUTH_O,
    [0x4A] = MIA_MOUTH_HAPPY, [0x4C] = MIA_MOUTH_O, [0x4D] = MIA_MOUTH_HAPPY,
    [0x4F] = MIA_MOUTH_HAPPY, [0x50] = MIA_MOUTH_HAPPY, [0x52] = MIA_MOUTH_O,
    [0x55] = MIA_MOUTH_O, [0x58] = MIA_MOUTH_V, [0x59] = MIA_MOUTH_V,
    [0x5A] = MIA_MOUTH_U, [0x5C] = MIA_MOUTH_U, [0x5D] = MIA_MOUTH_U,
    [0x65] = MIA_MOUTH_O, [0x66] = MIA_MOUTH_HAPPY, [0x67] = MIA_MOUTH_HAPPY,
    [0x68] = MIA_MOUTH_O, [0x69] = MIA_MOUTH_HAPPY, [0x6A] = MIA_MOUTH_O,
    [0x6B] = MIA_MOUTH_O, [0x6D] = MIA_MOUTH_O, [0x74] = MIA_MOUTH_O,
    [0x75] = MIA_MOUTH_O, [0x76] = MIA_MOUTH_WAAA, [0x77] = MIA_MOUTH_V,
    [0x78] = MIA_MOUTH_WATER, [0x7A] = MIA_MOUTH_WAAA, [0x7B] = MIA_MOUTH_V,
    [0x7C] = MIA_MOUTH_V, [0x7D] = MIA_MOUTH_WATER, [0x7E] = MIA_MOUTH_WATER,
    [0x7F] = MIA_MOUTH_WATER, [0x80] = MIA_MOUTH_WATER,
    [0x82] = MIA_MOUTH_V, [0x83] = MIA_MOUTH_O, [0x85] = MIA_MOUTH_CAT,
    [0x86] = MIA_MOUTH_CAT, [0x87] = MIA_MOUTH_CAT, [0x88] = MIA_MOUTH_O,
    [0x89] = MIA_MOUTH_O, [0x8A] = MIA_MOUTH_V, [0x8C] = MIA_MOUTH_U,
    [0x8E] = MIA_MOUTH_U, [0x91] = MIA_MOUTH_U, [0x92] = MIA_MOUTH_O,
    [0x95] = MIA_MOUTH_O, [0x99] = MIA_MOUTH_CAT, [0x9C] = MIA_MOUTH_HAPPY,
    [0x9E] = MIA_MOUTH_V, [0xA8] = MIA_MOUTH_V, [0xAE] = MIA_MOUTH_O,
    [0xB3] = MIA_MOUTH_HAPPY, [0xB5] = MIA_MOUTH_V, [0xB6] = MIA_MOUTH_V,
    [0xB7] = MIA_MOUTH_V, [0xB8] = MIA_MOUTH_WATER, [0xB9] = MIA_MOUTH_WATER,
    [0xBB] = MIA_MOUTH_WATER, [0xBC] = MIA_MOUTH_O,
    [0xBD] = MIA_MOUTH_O, [0xC0] = MIA_MOUTH_HAPPY, [0xC1] = MIA_MOUTH_HAPPY,
    [0xC2] = MIA_MOUTH_U, [0xC6] = MIA_MOUTH_WATER, [0xC7] = MIA_MOUTH_WATER,
    [0xCD] = MIA_MOUTH_HAPPY, [0xD0] = MIA_MOUTH_HAPPY, [0xCC] = MIA_MOUTH_HAPPY,[0xCB] = MIA_MOUTH_HAPPY
};

static s8 gMiaAnimTableEyes[0xD1] = {
    [0x1D] = 8, [0x27] = 8, [0x6E] = 8, [0xB3] = 8, [0xCD] = 8,
    [0xC0] = 9, [0x4F] = 9, [0x66] = 9, [0x7C] = 9,
    [0x7B] = 9, [0x74] = 9, [0x75] = 9, [0x2D] = 9,
    [0x02] = 9, [0xA8] = 9, [0x9E] = 9, [0x31] = 9,
    [0x71] = 9, [0x8A] = 9, [0x76] = 9, [0x29] = 9,
    [0x01] = 9, [0x2C] = 9, [0x3A] = 9, [0x94] = 9,
    [0x7A] = 9, [0xB7] = 9, [0xB5] = 9, [0x59] = 9,
    [0x58] = 9,
    [0x4C] = MARIO_EYES_LOOK_DOWN - 1, [0x3D] = MARIO_EYES_LOOK_DOWN - 1,
    [0x1C] = MARIO_EYES_LOOK_DOWN - 1, [0x56] = MARIO_EYES_LOOK_DOWN - 1,
    [0x0D] = MARIO_EYES_LOOK_DOWN - 1, [0xC6] = MARIO_EYES_LOOK_DOWN - 1,
    [0xC7] = MARIO_EYES_LOOK_DOWN - 1, [0x2B] = MARIO_EYES_LOOK_DOWN - 1,
    [0x50] = MARIO_EYES_LOOK_UP - 1, [0x05] = MARIO_EYES_LOOK_UP - 1,
    [0x95] = MARIO_EYES_LOOK_UP - 1, [0x33] = MARIO_EYES_LOOK_UP - 1,
    [0x35] = MARIO_EYES_LOOK_UP - 1,
    [0x0F] = MARIO_EYES_LOOK_RIGHT - 1, [0xBC] = MARIO_EYES_LOOK_RIGHT - 1,
    [0x7F] = MARIO_EYES_LOOK_RIGHT - 1, [0x80] = MARIO_EYES_LOOK_LEFT - 1,
    [0x36] = MARIO_EYES_CLOSED - 1, [0x37] = MARIO_EYES_CLOSED - 1,
    [0x81] = MARIO_EYES_HALF_CLOSED - 1, [0x82] = MARIO_EYES_HALF_CLOSED - 1,
    [0x84] = MARIO_EYES_HALF_CLOSED - 1, [0x83] = MARIO_EYES_HALF_CLOSED - 1,
};

static void mia_special_expressions(struct MarioState *m, s16 *eyeState, s16 *mouthState) {
    s16 animID = m->marioObj->header.gfx.animInfo.animID;
    s16 animFrame = m->marioObj->header.gfx.animInfo.animFrame;

    *eyeState = -1;
    *mouthState = -1;
    if (m->action == ACT_IDLE && m->actionTimer >= 7) {
        *eyeState = MARIO_EYES_HALF_CLOSED - 1;
    } else if (animID == MARIO_ANIM_TURNING_PART2 && animFrame < 9) {
        *eyeState = MARIO_EYES_LOOK_RIGHT - 1;
        *mouthState = MIA_MOUTH_O;
    } else if (animID == MARIO_ANIM_TRIPLE_JUMP_LAND && animFrame > 20) {
        *eyeState = MARIO_EYES_BLINK;
        *mouthState = MIA_MOUTH_NORMAL;
    } else if (animID == MARIO_ANIM_GROUND_KICK && animFrame > 15) {
        *eyeState = MARIO_EYES_BLINK;
        *mouthState = MIA_MOUTH_NORMAL;
    } else if (animID == MARIO_ANIM_TRIPLE_JUMP && animFrame > 30) {
        *eyeState = MARIO_EYES_LOOK_DOWN - 1;
        *mouthState = MIA_MOUTH_O;
    } else if ((animID == MARIO_ANIM_FAST_LONGJUMP || animID == MARIO_ANIM_SLOW_LONGJUMP)
               && animFrame > 11) {
        *mouthState = MIA_MOUTH_O;
    } else if (animID == MARIO_ANIM_TAKE_CAP_OFF_THEN_ON && animFrame > 83) {
        *eyeState = MARIO_EYES_BLINK;
    } else if (m->action == ACT_SOFT_BONK || m->action == ACT_SHOCKWAVE_BOUNCE
                 || (m->particleFlags & PARTICLE_FIRE)
               || (m->hurtCounter > 0 && !(m->action & ACT_GROUP_CUTSCENE))) {
        *eyeState = 9;
        *mouthState = MIA_MOUTH_V;
    } else if ((m->flags & MARIO_UNKNOWN_18) && !(m->action & ACT_GROUP_CUTSCENE)) {
        *eyeState = 9;
        *mouthState = MIA_MOUTH_WAAA;
    } else if (m->action == ACT_STEEP_JUMP) {
        *mouthState = MIA_MOUTH_O;
    }

    if (*mouthState < 0 && m->action == ACT_WALKING && m->forwardVel > 40.0f) {
        *mouthState = MIA_MOUTH_HAPPY;
    }
    if (animID == MIA_ANIM_JUMP){
        *mouthState = MIA_MOUTH_HAPPY;
    } else if (animID == MIA_ANIM_FIRST_PERSON){
        *mouthState = MIA_MOUTH_U;
    }
    if (animID == MIA_ANIM_STAR_DANCE) {
        if (animFrame < 27){
            *mouthState = MIA_MOUTH_HAPPY;
            *eyeState = MARIO_EYES_BLINK;
            //hands = MARIO_HAND_FISTS;
        }else if (animFrame >= 27 && animFrame < 48) {
             *mouthState = MIA_MOUTH_O;
             *eyeState = 9;
             //hands = MARIO_HAND_OPEN;
        }else if (animFrame >= 48 && animFrame <= 57) {
            *mouthState = MIA_MOUTH_NORMAL;
            *eyeState = MARIO_EYES_CLOSED  - 1;
            //hands = MARIO_HAND_FISTS ;
        }else if (animFrame > 57) {
             *mouthState = MIA_MOUTH_HAPPY;
             *eyeState = 8;
             //hands = MARIO_HAND_PEACE_SIGN;
        }
    }

    if (m->action == ACT_WALKING || m->action == ACT_BUTT_SLIDE ||
            m->action == ACT_DIVE_SLIDE ||
            m->action == ACT_STOMACH_SLIDE ||
            m->action == ACT_HOLD_BUTT_SLIDE ||
            m->action == ACT_SLIDE_KICK_SLIDE ||
            m->action == ACT_HOLD_STOMACH_SLIDE) {
        if (m->input & INPUT_NONZERO_ANALOG) {
            if (m->faceAngle[1] < m->intendedYaw) {
                *eyeState = MARIO_EYES_LOOK_RIGHT - 1;
            }

            if (m->faceAngle[1] > m->intendedYaw) {
                *eyeState = MARIO_EYES_LOOK_LEFT - 1;
            }
        }
    }
    if ((m->health < 0x300) && !(m->action & ACT_GROUP_CUTSCENE)) {
        *mouthState = MIA_MOUTH_V;
    }
}


#define CLAMP(x, low, high)  MIN(MAX((x), (low)), (high))
/**
 * The eye texture on succesive frames of Mario's blink animation.
 * He intentionally blinks twice each time.
 */
static s8 gMarioBlinkAnimation[7] = { 1, 2, 1, 0, 1, 2, 1 };

//mia...mia
static s8 gMiaBlinkAnimation[6] = { 1, 2, 2, 1, 0, 0 };

/**
 * The scale values per frame for Mario's foot/hand for his attack animation
 * There are 3 scale animations in groups of 6 frames.
 * The first animation starts at frame index 3 and goes down, the others start at frame index 5.
 * The values get divided by 10 before assigning, so e.g. 12 gives a scale factor 1.2.
 * All combined, this means e.g. the first animation scales Mario's fist by {2.4, 1.6, 1.2, 1.0} on
 * successive frames.
 */
static s8 gMarioAttackScaleAnimation[3 * 6] = {
    10, 12, 16, 24, 10, 10, 10, 14, 20, 30, 10, 10, 10, 16, 20, 26, 26, 20,
};

#define gBodyStates (g_state->mgBodyStates)
//struct MarioBodyState gBodyStates[2]; // 2nd is never accessed in practice, most likely Luigi related
//struct GraphNodeObject gMirrorMario;  // copy of Mario's geo node for drawing mirror Mario

// This whole file is weirdly organized. It has to be the same file due
// to rodata boundaries and function aligns, which means the programmer
// treated this like a "misc" file for vaguely Mario related things
// (message NPC related things, the Mario head geo, and Mario geo
// functions)

/**
 * Geo node script that draws Mario's head on the title screen.
 */
// Gfx *geo_draw_mario_head_goddard(s32 callContext, struct GraphNode *node, Mat4 *c) {
//     Gfx *gfx = NULL;
//     s16 sfx = 0;
//     struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
//     UNUSED Mat4 *transform = c;
// 
//     if (callContext == GEO_CONTEXT_RENDER) {
//         if (gPlayerController->controllerData != NULL && !gWarpTransition.isActive) {
//             gd_copy_p1_contpad(gPlayer1Controller->controllerData);
//         }
//         gfx = (Gfx *) PHYSICAL_TO_VIRTUAL(gdm_gettestdl(asGenerated->parameter));
//         D_8032C6A0 = gd_vblank;
//         sfx = gd_sfx_to_play();
//         play_menu_sounds(sfx);
//     }
//     return gfx;
// }

static void toad_message_faded(void) {
//  if (gCurrentObject->oDistanceToMario > 700.0f) {
//      gCurrentObject->oToadMessageRecentlyTalked = FALSE;
//  }
//  if (!gCurrentObject->oToadMessageRecentlyTalked && gCurrentObject->oDistanceToMario < 600.0f) {
//      gCurrentObject->oToadMessageState = TOAD_MESSAGE_OPACIFYING;
//  }
}

static void toad_message_opaque(void) {
//  if (gCurrentObject->oDistanceToMario > 700.0f) {
//      gCurrentObject->oToadMessageState = TOAD_MESSAGE_FADING;
//  } else if (!gCurrentObject->oToadMessageRecentlyTalked) {
//      gCurrentObject->oInteractionSubtype = INT_SUBTYPE_NPC;
//      if (gCurrentObject->oInteractStatus & INT_STATUS_INTERACTED) {
//          gCurrentObject->oInteractStatus = 0;
//          gCurrentObject->oToadMessageState = TOAD_MESSAGE_TALKING;
//          play_toads_jingle();
//      }
//  }
}

static void toad_message_talking(void) {
//  if (cur_obj_update_dialog_with_cutscene(3, 1, CUTSCENE_DIALOG, gCurrentObject->oToadMessageDialogId)
//      != 0) {
//      gCurrentObject->oToadMessageRecentlyTalked = TRUE;
//      gCurrentObject->oToadMessageState = TOAD_MESSAGE_FADING;
//      switch (gCurrentObject->oToadMessageDialogId) {
//          case TOAD_STAR_1_DIALOG:
//              gCurrentObject->oToadMessageDialogId = TOAD_STAR_1_DIALOG_AFTER;
//              bhv_spawn_star_no_level_exit(0);
//              break;
//          case TOAD_STAR_2_DIALOG:
//              gCurrentObject->oToadMessageDialogId = TOAD_STAR_2_DIALOG_AFTER;
//              bhv_spawn_star_no_level_exit(1);
//              break;
//          case TOAD_STAR_3_DIALOG:
//              gCurrentObject->oToadMessageDialogId = TOAD_STAR_3_DIALOG_AFTER;
//              bhv_spawn_star_no_level_exit(2);
//              break;
//      }
//  }
}

static void toad_message_opacifying(void) {
    if ((gCurrentObject->oOpacity += 6) == 255) {
        gCurrentObject->oToadMessageState = TOAD_MESSAGE_OPAQUE;
    }
}

static void toad_message_fading(void) {
    if ((gCurrentObject->oOpacity -= 6) == 81) {
        gCurrentObject->oToadMessageState = TOAD_MESSAGE_FADED;
    }
}

void bhv_toad_message_loop(void) {
    if (gCurrentObject->header.gfx.node.flags & GRAPH_RENDER_ACTIVE) {
        gCurrentObject->oInteractionSubtype = 0;
        switch (gCurrentObject->oToadMessageState) {
            case TOAD_MESSAGE_FADED:
                toad_message_faded();
                break;
            case TOAD_MESSAGE_OPAQUE:
                toad_message_opaque();
                break;
            case TOAD_MESSAGE_OPACIFYING:
                toad_message_opacifying();
                break;
            case TOAD_MESSAGE_FADING:
                toad_message_fading();
                break;
            case TOAD_MESSAGE_TALKING:
                toad_message_talking();
                break;
        }
    }
}

void bhv_toad_message_init(void) {
//    s32 saveFlags = save_file_get_flags();
//    s32 starCount = save_file_get_total_star_count(gCurrSaveFileNum - 1, COURSE_MIN - 1, COURSE_MAX - 1);
//    s32 dialogId = (gCurrentObject->oBehParams >> 24) & 0xFF;
//    s32 enoughStars = TRUE;
//
//    switch (dialogId) {
//        case TOAD_STAR_1_DIALOG:
//            enoughStars = (starCount >= TOAD_STAR_1_REQUIREMENT);
//            if (saveFlags & SAVE_FLAG_COLLECTED_TOAD_STAR_1) {
//                dialogId = TOAD_STAR_1_DIALOG_AFTER;
//            }
//            break;
//        case TOAD_STAR_2_DIALOG:
//            enoughStars = (starCount >= TOAD_STAR_2_REQUIREMENT);
//            if (saveFlags & SAVE_FLAG_COLLECTED_TOAD_STAR_2) {
//                dialogId = TOAD_STAR_2_DIALOG_AFTER;
//            }
//            break;
//        case TOAD_STAR_3_DIALOG:
//            enoughStars = (starCount >= TOAD_STAR_3_REQUIREMENT);
//            if (saveFlags & SAVE_FLAG_COLLECTED_TOAD_STAR_3) {
//                dialogId = TOAD_STAR_3_DIALOG_AFTER;
//            }
//            break;
//    }
//    if (enoughStars) {
//        gCurrentObject->oToadMessageDialogId = dialogId;
//        gCurrentObject->oToadMessageRecentlyTalked = FALSE;
//        gCurrentObject->oToadMessageState = TOAD_MESSAGE_FADED;
//        gCurrentObject->oOpacity = 81;
//    } else {
//        obj_mark_for_deletion(gCurrentObject);
//    }
}

//static void star_door_unlock_spawn_particles(s16 angleOffset) {
//    struct Object *sparkleParticle = spawn_object(gCurrentObject, 0, bhvSparkleSpawn);
//
//    sparkleParticle->oPosX +=
//        100.0f * sins((gCurrentObject->oUnlockDoorStarTimer * 0x2800) + angleOffset);
//    sparkleParticle->oPosZ +=
//        100.0f * coss((gCurrentObject->oUnlockDoorStarTimer * 0x2800) + angleOffset);
//    // Particles are spawned lower each frame
//    sparkleParticle->oPosY -= gCurrentObject->oUnlockDoorStarTimer * 10.0f;
//}

void bhv_unlock_door_star_init(void) {
//    gCurrentObject->oUnlockDoorStarState = UNLOCK_DOOR_STAR_RISING;
//    gCurrentObject->oUnlockDoorStarTimer = 0;
//    gCurrentObject->oUnlockDoorStarYawVel = 0x1000;
//    gCurrentObject->oPosX += 30.0f * sins(gMarioState->faceAngle[1] - 0x4000);
//    gCurrentObject->oPosY += 160.0f;
//    gCurrentObject->oPosZ += 30.0f * coss(gMarioState->faceAngle[1] - 0x4000);
//    gCurrentObject->oMoveAngleYaw = 0x7800;
//    obj_scale(gCurrentObject, 0.5f);
}

void bhv_unlock_door_star_loop(void) {
//    UNUSED u8 unused1[4];
//    s16 prevYaw = gCurrentObject->oMoveAngleYaw;
//    UNUSED u8 unused2[4];
//
//    // Speed up the star every frame
//    if (gCurrentObject->oUnlockDoorStarYawVel < 0x2400) {
//        gCurrentObject->oUnlockDoorStarYawVel += 0x60;
//    }
//    switch (gCurrentObject->oUnlockDoorStarState) {
//        case UNLOCK_DOOR_STAR_RISING:
//            gCurrentObject->oPosY += 3.4f; // Raise the star up in the air
//            gCurrentObject->oMoveAngleYaw +=
//                gCurrentObject->oUnlockDoorStarYawVel; // Apply yaw velocity
//            obj_scale(gCurrentObject, gCurrentObject->oUnlockDoorStarTimer / 50.0f
//                                             + 0.5f); // Scale the star to be bigger
//            if (++gCurrentObject->oUnlockDoorStarTimer == 30) {
//                gCurrentObject->oUnlockDoorStarTimer = 0;
//                gCurrentObject->oUnlockDoorStarState++; // Sets state to UNLOCK_DOOR_STAR_WAITING
//            }
//            break;
//        case UNLOCK_DOOR_STAR_WAITING:
//            gCurrentObject->oMoveAngleYaw +=
//                gCurrentObject->oUnlockDoorStarYawVel; // Apply yaw velocity
//            if (++gCurrentObject->oUnlockDoorStarTimer == 30) {
//                play_sound(SOUND_MENU_STAR_SOUND,
//                           gCurrentObject->header.gfx.cameraToObject); // Play final sound
//                cur_obj_hide();                                            // Hide the object
//                gCurrentObject->oUnlockDoorStarTimer = 0;
//                gCurrentObject
//                    ->oUnlockDoorStarState++; // Sets state to UNLOCK_DOOR_STAR_SPAWNING_PARTICLES
//            }
//            break;
//        case UNLOCK_DOOR_STAR_SPAWNING_PARTICLES:
//            // Spawn two particles, opposite sides of the star.
//            star_door_unlock_spawn_particles(0);
//            star_door_unlock_spawn_particles(0x8000);
//            if (gCurrentObject->oUnlockDoorStarTimer++ == 20) {
//                gCurrentObject->oUnlockDoorStarTimer = 0;
//                gCurrentObject->oUnlockDoorStarState++; // Sets state to UNLOCK_DOOR_STAR_DONE
//            }
//            break;
//        case UNLOCK_DOOR_STAR_DONE: // The object stays loaded for an additional 50 frames so that the
//                                    // sound doesn't immediately stop.
//            if (gCurrentObject->oUnlockDoorStarTimer++ == 50) {
//                obj_mark_for_deletion(gCurrentObject);
//            }
//            break;
//    }
//    // Checks if the angle has cycled back to 0.
//    // This means that the code will execute when the star completes a full revolution.
//    if (prevYaw > (s16) gCurrentObject->oMoveAngleYaw) {
//        play_sound(
//            SOUND_GENERAL_SHORT_STAR,
//            gCurrentObject->header.gfx.cameraToObject); // Play a sound every time the star spins once
//    }
}

/**
 * Generate a display list that sets the correct blend mode and color for mirror Mario.
 */
static Gfx *make_gfx_mario_alpha(struct GraphNodeGenerated *node, s16 alpha) {
    Gfx *gfx;
    Gfx *gfxHead = NULL;

    if (alpha == 255) {
        node->fnNode.node.flags = (node->fnNode.node.flags & 0xFF) | (LAYER_OPAQUE << 8);
        gfxHead = alloc_display_list(2 * sizeof(*gfxHead));
        gfx = gfxHead;
    } else {
        node->fnNode.node.flags = (node->fnNode.node.flags & 0xFF) | (LAYER_TRANSPARENT << 8);
        gfxHead = alloc_display_list(3 * sizeof(*gfxHead));
        gfx = gfxHead;
        gDPSetAlphaCompare(gfx++, G_AC_DITHER);
    }
    gDPSetEnvColor(gfx++, 255, 255, 255, alpha);
    gSPEndDisplayList(gfx);
    return gfxHead;
}

/**
 * Sets the correct blend mode and color for mirror Mario.
 */
Gfx *geo_mirror_mario_set_alpha(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    UNUSED u8 unused1[4];
    Gfx *gfx = NULL;
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
    struct MarioBodyState *bodyState = &gBodyStates[asGenerated->parameter];
    s16 alpha;
    UNUSED u8 unused2[4];

    if (callContext == GEO_CONTEXT_RENDER) {
        alpha = (bodyState->modelState & 0x100) ? (bodyState->modelState & 0xFF) : 255;
        gfx = make_gfx_mario_alpha(asGenerated, alpha);
    }
    return gfx;
}

/**
 * Determines if Mario is standing or running for the level of detail of his model.
 * If Mario is standing still, he is always high poly. If he is running,
 * his level of detail depends on the distance to the camera.
 */
Gfx *geo_switch_mario_stand_run(s32 callContext, struct GraphNode *node, UNUSED Mat4 *mtx) {
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct MarioBodyState *bodyState = &gBodyStates[switchCase->numCases];

    if (callContext == GEO_CONTEXT_RENDER) {
        // assign result. 0 if moving, 1 if stationary.
        switchCase->selectedCase = ((bodyState->action & ACT_FLAG_STATIONARY) == 0);
    }
    return NULL;
}


Gfx *geo_switch_mario_level_of_detail(s32 callContext, struct GraphNode *node, UNUSED Mat4 *mtx) {
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct MarioBodyState *bodyState = &gBodyStates[switchCase->numCases];

    if (callContext == GEO_CONTEXT_RENDER) {
        if (!bodyState->lodsOverride)
        {
            if ((bodyState->action & ACT_FLAG_STATIONARY) == 0){
                switchCase->selectedCase = bodyState->lodState;
            }else
            {
                switchCase->selectedCase = 0;
            }
        }else{
            switchCase->selectedCase = bodyState->lodState;
        }
    }
    return NULL;
}


/**
 * Geo node script that makes Mario blink
 */
Gfx *geo_switch_mario_eyes(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct MarioBodyState *bodyState = &gBodyStates[switchCase->numCases];
    s16 blinkFrame;

    if (callContext == GEO_CONTEXT_RENDER) {
        if (bodyState->eyeState == 0) {
            blinkFrame = ((switchCase->numCases * 32 + gAreaUpdateCounter) >> 1) & 0x1F;
            if (blinkFrame < 7) {
                switchCase->selectedCase = gMarioBlinkAnimation[blinkFrame];
            } else {
                switchCase->selectedCase = 0;
            }
        } else {
            switchCase->selectedCase = bodyState->eyeState - 1;
        }
    }
    return NULL;
}

/**
 * Makes Mario's upper body tilt depending on the rotation stored in his bodyState
 */
Gfx *geo_mario_tilt_torso(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
    struct MarioBodyState *bodyState = &gBodyStates[asGenerated->parameter];
    s32 action = bodyState->action;

    if (callContext == GEO_CONTEXT_RENDER) {
        struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) node->next;

        if (action != ACT_BUTT_SLIDE && action != ACT_HOLD_BUTT_SLIDE && action != ACT_WALKING
            && action != ACT_RIDING_SHELL_GROUND) {
            vec3s_copy(bodyState->torsoAngle, gVec3sZero);
        }
        rotNode->rotation[0] = bodyState->torsoAngle[1];
        rotNode->rotation[1] = bodyState->torsoAngle[2];
        rotNode->rotation[2] = bodyState->torsoAngle[0];
    }
    return NULL;
}

/**
 * Makes Mario's head rotate with the camera angle when in C-up mode
 */
Gfx *geo_mario_head_rotation(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
    struct MarioBodyState *bodyState = &gBodyStates[asGenerated->parameter];
    s32 action = bodyState->action;

    if (callContext == GEO_CONTEXT_RENDER) {
        struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) node->next;
        struct Camera *camera = gCurGraphNodeCamera->config.camera;

      if (camera->mode == CAMERA_MODE_C_UP) {
          rotNode->rotation[0] = gPlayerCameraState->headRotation[1]; // PATCHED
          rotNode->rotation[2] = gPlayerCameraState->headRotation[0]; 
      } else if (action & ACT_FLAG_WATER_OR_TEXT) {
          rotNode->rotation[0] = bodyState->headAngle[1];
          rotNode->rotation[1] = bodyState->headAngle[2];
          rotNode->rotation[2] = bodyState->headAngle[0];
      } else {
            vec3s_set(bodyState->headAngle, 0, 0, 0);
            vec3s_set(rotNode->rotation, 0, 0, 0);
      }
    }
    return NULL;
}

/**
 * Switch between hand models.
 * Possible options are described in the MarioHandGSCId enum.
 */
Gfx *geo_switch_mario_hand(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct MarioBodyState *bodyState = &gBodyStates[0];

    if (callContext == GEO_CONTEXT_RENDER) {
        if (bodyState->handState == MARIO_HAND_FISTS) {
            // switch between fists (0) and open (1)
            switchCase->selectedCase = ((bodyState->action & ACT_FLAG_SWIMMING_OR_FLYING) != 0);
        } else {
            if (switchCase->numCases == 0) {
                switchCase->selectedCase =
                    (bodyState->handState < 5) ? bodyState->handState : MARIO_HAND_OPEN;
            } else {
                switchCase->selectedCase =
                    (bodyState->handState < 2) ? bodyState->handState : MARIO_HAND_FISTS;
            }
        }
    }
    return NULL;
}

/**
 * Increase Mario's hand / foot size when he punches / kicks.
 * Since animation geo nodes only support rotation, this scaling animation
 * was scripted separately. The node with this script should be placed before
 * a scaling node containing the hand / foot geo layout.
 * ! Since the animation gets updated in GEO_CONTEXT_RENDER, drawing Mario multiple times
 * (such as in the mirror room) results in a faster and desynced punch / kick animation.
 */
Gfx *geo_mario_hand_foot_scaler(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
    struct GraphNodeScale *scaleNode = (struct GraphNodeScale *) node->next;
    struct MarioBodyState *bodyState = &gBodyStates[0];

    if (callContext == GEO_CONTEXT_RENDER) {
        scaleNode->scale = 1.0f;
        if (asGenerated->parameter == bodyState->punchState >> 6) {
            if (g_state->msMarioAttackAnimCounter != gAreaUpdateCounter && (bodyState->punchState & 0x3F) > 0) {
                bodyState->punchState -= 1;
                g_state->msMarioAttackAnimCounter = gAreaUpdateCounter;
            }
            scaleNode->scale =
                gMarioAttackScaleAnimation[asGenerated->parameter * 6 + (bodyState->punchState & 0x3F)]
                / 10.0f;
        }
    }
    return NULL;
}

/**
 * Switch between normal cap, wing cap, vanish cap and metal cap.
 */
Gfx *geo_switch_mario_cap_effect(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct MarioBodyState *bodyState = &gBodyStates[switchCase->numCases];

    if (callContext == GEO_CONTEXT_RENDER) {
        switchCase->selectedCase = bodyState->modelState >> 8;
    }
    return NULL;
}

/**
 * Determine whether Mario's head is drawn with or without a cap on.
 * Also sets the visibility of the wing cap wings on or off.
 */
Gfx *geo_switch_mario_cap_on_off(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    struct GraphNode *next = node->next;
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct MarioBodyState *bodyState = &gBodyStates[switchCase->numCases];

    if (callContext == GEO_CONTEXT_RENDER) {
        switchCase->selectedCase = bodyState->capState & 1;
        while (next != node) {
            if (next->type == GRAPH_NODE_TYPE_TRANSLATION_ROTATION) {
                if (bodyState->capState & 2) {
                    next->flags |= GRAPH_RENDER_ACTIVE;
                } else {
                    next->flags &= ~GRAPH_RENDER_ACTIVE;
                }
            }
            next = next->next;
        }
    }
    return NULL;
}

/**
 * Geo node script that makes the wings on Mario's wing cap flap.
 * Should be placed before a rotation node.
 */
Gfx *geo_mario_rotate_wing_cap_wings(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
    s16 rotX;
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;

    if (callContext == GEO_CONTEXT_RENDER) {
        struct GraphNodeRotation *rotNode = (struct GraphNodeRotation *) node->next;

        if (!gBodyStates[asGenerated->parameter >> 1].wingFlutter) {
            rotX = (coss((gAreaUpdateCounter & 0xF) << 12) + 1.0f) * 4096.0f;
        } else {
            rotX = (coss((gAreaUpdateCounter & 7) << 13) + 1.0f) * 6144.0f;
        }
        if (!(asGenerated->parameter & 1)) {
            rotNode->rotation[0] = -rotX;
        } else {
            rotNode->rotation[0] = rotX;
        }
    }
    return NULL;
}

/**
 * Geo node that updates the held object node and the HOLP.
 */
Gfx *geo_switch_mario_hand_grab_pos(s32 callContext, struct GraphNode *b, Mat4 *mtx) {
    struct GraphNodeHeldObject *asHeldObj = (struct GraphNodeHeldObject *) b;
    Mat4 *curTransform = mtx;
    struct MarioState *marioState = gMarioState; // &gMarioState[asHeldObj->playerIndex]; // PATCH

    if (callContext == GEO_CONTEXT_RENDER) {
        asHeldObj->objNode = NULL;
        if (marioState->heldObj != NULL) {
            asHeldObj->objNode = marioState->heldObj;
            switch (marioState->marioBodyState->grabPos) {
                case GRAB_POS_LIGHT_OBJ:
                    if (marioState->action & ACT_FLAG_THROWING) {
                        vec3s_set(asHeldObj->translation, 50, 0, 0);
                    } else {
                        vec3s_set(asHeldObj->translation, 50, 0, 110);
                    }
                    break;
                case GRAB_POS_HEAVY_OBJ:
                    vec3s_set(asHeldObj->translation, 145, -173, 180);
                    break;
                case GRAB_POS_BOWSER:
                    vec3s_set(asHeldObj->translation, 80, -270, 1260);
                    break;
            }
        }
    } else if (callContext == GEO_CONTEXT_HELD_OBJ) {
        // ! The HOLP is set here, which is why it only updates when the held object is drawn.
        // This is why it won't update during a pause buffered hitstun or when the camera is very far
        // away.
        get_pos_from_transform_mtx(marioState->marioBodyState->heldObjLastPosition, *curTransform,
                                   *gCurGraphNodeCamera->matrixPtr);
    }
    return NULL;
}

// X position of the mirror
#define MIRROR_X 4331.53

/**
 * Geo node that creates a clone of Mario's geo node and updates it to becomes
 * a mirror image of the player.
 */
Gfx *geo_render_mirror_mario(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
//  f32 mirroredX;
//  struct Object *mario = gMarioState->marioObj; // PATCH  gMarioState[0].marioObj;

//  switch (callContext) {
//      case GEO_CONTEXT_CREATE:
//          init_graph_node_object(NULL, &gMirrorMario, NULL, gVec3fZero, gVec3sZero, gVec3fOne);
//          break;
//      case GEO_CONTEXT_AREA_LOAD:
//          geo_add_child(node, &gMirrorMario.node);
//          break;
//      case GEO_CONTEXT_AREA_UNLOAD:
//          geo_remove_child(&gMirrorMario.node);
//          break;
//      case GEO_CONTEXT_RENDER:
//          if (mario->header.gfx.pos[0] > 1700.0f) {
//              // TODO: Is this a geo layout copy or a graph node copy?
//              gMirrorMario.sharedChild = mario->header.gfx.sharedChild;
//              gMirrorMario.areaIndex = mario->header.gfx.areaIndex;
//              vec3s_copy(gMirrorMario.angle, mario->header.gfx.angle);
//              vec3f_copy(gMirrorMario.pos, mario->header.gfx.pos);
//              vec3f_copy(gMirrorMario.scale, mario->header.gfx.scale);

//              gMirrorMario.animInfo = mario->header.gfx.animInfo;
//              mirroredX = MIRROR_X - gMirrorMario.pos[0];
//              gMirrorMario.pos[0] = mirroredX + MIRROR_X;
//              gMirrorMario.angle[1] = -gMirrorMario.angle[1];
//              gMirrorMario.scale[0] *= -1.0f;
//              ((struct GraphNode *) &gMirrorMario)->flags |= 1;
//          } else {
//              ((struct GraphNode *) &gMirrorMario)->flags &= ~1;
//          }
//          break;
//  }
    return NULL;
}

/**
 * Since Mirror Mario has an x scale of -1, the mesh becomes inside out.
 * This node corrects that by changing the culling mode accordingly.
 */
Gfx *geo_mirror_mario_backface_culling(s32 callContext, struct GraphNode *node, UNUSED Mat4 *c) {
//  struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
//  Gfx *gfx = NULL;

//  if (callContext == GEO_CONTEXT_RENDER && gCurGraphNodeObject == &gMirrorMario) {
//      gfx = alloc_display_list(3 * sizeof(*gfx));

//      if (asGenerated->parameter == 0) {
//          gSPClearGeometryMode(&gfx[0], G_CULL_BACK);
//          gSPSetGeometryMode(&gfx[1], G_CULL_FRONT);
//          gSPEndDisplayList(&gfx[2]);
//      } else {
//          gSPClearGeometryMode(&gfx[0], G_CULL_FRONT);
//          gSPSetGeometryMode(&gfx[1], G_CULL_BACK);
//          gSPEndDisplayList(&gfx[2]);
//      }
//      asGenerated->fnNode.node.flags = (asGenerated->fnNode.node.flags & 0xFF) | (LAYER_OPAQUE << 8);
//  }
//  return gfx;
    return NULL;
}




//mia    miaaa

Gfx *mia_eyes_func(s32 callContext, struct GraphNode *node, UNUSED Mat4 *mtx) {
    
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct MarioState *m = gMarioState;
    struct MarioBodyState *bodyState = &gBodyStates[switchCase->numCases];
    s16 eyeState;
    s16 mouthState;
    s16 animID;
    s16 targetYawVel = (-(s16)(10.0 * gController.stickX));
    s16 targetPitchVel = (-(s16)(10.0 * gController.stickY));
    s16 blinkFrame;
    if (callContext == GEO_CONTEXT_RENDER) {
        animID = m->marioObj->header.gfx.animInfo.animID;
        mia_special_expressions(m, &eyeState, &mouthState);
        
        if (m->action & ACT_FLAG_SWIMMING_OR_FLYING) {
            if (targetYawVel > 0) {
                eyeState = MARIO_EYES_LOOK_RIGHT -1;
            }
            if (targetYawVel < 0) {
                eyeState = MARIO_EYES_LOOK_LEFT -1;
            }
            if (targetPitchVel > 0) {
                eyeState = MARIO_EYES_LOOK_UP-1;
            }
            if (targetPitchVel < 0) {
                eyeState = MARIO_EYES_LOOK_DOWN-1;
            }
        }
        
        if (eyeState < 0) {
            eyeState = (animID >= 0 && animID < 0xD1) ? gMiaAnimTableEyes[animID] : 0;
            if (bodyState->eyeState != 0) {
                eyeState = bodyState->eyeState - 1;
            }
        }
        if (eyeState == 0) {
            blinkFrame = ((switchCase->numCases * 32 + gAreaUpdateCounter) >> 1) & 0x1F;
            if (blinkFrame < 6) {
                switchCase->selectedCase = gMiaBlinkAnimation[blinkFrame];
            } else {
                switchCase->selectedCase = 0;
            }
        } else {
            switchCase->selectedCase = eyeState;
        }
        
    }
    
    return NULL;
}
Gfx *mia_extra_switch_func(s32 callContext, struct GraphNode *node, UNUSED Mat4 *mtx) {
    
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct MarioBodyState *bodyState = &gBodyStates[switchCase->numCases];
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
    struct MarioState *m = gMarioState;

    if (callContext == GEO_CONTEXT_RENDER) {
        if (asGenerated->parameter == 0){
            if (m->waterLevel > m->pos[1] + 70){ 
                switchCase->selectedCase = 1;
            }else{
                switchCase->selectedCase = 0;
            }
        
        }else{
        switchCase->selectedCase = 0;
        }
    }
    
    return NULL; 
}

Gfx *mia_mouth_func(s32 callContext, struct GraphNode *node, UNUSED Mat4 *mtx) {
    
    struct GraphNodeSwitchCase *switchCase = (struct GraphNodeSwitchCase *) node;
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
    struct Object *mario = gMarioState->marioObj;
    struct MarioState *m = gMarioState;
    
    if (callContext == GEO_CONTEXT_RENDER) {
            s16 animID = mario->header.gfx.animInfo.animID;
            s16 eyeState;
            s16 mouthState;

            switchCase->selectedCase = (animID >= 0 && animID < 0xD1)
                ? gMiaAnimTableMouth[animID]
                : MIA_MOUTH_NORMAL;
            mia_special_expressions(gMarioState, &eyeState, &mouthState);

            if ((m->action & ACT_FLAG_SWIMMING) || ((m->input & INPUT_IN_POISON_GAS) && !(m->action & ACT_GROUP_CUTSCENE))) {
                mouthState = MIA_MOUTH_WATER;
            }
        if (mouthState >= 0) {
            switchCase->selectedCase = mouthState;
        }
    }
    
    return NULL;
}



Gfx *geo_physbone_chain(s32 callContext, struct GraphNode *node, UNUSED Mat4 *mtx) {
    
    struct GraphNodeGenerated *asGenerated = (struct GraphNodeGenerated *) node;
    struct MarioState *m = gMarioState;
    struct MarioBodyState *bodyState = &gBodyStates[asGenerated->parameter];



    if (callContext == GEO_CONTEXT_RENDER) {
        struct GraphNodeTranslationRotation *rotNode = (struct GraphNodeTranslationRotation *) node->next;
        
        rotNode->rotation[0] += bodyState->headAngle[2];
        rotNode->rotation[2] += bodyState->headAngle[0];

        rotNode->translation[0] = 0;
        rotNode->translation[1] = 0;
        rotNode->translation[2] = 0;

        if (m->waterLevel > (m->pos[1] + 120)) {
                
            rotNode->rotation[2] = m->marioObj->header.gfx.angle[0] - DEGREES(90);

            if (DEGREES(rotNode->rotation[2]) > - 35) {
                rotNode->rotation[2] = DEGREES(-35);
            }
        }else if(m->area->camera->mode == CAMERA_MODE_C_UP) {
            rotNode->rotation[2] = m->statusForCamera->headRotation[0] *0.8;
        }else if (!(m->action & ACT_GROUP_CUTSCENE)){
            rotNode->rotation[2] = DEGREES((-(sins(m->marioObj->header.gfx.angle[1]) * m->vel[0]) + -(coss(m->marioObj->header.gfx.angle[1]) * m->vel[2])));
                
            if ((bodyState->action & ACT_FLAG_AIR) != 0){
                 rotNode->rotation[2] += DEGREES(CLAMP(m->vel[1], DEGREES(-10.0), DEGREES(90.0 - 25.0)));
            }
                rotNode->rotation[1] = DEGREES(180) + DEGREES((-(coss(m->marioObj->header.gfx.angle[1]) * m->vel[0]) + (sins(m->marioObj->header.gfx.angle[1]) * m->vel[2])));

            if (rotNode->rotation[2] > 0x461){
                rotNode->rotation[2] = 0x461;
            }
            if (rotNode->rotation[2] < -16000){
                rotNode->rotation[2] = -16000;
            }
        }else{
            rotNode->rotation[2] = 0;
        }
        //char text[14];
        //sprintf(text, "%#04d", rotNode->rotation[2]);
        //print_text(20, 100, text);
    }
    
    return NULL;
}