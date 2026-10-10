#pragma once

#include <stdint.h>


#define SM64_TEX_MARIO_BASE 0
#define SM64_TEX_MARIO_COUNT 11

#define SM64_TEX_GOOMBA_BASE 11
#define SM64_TEX_GOOMBA_COUNT 3

#define SM64_TEX_COIN_BASE 14
#define SM64_TEX_COIN_COUNT 4

#define NUM_USED_TEXTURES (SM64_TEX_MARIO_COUNT + SM64_TEX_GOOMBA_COUNT + SM64_TEX_COIN_COUNT)

#define SM64_ATLAS_WIDTH (NUM_USED_TEXTURES * 64)
#define SM64_ATLAS_HEIGHT 64

enum MarioTextures
{
    mario_texture_metal = SM64_TEX_MARIO_BASE + 0,
    mario_texture_yellow_button = SM64_TEX_MARIO_BASE + 1,
    mario_texture_m_logo = SM64_TEX_MARIO_BASE + 2,
    mario_texture_hair_sideburn = SM64_TEX_MARIO_BASE + 3,
    mario_texture_mustache = SM64_TEX_MARIO_BASE + 4,
    mario_texture_eyes_front = SM64_TEX_MARIO_BASE + 5,
    mario_texture_eyes_half_closed = SM64_TEX_MARIO_BASE + 6,
    mario_texture_eyes_closed = SM64_TEX_MARIO_BASE + 7,
    mario_texture_eyes_dead = SM64_TEX_MARIO_BASE + 8,
    mario_texture_wings_half_1 = SM64_TEX_MARIO_BASE + 9,
    mario_texture_wings_half_2 = SM64_TEX_MARIO_BASE + 10,
    mario_texture_metal_wings_half_1 = 1000,
    mario_texture_metal_wings_half_2,
    mario_texture_eyes_closed_unused1,
    mario_texture_eyes_closed_unused2,
    mario_texture_eyes_right,
    mario_texture_eyes_left,
    mario_texture_eyes_up,
    mario_texture_eyes_down
};


extern const int sm64_tex_widths[NUM_USED_TEXTURES];
extern const int sm64_tex_heights[NUM_USED_TEXTURES];

extern const int mario_tex_offsets[SM64_TEX_MARIO_COUNT];
extern const int goomba_tex_offsets[SM64_TEX_GOOMBA_COUNT];
extern const int coin_tex_offsets[SM64_TEX_COIN_COUNT];

void load_mario_textures_from_rom( const uint8_t *rom, uint8_t *outTexture );
void load_goomba_textures_from_rom( const uint8_t *rom, uint8_t *outTexture );
void load_coin_textures_from_rom( const uint8_t *rom, uint8_t *outTexture );
