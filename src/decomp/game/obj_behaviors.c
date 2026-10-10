
#include "../shim.h"
#include "../global_state.h"
#include "../include/types.h"
#include "../include/sm64.h"
#include "../include/object_fields.h"
#include "../include/behavior_data.h"
#include "../../object_constants_min.h"
#include "../../object_helpers_min.h"
#include "object_stuff.h"
#include "interaction.h"
#include "mario.h"
#include "mario_misc.h"
#include "behavior_actions.h"
#include "rendering_graph_node.h"
#include "../engine/math_util.h"
#include "../engine/surface_collision.h"
#include "../engine/geo_layout.h"
#include "../audio/external.h"
#include "../include/seq_ids.h"
#include "../actors/goomba/model.inc.h"

#define o (gCurrentObject)


#define MODEL_GOOMBA 0

struct Object *spawn_object_relative(UNUSED u32 modelId, UNUSED s16 relX, UNUSED s16 relY,
                                     UNUSED s16 relZ, UNUSED struct Object *parent,
                                     UNUSED s16 model, UNUSED const BehaviorScript *behavior) {
    return NULL;
}
#include "../actors/goomba/behavior.inc.c"
