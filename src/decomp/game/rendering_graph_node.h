#ifndef RENDERING_GRAPH_NODE_H
#define RENDERING_GRAPH_NODE_H

#include "../include/PR/ultratypes.h"

#include "../engine/graph_node.h"

extern struct GraphNodeRoot *gCurGraphNodeRoot;
extern struct GraphNodeMasterList *gCurGraphNodeMasterList;
extern struct GraphNodePerspective *gCurGraphNodeCamFrustum;
extern struct GraphNodeCamera *gCurGraphNodeCamera;
extern struct GraphNodeObject *gCurGraphNodeObject;
extern struct GraphNodeHeldObject *gCurGraphNodeHeldObject;

/* The actor currently being walked, for layouts with no GraphNodeObject.
 *
 * Upstream's geo callbacks (geo_switch_anim_state and friends) read the current
 * object via gCurGraphNodeObject, which the GraphNodeObject case sets and clears
 * around its child walk. Actor layouts (goomba, coin, star, ...) have no such node,
 * so gCurGraphNodeObject is NULL for them and every GEO_SWITCH_CASE would keep
 * selectedCase at 0. sm64_actor_tick() sets this for the duration of its walk so
 * those callbacks can see the actor, and clears it afterwards.
 *
 * NULL whenever no actor is being drawn, including during Mario's own walk. */
extern struct Object *gCurGraphNodeActor;

// after processing an object, the type is reset to this
#define ANIM_TYPE_NONE                  0

// Not all parts have full animation: to save space, some animations only
// have xz, y, or no translation at all. All animations have rotations though
#define ANIM_TYPE_TRANSLATION           1
#define ANIM_TYPE_VERTICAL_TRANSLATION  2
#define ANIM_TYPE_LATERAL_TRANSLATION   3
#define ANIM_TYPE_NO_TRANSLATION        4

// Every animation includes rotation, after processing any of the above
// translation types the type is set to this
#define ANIM_TYPE_ROTATION              5

void geo_process_node_and_siblings(struct GraphNode *firstNode);
void geo_process_root_hack_single_node_obj(struct Object *obj, struct GraphNode *node);
//void geo_process_root(struct GraphNodeRoot *node, Vp *b, Vp *c, s32 clearColor);
void geo_process_root_hack_single_node(struct GraphNode *node);

#endif // RENDERING_GRAPH_NODE_H
