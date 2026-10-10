#pragma once

#include "decomp/include/types.h"

#ifdef __cplusplus
extern "C" {
#endif



f32 absf(f32 x);
s32 obj_update_standard_actions(f32 scale);
void cur_obj_scale(f32 scale);
void cur_obj_update_floor_and_walls(void);
void cur_obj_update_floor_height(void);
void cur_obj_init_animation_with_accel_and_sound(s32 animIndex, f32 accel);
void cur_obj_move_standard(s16 steepSlopeAngleDegrees);
s32 cur_obj_rotate_yaw_toward(s16 target, s16 increment);
void cur_obj_move_y(f32 gravity, f32 bounciness, f32 buoyancy);
s32 cur_obj_resolve_wall_collisions(void);
struct SM64SurfaceCollisionData *cur_obj_update_floor_height_and_get_floor(void);
void cur_obj_if_hit_wall_bounce_away(void);
s8 is_point_within_radius_of_mario(f32 x, f32 y, f32 z, s32 dist);
void set_object_visibility(struct Object *obj, s32 dist);
s32 obj_handle_attacks(struct ObjectHitbox *hitbox, s32 attackedMarioAction, u8 *attackHandlers);
void obj_die_if_health_non_positive(void);
void obj_set_knockback_action(s32 attackType);
void obj_set_squished_action(void);
void obj_set_speed_to_zero(void);
void obj_set_hitbox(struct Object *obj, struct ObjectHitbox *hitbox);
void obj_mark_for_deletion(struct Object *obj);
void obj_spawn_loot_coins(struct Object *obj, s32 numCoins, f32 baseVelY,
                          const BehaviorScript *coinBehavior, s16 posJitter, s16 model);
void obj_spawn_loot_yellow_coins(struct Object *obj, s32 numCoins, f32 baseVelY);
struct Object *spawn_object(struct Object *parent, s32 model, const BehaviorScript *behavior);
s16 object_step(void);
void treat_far_home_as_mario(f32 threshold);
s32 obj_forward_vel_approach(f32 target, f32 delta);
s32 obj_resolve_collisions_and_turn(s16 targetYaw, s16 turnSpeed);
s32 obj_bounce_off_walls_edges_objects(s32 *targetYaw);
s32 obj_resolve_object_collisions(s32 *targetYaw);

void obj_update_blinking(s32 *blinkTimer, s16 baseCycleLength, s16 cycleLengthRange,
                         s16 blinkLength);
s32 cur_obj_play_sound_at_anim_range(s8 arg0, s8 arg1, u32 sound);
void cur_obj_extend_animation_if_at_end(void);
void cur_obj_play_sound_2(s32 soundMagic);
void spawn_mist_particles(void);
void spawn_mist_particles_with_sound(u32 soundMagic);
s16 random_linear_offset(s16 base, s16 range);
s16 obj_random_fixed_turn(s16 delta);
s32 random_sign(void);
u16 random_u16(void);

void obj_set_face_angle_to_move_angle(struct Object *obj);
void obj_update_gfx_pos_and_angle(struct Object *obj);
void cur_obj_move_xz_using_fvel_and_yaw(void);
void cur_obj_move_y_with_terminal_vel(void);

s32 obj_is_rendering_enabled(void);
void cur_obj_become_tangible(void);
void cur_obj_become_intangible(void);
void cur_obj_hide(void);
void cur_obj_unhide(void);
void cur_obj_enable_rendering(void);
void cur_obj_disable_rendering(void);
f32 dist_between_objects(struct Object *obj1, struct Object *obj2);
s16 obj_angle_to_object(struct Object *obj1, struct Object *obj2);

s32 cur_obj_wait_then_blink(s32 timeUntilBlinking, s32 numBlinks);
void bhv_init_room(void);
s8 obj_flicker_and_disappear(struct Object *obj, s16 lifeSpan);

void huge_goomba_weakly_attacked(void);
void shelled_koopa_attack_handler(s32 attackType);
void wiggler_jumped_on_attack_handler(void);
void set_object_respawn_info_bits(struct Object *obj, u8 bits);

s32 cur_obj_get_int(u8 field);
void cur_obj_set_int(u8 field, s32 value);
void cur_obj_or_int(u8 field, s32 value);
void cur_obj_and_int(u8 field, s32 value);
void cur_obj_add_int(u8 field, s32 value);

f32 cur_obj_get_float(u8 field);
void cur_obj_set_float(u8 field, f32 value);
void cur_obj_add_float(u8 field, f32 value);

void cur_obj_set_vptr(u8 field, void *value);

void obj_and_int(struct Object *obj, u8 field, s32 value);

#ifdef __cplusplus
}
#endif