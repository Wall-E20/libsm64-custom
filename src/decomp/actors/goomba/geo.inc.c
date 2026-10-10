#include "../../../decomp/include/sm64.h"
#include "../../../decomp/include/types.h"
#include "../../../decomp/include/geo_commands.h"
#include "../../../decomp/game/rendering_graph_node.h"
#include "../../../decomp/shim.h"
#include "../../../decomp/game/object_stuff.h"
#include "../../../decomp/game/behavior_actions.h"
#include "model.inc.h"

/* Shadow node type. Not defined by gbi.h in this fork; Mario declares its own
 * equivalent locally. Upstream SM64 has SHADOW_CIRCLE_4_VERTS == 0. */
#define SHADOW_CIRCLE_4_VERTS 0

// 0x0F0006E4
const GeoLayout goomba_geo[] = {
    GEO_SHADOW(SHADOW_CIRCLE_4_VERTS, 0x96, 100),
    GEO_OPEN_NODE(),
        GEO_SCALE(0x00, 16384),
        GEO_OPEN_NODE(),
            GEO_ANIMATED_PART(LAYER_OPAQUE, 0, 0, 0, goomba_seg8_dl_0801D760),
            GEO_OPEN_NODE(),
                GEO_ANIMATED_PART(LAYER_OPAQUE, 0, 0, 0, NULL),
                GEO_OPEN_NODE(),
                    GEO_BILLBOARD(),
                    GEO_OPEN_NODE(),
                        GEO_DISPLAY_LIST(LAYER_ALPHA, goomba_seg8_dl_0801B690),
                    GEO_CLOSE_NODE(),
                GEO_CLOSE_NODE(),
                GEO_OPEN_NODE(),
                    GEO_SWITCH_CASE(2, geo_switch_anim_state),
                    GEO_OPEN_NODE(),
                        GEO_ANIMATED_PART(LAYER_OPAQUE, 48, 0, 0, goomba_seg8_dl_0801B5C8),
                        GEO_ANIMATED_PART(LAYER_OPAQUE, 48, 0, 0, goomba_seg8_dl_0801B5F0),
                    GEO_CLOSE_NODE(),
                    GEO_ANIMATED_PART(LAYER_OPAQUE, -60, -16, 45, NULL),
                    GEO_OPEN_NODE(),
                        GEO_ANIMATED_PART(LAYER_OPAQUE, 0, 0, 0, goomba_seg8_dl_0801CE20),
                    GEO_CLOSE_NODE(),
                    GEO_ANIMATED_PART(LAYER_OPAQUE, -60, -16, -45, NULL),
                    GEO_OPEN_NODE(),
                        GEO_ANIMATED_PART(LAYER_OPAQUE, 0, 0, 0, goomba_seg8_dl_0801CF78),
                    GEO_CLOSE_NODE(),
                GEO_CLOSE_NODE(),
            GEO_CLOSE_NODE(),
        GEO_CLOSE_NODE(),
    GEO_CLOSE_NODE(),
    GEO_END(),
};

/* GeoLayout is a scalar typedef, so a void* alias is needed to hand this to
 * process_geo_layout(), same as mario_geo_ptr. */
void *goomba_geo_ptr = (void *)goomba_geo;
