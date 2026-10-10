#include "load_tex_data.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "libsm64.h"

#include "decomp/tools/libmio0.h"
#include "decomp/tools/n64graphics.h"


#define TEX_BLOCK_ROM_OFFSET 1132368   //0x114750


#define SEG8_BLOCK_ROM_OFFSET 0x1F2200
#define SEG8_BASE 0x1C

_Static_assert( NUM_USED_TEXTURES == SM64_TEXTURE_SLOTS,
                "atlas slot count disagrees with libsm64.h's SM64_TEXTURE_SLOTS" );

const int sm64_tex_widths [NUM_USED_TEXTURES] = {
    /* mario*/ 64, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    /* goomba*/ 32, 32, 32
};
const int sm64_tex_heights[NUM_USED_TEXTURES] = {
    /* mario*/ 32, 32, 32, 32, 32, 32, 32, 32, 32, 64, 64,
    /* goomba*/ 32, 32, 32
};

const int mario_tex_offsets[SM64_TEX_MARIO_COUNT] = {
    144, 4240, 6288, 8336, 10384, 12432, 14480, 16528, 30864, 32912, 37008
};

const int goomba_tex_offsets[SM64_TEX_GOOMBA_COUNT] = {
    SEG8_BASE + 0x19530 + 0x22,
    SEG8_BASE + 0x19D30 + 0x22,
    SEG8_BASE + 0x1A530 + 0x22,
};

static void blt_image_to_atlas( rgba *img, int slot, int w, int h, uint8_t *outTexture )
{
    for( int iy = 0; iy < h; ++iy )
    for( int ix = 0; ix < w; ++ix )
    {
        int o = (ix + 64 * slot) + iy * SM64_ATLAS_WIDTH;
        int q = ix + iy * w;
        outTexture[4*o + 0] = img[q].red;
        outTexture[4*o + 1] = img[q].green;
        outTexture[4*o + 2] = img[q].blue;
        outTexture[4*o + 3] = img[q].alpha;
    }
}

void load_mario_textures_from_rom( const uint8_t *rom, uint8_t *outTexture )
{
    memset( outTexture, 0, 4 * SM64_ATLAS_WIDTH * SM64_ATLAS_HEIGHT );

    mio0_header_t head;
    if( !mio0_decode_header( rom + TEX_BLOCK_ROM_OFFSET, &head ) ) return;

    uint8_t *block = malloc( head.dest_size );
    if( !block ) return;
    if( mio0_decode( rom + TEX_BLOCK_ROM_OFFSET, block, NULL ) <= 0 ) { free( block ); return; }

    for( int i = 0; i < SM64_TEX_MARIO_COUNT; ++i )
    {
        int off = mario_tex_offsets[i];
        if( off + 4 > (int)head.dest_size ) continue;
        uint8_t *raw = block + off;
        rgba *img = raw2rgba( raw, sm64_tex_widths[SM64_TEX_MARIO_BASE + i],
                              sm64_tex_heights[SM64_TEX_MARIO_BASE + i], 16 );
        if( !img ) continue;
        blt_image_to_atlas( img, SM64_TEX_MARIO_BASE + i,
                            sm64_tex_widths[SM64_TEX_MARIO_BASE + i],
                            sm64_tex_heights[SM64_TEX_MARIO_BASE + i], outTexture );
        free( img );
    }

    free( block );
}


void load_goomba_textures_from_rom( const uint8_t *rom, uint8_t *outTexture )
{
    mio0_header_t head;
    if( !mio0_decode_header( rom + SEG8_BLOCK_ROM_OFFSET, &head ) ) return;

    uint8_t *block = malloc( head.dest_size );
    if( !block ) return;
    if( mio0_decode( rom + SEG8_BLOCK_ROM_OFFSET, block, NULL ) <= 0 ) { free( block ); return; }

    for( int i = 0; i < SM64_TEX_GOOMBA_COUNT; ++i )
    {
        int off = goomba_tex_offsets[i];
        int w = sm64_tex_widths[SM64_TEX_GOOMBA_BASE + i];
        int h = sm64_tex_heights[SM64_TEX_GOOMBA_BASE + i];
        if( off < 0 || off + w * h * 2 > (int)head.dest_size ) continue;

        uint8_t *raw = block + off;
        rgba *img = raw2rgba( raw, w, h, 16 );
        if( !img ) continue;
        blt_image_to_atlas( img, SM64_TEX_GOOMBA_BASE + i, w, h, outTexture );
        free( img );
    }

    free( block );
}
