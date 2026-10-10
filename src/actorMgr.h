#ifndef LIB_SM64_ACTOR_H
#define LIB_SM64_ACTOR_H

#include <stdint.h>

#include "libsm64.h"   ///SM64MarioGeometryBuffers
#include "obj_pool.h"
#include "decomp/shim.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    SM64_ACTOR_GOOMBA = 0,
    SM64_ACTOR_COIN = 1,
    SM64_ACTOR_STAR = 2,
};


//Per-actor state
 
struct SM64ActorState
{
    float position[3];
    float velocity[3];
    float rotation[3];
    float scale[3];
    uint32_t animState;
};


int putObjectInActorPool(struct Object* obj);

struct ObjPool* getActorPool();

extern SM64_LIB_FN void sm64_actor_init(const uint8_t *rom);


extern SM64_LIB_FN int32_t sm64_actor_spawn(int32_t actorType, float x, float y, float z);


extern SM64_LIB_FN int32_t sm64_actor_spawn_bhv(const BehaviorScript *bhv, float x, float y, float z);

struct Object;

extern SM64_LIB_FN void sm64_actor_set_behavior(int32_t actorType,
                                                 void (*behavior)(struct Object *obj));

struct ObjPool;

struct ObjPool *getActorPool(void);
struct Object *getActor(int32_t actorId);
int getActorObjList(int32_t actorId);

extern struct ObjPool s_mario_instance_pool;

extern SM64_LIB_FN void sm64_actor_tick(int32_t actorId, struct SM64ActorState *outState, struct SM64MarioGeometryBuffers *outBuffers);

extern SM64_LIB_FN void sm64_actor_delete(int32_t actorId);

#ifdef __cplusplus
}
#endif

#endif//LIB_SM64_ACTOR_H