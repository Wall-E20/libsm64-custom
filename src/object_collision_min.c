

#include "decomp/shim.h"
#include "decomp/global_state.h"
#include "decomp/include/types.h"
#include "decomp/include/sm64.h"
#include "decomp/include/object_fields.h"
#include "object_constants_min.h"
#include "decomp/game/interaction.h"
#include "object_collision_min.h"
#include "obj_pool.h"
#include "actorMgr.h"
#include "../include/PR/ultratypes.h"
#include "decomp/game/mario.h"
#include "decomp/game/object_list_processor.h"
#include "../../actorMgr.h"



s32 detect_object_hitbox_overlap(struct Object *a, struct Object *b)
{
    f32 aBase = a->oPosY - a->hitboxDownOffset;
    f32 bBase = b->oPosY - b->hitboxDownOffset;
    f32 dx = a->oPosX - b->oPosX;
    UNUSED f32 dy = aBase - bBase;
    f32 dz = a->oPosZ - b->oPosZ;
    f32 collisionRadius = a->hitboxRadius + b->hitboxRadius;
    f32 distance = sqrtf(dx * dx + dz * dz);

    if (collisionRadius > distance) {
        f32 aTop = a->hitboxHeight + aBase;
        f32 bTop = b->hitboxHeight + bBase;

        if (aBase > bTop) {
            return 0;
        }
        if (aTop < bBase) {
            return 0;
        }
        if (a->numCollidedObjs >= 4) {
            return 0;
        }
        if (b->numCollidedObjs >= 4) {
            return 0;
        }
        a->collidedObjs[a->numCollidedObjs] = b;
        b->collidedObjs[b->numCollidedObjs] = a;
        a->collidedObjInteractTypes |= b->oInteractType;
        b->collidedObjInteractTypes |= a->oInteractType;
        a->numCollidedObjs++;
        b->numCollidedObjs++;
        return 1;
    }

    return 0;
}


s32 detect_object_hurtbox_overlap(struct Object *a, struct Object *b)
{
    f32 aBase = a->oPosY - a->hitboxDownOffset;
    f32 bBase = b->oPosY - b->hitboxDownOffset;
    f32 dx = a->oPosX - b->oPosX;
    UNUSED f32 dy = aBase - bBase;
    f32 dz = a->oPosZ - b->oPosZ;
    f32 collisionRadius = a->hurtboxRadius + b->hurtboxRadius;
    f32 distance = sqrtf(dx * dx + dz * dz);

    if (a == gMarioObject) {
        b->oInteractionSubtype |= INT_SUBTYPE_DELAY_INVINCIBILITY;
    }

    if (collisionRadius > distance) {
        f32 aTop = a->hitboxHeight + aBase;
        f32 bTop = b->hurtboxHeight + bBase;

        if (aBase > bTop) {
            return 0;
        }
        if (aTop < bBase) {
            return 0;
        }
        if (a == gMarioObject) {
            b->oInteractionSubtype &= ~INT_SUBTYPE_DELAY_INVINCIBILITY;
        }
        return 1;
    }

    return 0;
}


void check_collision_in_list(struct Object *a, enum ObjectList objType) {
    if (a->oIntangibleTimer == 0) {
        struct ObjPool* pool = getActorPool();
        for (int i = 0; i < pool->size; i++) {
            struct Object* b = getActor(i);
            if (b==NULL || b==a) continue;
            enum ObjectList actorType = (enum ObjectList)getActorObjList(i);
            if (actorType == objType || objType == -1) {
                if (b->oIntangibleTimer == 0) {
                    if (detect_object_hitbox_overlap(a, b) && b->hurtboxRadius != 0.0f) {
                        detect_object_hurtbox_overlap(a, b);
                    }
                }
            }
        }
    }
}


s32 check_object_collision_pair(struct Object *a, struct Object *b)
{
    if (a->oIntangibleTimer == 0 && b->oIntangibleTimer == 0) {
        if (detect_object_hitbox_overlap(a, b) && b->hurtboxRadius != 0.0f) {
            detect_object_hurtbox_overlap(a, b);
        }
        return 1;
    }
    return 0;
}

void clear_object_collision(enum ObjectList objType)
{
    struct ObjPool *pool = getActorPool();

    for (int i = 0; i < pool->size; i++) {
        struct Object *obj = getActor(i);
        if (obj == NULL) continue;
        if (objType != -1 && getActorObjList(i) != objType) continue;

        obj->numCollidedObjs = 0;
        obj->collidedObjInteractTypes = 0;
        if (obj->oIntangibleTimer > 0) {
            obj->oIntangibleTimer--;
        }
    }
}



void check_player_object_collision(void) { // TODO: rewrite this to handle new "object list"
    for (int i = 0;i<s_mario_instance_pool.size;i++) {
        if (s_mario_instance_pool.objects[ i ]==NULL)
            continue;
        struct Object* b = (*((struct GlobalState **)s_mario_instance_pool.objects[ i ]))->mgMarioObject;
        if (b==NULL) continue;
        //if (((b->behavior[0] >> 16) & 0xFFFF)==OBJ_LIST_PLAYER){
            //
            check_collision_in_list(b, OBJ_LIST_POLELIKE);
            check_collision_in_list(b, OBJ_LIST_LEVEL);
            check_collision_in_list(b, OBJ_LIST_GENACTOR);
            check_collision_in_list(b, OBJ_LIST_PUSHABLE);
            check_collision_in_list(b, OBJ_LIST_SURFACE);
            check_collision_in_list(b, OBJ_LIST_DESTRUCTIVE);
            //
        //}
    }
}

void check_pushable_object_collision(void) {
    struct ObjPool* pool = getActorPool();
    for (int i = 0; i < pool->size; i++) {
        struct Object* b = getActor(i);
        if (b==NULL) continue;
        enum ObjectList actorType = (enum ObjectList)getActorObjList(i);
        if (actorType == OBJ_LIST_PUSHABLE) {
            //
            check_collision_in_list(b, -1);
            //
        }
    }
}

void check_destructive_object_collision(void) {
    // struct Object *sp1C = (struct Object *) &gObjectLists[OBJ_LIST_DESTRUCTIVE];
    // struct Object *sp18 = (struct Object *) sp1C->header.next;

    // while (sp18 != sp1C) {
    //     if (sp18->oDistanceToMario < 2000.0f && !(sp18->activeFlags & ACTIVE_FLAG_UNK9)) {
    //         check_collision_in_list(sp18, OBJ_LIST_DESTRUCTIVE);
    //         check_collision_in_list(sp18, OBJ_LIST_GENACTOR);
    //         check_collision_in_list(sp18, OBJ_LIST_PUSHABLE);
    //         check_collision_in_list(sp18, OBJ_LIST_SURFACE);
    //     }
    //     sp18 = (struct Object *) sp18->header.next;
    // }
}


static void clear_mario_collision(void)
{
    for (int i = 0; i < s_mario_instance_pool.size; i++) {
        if (s_mario_instance_pool.objects[i] == NULL) continue;
        struct Object *mario = (*((struct GlobalState **)s_mario_instance_pool.objects[i]))->mgMarioObject;
        if (mario == NULL) continue;

        mario->numCollidedObjs = 0;
        mario->collidedObjInteractTypes = 0;
        if (mario->oIntangibleTimer > 0) {
            mario->oIntangibleTimer--;
        }
    }
}

void detect_object_collisions(void) {
    clear_object_collision(OBJ_LIST_POLELIKE);
    clear_object_collision(-1);
    clear_object_collision(OBJ_LIST_PUSHABLE);
    clear_object_collision(OBJ_LIST_GENACTOR);
    clear_object_collision(OBJ_LIST_LEVEL);
    clear_object_collision(OBJ_LIST_SURFACE);
    clear_object_collision(OBJ_LIST_DESTRUCTIVE);
    clear_mario_collision();
    check_player_object_collision();
    check_destructive_object_collision();
    check_pushable_object_collision();
}