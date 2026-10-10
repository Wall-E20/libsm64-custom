#pragma once
#include "decomp/include/types.h"

u32 fake_determine_knockback_action(struct MarioState *m, s32 damage,float xSrc,float ySrc,float zSrc);
s16 fake_mario_obj_angle_to_object(struct MarioState *m, float xSrc,float zSrc);
uint32_t fake_damage_knock_back(struct MarioState *m, uint32_t damage,uint32_t interactionSubtype,float xSrc,float ySrc,float zSrc);
u32 fake_interact_hit_from_below(struct MarioState *m, float x, float y, float z, float hitboxHeight);
u32 fake_interact_bounce_top(struct MarioState *m, float x, float y, float z, float hitboxHeight);

/* Defined in fake_interaction.c but previously undeclared, so callers could not
 * use them. Needed by the sInteractionHandlers adapters in
 * game/interaction.c, which are the first real (non-noop) handlers this fork
 * has wired up. */
u32 fake_determine_interaction(struct MarioState *m, float x, float y, float z);
void fake_bounce_back_from_attack(struct MarioState *m, u32 interaction);
void fake_bounce_off_object(struct MarioState *m, float x, float y, float z, float hitboxHeight, f32 velY);