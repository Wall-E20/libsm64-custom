#pragma once

#include <stdint.h>

enum MarioTextures
{
    mario_texture_yellow_button = 0,
    mario_texture_m_logo,
    mario_texture_hair_sideburn,
    mario_texture_mustache,
    mario_texture_eyes_front,
    mario_texture_eyes_half_closed,
    mario_texture_eyes_closed,
    mario_texture_eyes_left,
    mario_texture_eyes_right,
    mario_texture_eyes_up,
    mario_texture_eyes_down,
    mario_texture_eyes_dead,
    mario_texture_wings_half_1,
    mario_texture_wings_half_2,
    mario_texture_metal_wings_half_1,
    mario_texture_metal_wings_half_2,
    mario_texture_eyes_closed_unused1,
    mario_texture_eyes_closed_unused2,
    mario_texture_metal
};

//ik mario doesnt have 22 textures but mia has so i just do this
#define NUM_USED_TEXTURES 22

static const int mario_tex_offsets[NUM_USED_TEXTURES] = {4240, 6288, 8336, 10384, 12432, 14480, 16528, 22672, 0x6090, 0x6890, 0x7090, 30864, 32912, 37008, 0xA090, 0xB090, 0x4890, 0x5090, 144 };
static const int mario_tex_widths [NUM_USED_TEXTURES] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 64 };
static const int mario_tex_heights[NUM_USED_TEXTURES] = {32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 64, 64, 64, 64, 32, 32, 32 };

extern void load_mario_textures_from_rom( const uint8_t *rom, uint8_t *outTexture );