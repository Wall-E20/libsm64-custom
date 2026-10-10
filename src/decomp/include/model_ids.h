#ifndef MODEL_IDS_H
#define MODEL_IDS_H

/*
 * Shim for n64decomp's include/model_ids.h, which data/behavior_data.c includes.
 *
 * Upstream these are indices into gLoadedGraphNodes[], the table of loaded model
 * root nodes. This fork has no such table: the renderer owns the graph nodes and
 * actorMgr binds an actor's root node directly at spawn, so a MODEL_* id has no
 * meaning at runtime.
 *
 * SET_MODEL() records the id in Object::oModelID for the host to read, and
 * resolves nothing. The values below are therefore arbitrary but stable, chosen to
 * match upstream's numbering where it is known so logs and docs line up.
 */

typedef enum {
    MODEL_NONE = 0,
    MODEL_GOOMBA = 1,
    MODEL_COIN = 2,
    MODEL_STAR = 3,
} ModelID;

#endif // MODEL_IDS_H
