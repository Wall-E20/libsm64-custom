#include "actorMgr.h"

#include <stdlib.h>
#include <string.h>

#include "libsm64.h"
#include "obj_pool.h"
#include "gfx_adapter.h"
#include "debug_print.h"
#include "decomp/shim.h"
#include "decomp/memory.h"
#include "decomp/global_state.h"
#include "decomp/include/types.h"
#include "decomp/include/sm64.h"
#include "decomp/include/object_fields.h"
#include "decomp/engine/math_util.h"
#include "decomp/engine/geo_layout.h"
#include "decomp/engine/graph_node.h"
#include "decomp/game/object_stuff.h"
#include "decomp/game/object_list_processor.h"
#include "decomp/game/rendering_graph_node.h"
#include "decomp/actors/goomba/geo.inc.h"
#include "object_constants_min.h"
#include "object_helpers_min.h"
#include "object_collision_min.h"
#include "decomp/actors/goomba/model.inc.h"
#include "decomp/include/behavior_data.h"        //
#include "decomp/game/behavior_script.h"       //sm64_behavior_run_script


struct ActorInstance
{
    struct GlobalState *globalState;
    struct Object *object;
    const BehaviorScript *behavior;
    struct ActorType *type;
};

static struct ObjPool s_actor_pool = { 0, 0 };
static struct AllocOnlyPool *s_actor_geo_pool = NULL;


struct ActorType
{
    const char *name;
    const BehaviorScript *bhv;
    void *geo;
    struct GraphNode *node;
    struct GraphNodeMasterList *masterList;
    int hasInitRun;
};


#define SM64_MAX_ACTOR_TYPES 8
static struct ActorType s_actor_types[SM64_MAX_ACTOR_TYPES];
static int s_actor_type_count = 0;

static struct ActorType *s_default_type = NULL;

static struct ActorType *find_actor_type( const BehaviorScript *bhv )
{
    for( int i = 0; i < s_actor_type_count; ++i )
    {
        if( s_actor_types[i].bhv == bhv ) return &s_actor_types[i];
    }
    return NULL;
}

static struct ActorInstance *get_actor( int32_t actorId )
{
    if( actorId < 0 || actorId >= (int32_t)s_actor_pool.size ) return NULL;
    return (struct ActorInstance *)s_actor_pool.objects[ actorId ];
}


static struct Object *find_mario_object( void )
{
    for( int32_t i = 0; i < s_mario_instance_pool.size; ++i )
    {
        if( s_mario_instance_pool.objects[ i ] == NULL ) continue;
        struct GlobalState *gs =
            ( *( (struct GlobalState **)s_mario_instance_pool.objects[ i ] ) );
        if( gs != NULL && gs->mgMarioObject != NULL ) return gs->mgMarioObject;
    }
    return NULL;
}


int putObjectInActorPool(struct Object* obj){
    int id = obj_pool_alloc_index( &s_actor_pool, sizeof( struct GlobalState ));
    s_actor_pool.objects[id] = global_state_create();
    ((struct GlobalState*)s_actor_pool.objects[ id ])->mgCurrentObject=obj;
    return id;

}

struct ObjPool* getActorPool(){
    return &s_actor_pool;
}

struct Object *getActor( int32_t actorId )
{
    struct ActorInstance *actor = get_actor( actorId );
    return actor != NULL ? actor->object : NULL;
}

{
    struct ActorInstance *actor = get_actor( actorId );
    if( actor == NULL || actor->object == NULL ) return (int)OBJ_LIST_DEFAULT;
    return (int)actor->object->oObjList;
}


static struct ActorType *register_actor_type( const char *name,
                                              const BehaviorScript *bhv,
                                              void *geo )
{
    if( bhv == NULL || geo == NULL )
    {
        DEBUG_PRINT("actor type '%s' has no behavior or geo; skipped", name);
        return NULL;
    }

    if( find_actor_type( bhv ) != NULL ) return find_actor_type( bhv );

    if( s_actor_type_count >= SM64_MAX_ACTOR_TYPES )
    {
        DEBUG_PRINT("actor type table full; '%s' not registered "
                    "(raise SM64_MAX_ACTOR_TYPES)", name);
        return NULL;
    }

    struct ActorType *type = &s_actor_types[s_actor_type_count++];
    type->name = name;
    type->bhv = bhv;
    type->geo = geo;
    type->node = NULL;
    type->masterList = NULL;
    type->hasInitRun = FALSE;
    return type;
}


static void ensure_actor_type_init( struct ActorType *type )
{
    if( type->hasInitRun ) return;
    type->hasInitRun = TRUE;

    if( s_actor_geo_pool == NULL ) s_actor_geo_pool = alloc_only_pool_init();

    type->masterList = init_graph_node_master_list( s_actor_geo_pool, NULL, TRUE );
    type->node = process_geo_layout( s_actor_geo_pool, type->geo );

    if( type->node != NULL && type->masterList != NULL )
    {
        geo_add_child( &(type->masterList->node), type->node );
    }
    else
    {
        DEBUG_PRINT("actor type '%s': geo layout produced no node", type->name);
    }
}

SM64_LIB_FN void sm64_actor_init( const uint8_t *rom UNUSED )
{


    if( s_default_type != NULL ) return;

    s_actor_geo_pool = alloc_only_pool_init();

    s_default_type = register_actor_type( "goomba", bhvGoomba, goomba_geo_ptr );
}


SM64_LIB_FN void sm64_actor_set_behavior( int32_t actorType UNUSED,
                                          void (*behavior)( struct Object *obj ) UNUSED )
{

    (void)behavior;
}

SM64_LIB_FN int32_t sm64_actor_spawn_bhv( const BehaviorScript *bhv, float x, float y, float z )
{
    if( s_default_type == NULL )
    {
        DEBUG_PRINT("sm64_actor_spawn_bhv before sm64_actor_init()");
        return -1;
    }

    struct ActorType *type = find_actor_type( bhv );
    if( type == NULL )
    {
        DEBUG_PRINT("sm64_actor_spawn_bhv: behavior not registered");
        return -1;
    }

    ensure_actor_type_init( type );
    if( type->node == NULL ) return -1;

    int32_t id = obj_pool_alloc_index( &s_actor_pool, sizeof( struct ActorInstance ) );
    struct ActorInstance *actor = (struct ActorInstance *)s_actor_pool.objects[ id ];
    if( !actor ) return -1;

    actor->type = type;
    actor->globalState = global_state_create();

    struct Object *mario = find_mario_object();

    global_state_bind( actor->globalState );
    actor->globalState->mgMarioObject = mario;
    actor->behavior = type->bhv;

    actor->object = hack_allocate_mario();
    if( !actor->object )
    {        global_state_delete( actor->globalState );
        obj_pool_free_index( &s_actor_pool, id );
        return -1;
    }

    struct Object *obj = actor->object;
    gCurrentObject = obj;

    obj->header.gfx.cameraToObject[0] = obj->header.gfx.pos[0];
    obj->header.gfx.cameraToObject[1] = obj->header.gfx.pos[1];
    obj->header.gfx.cameraToObject[2] = obj->header.gfx.pos[2];

    obj->oPosX = x;
    obj->oPosY = y;
    obj->oPosZ = z;
    obj->oVelX = obj->oVelY = obj->oVelZ = 0.0f;
    obj->oAnimState = 0;
    obj->header.gfx.pos[0] = x;
    obj->header.gfx.pos[1] = y;
    obj->header.gfx.pos[2] = z;
    vec3s_set( obj->header.gfx.angle, 0, 0, 0 );

    obj->parentObj = obj;

    sm64_behavior_start_script( obj, type->bhv );

    if( mario != NULL ) sm64_behavior_run_script( obj );

    return id;
}

SM64_LIB_FN int32_t sm64_actor_spawn( int32_t actorType, float x, float y, float z )
{
    const BehaviorScript *bhv;

    switch( actorType )
    {
        case SM64_ACTOR_GOOMBA: bhv = bhvGoomba; break;
        default:
            DEBUG_PRINT("unknown actor type %d", actorType);
            return -1;
    }

    return sm64_actor_spawn_bhv( bhv, x, y, z );
}

SM64_LIB_FN void sm64_actor_tick( int32_t actorId, struct SM64ActorState *outState,
                                  struct SM64MarioGeometryBuffers *outBuffers )
{
    struct ActorInstance *actor = get_actor( actorId );
    if( !actor || !actor->object ) return;

    struct Object *mario = find_mario_object();

    global_state_bind( actor->globalState );

    struct Object *obj = actor->object;
    gCurrentObject = obj;
    actor->globalState->mgMarioObject = mario;

    gCurrentObject = obj;

    if( actor->behavior != NULL && mario != NULL ) sm64_behavior_run_script( obj );

    if( obj->activeFlags == ACTIVE_FLAG_DEACTIVATED )
    {
        if( outBuffers != NULL ) outBuffers->numTrianglesUsed = 0;
        if( outState != NULL ) memset( outState, 0, sizeof( *outState ) );

        global_state_delete( actor->globalState );
        obj_pool_free_index( &s_actor_pool, actorId );
        free( obj );
        return;
    }

    if (obj->oTimer < 0x3FFFFFFF) {
        obj->oTimer++;
    }
    if (obj->oAction != obj->oPrevAction) {
        (void) (obj->oTimer = 0, obj->oSubAction = 0,
        obj->oPrevAction = obj->oAction);
    }

    if( obj->oFlags & OBJ_FLAG_SET_FACE_ANGLE_TO_MOVE_ANGLE )
    {
        obj_set_face_angle_to_move_angle( obj );
    }
    if( obj->oFlags & OBJ_FLAG_SET_FACE_YAW_TO_MOVE_YAW )
    {
        obj->oFaceAngleYaw = obj->oMoveAngleYaw;
    }
    if( obj->oFlags & OBJ_FLAG_MOVE_XZ_USING_FVEL )
    {
        cur_obj_move_xz_using_fvel_and_yaw();
    }
    if( obj->oFlags & OBJ_FLAG_MOVE_Y_WITH_TERMINAL_VEL )
    {
        cur_obj_move_y_with_terminal_vel();
    }
    if( obj->oFlags & OBJ_FLAG_UPDATE_GFX_POS_AND_ANGLE )
    {
        obj_update_gfx_pos_and_angle( obj );
    }
    // Calculate the distance from the object to Mario.
    if (obj->oFlags & OBJ_FLAG_COMPUTE_DIST_TO_MARIO) {
        obj->oDistanceToMario = dist_between_objects(obj, gMarioObject);
    } else {
        obj->oDistanceToMario = 0.0f;
    }

    // Calculate the angle from the object to Mario.
    if (obj->oFlags & OBJ_FLAG_COMPUTE_ANGLE_TO_MARIO) {
        obj->oAngleToMario = obj_angle_to_object(obj, gMarioObject);
    }
    
    detect_object_collisions();

    if( outBuffers != NULL )
    {
        gfx_adapter_bind_output_buffers( outBuffers );

        struct ActorType *type = actor->type;
        if( type != NULL && type->masterList != NULL )
        {
            ensure_actor_type_init( type );

            struct Object *prevActor = gCurGraphNodeActor;
            gCurGraphNodeActor = obj;
            geo_process_root_hack_single_node_obj( obj, (struct GraphNode *)type->masterList );
            gCurGraphNodeActor = prevActor;
        }
        gAreaUpdateCounter++;
    }

    if( outState != NULL )
    {
        outState->position[0] = obj->oPosX;
        outState->position[1] = obj->oPosY;
        outState->position[2] = obj->oPosZ;
        outState->velocity[0] = obj->oVelX;
        outState->velocity[1] = obj->oVelY;
        outState->velocity[2] = obj->oVelZ;
        outState->rotation[0] = obj->header.gfx.angle[0];
        outState->rotation[1] = obj->header.gfx.angle[1];
        outState->rotation[2] = obj->header.gfx.angle[2];
        outState->scale[0] = obj->header.gfx.scale[0];
        outState->scale[1] = obj->header.gfx.scale[1];
        outState->scale[2] = obj->header.gfx.scale[2];
        outState->animState = obj->oAnimState;
    }
}

SM64_LIB_FN void sm64_actor_delete( int32_t actorId )
{
    struct ActorInstance *actor = get_actor( actorId );
    if( !actor ) return;

    if( actor->object ) free( actor->object );
    if( actor->globalState )
    {
        global_state_bind( actor->globalState );
        global_state_delete( actor->globalState );
    }

    obj_pool_free_index( &s_actor_pool, actorId );
}