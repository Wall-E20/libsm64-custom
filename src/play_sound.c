#include "play_sound.h"

#include "decomp/audio/external.h"
#include "debug_print.h"
#include "load_audio_data.h"
#include "decomp/shim.h"

SM64PlaySoundFunctionPtr g_play_sound_func = NULL;

extern void play_sound( uint32_t soundBits, f32 *pos ) {
    if ( g_is_audio_initialized ) {
        DEBUG_PRINT("$ play_sound(%d) request %d; pos %f %f %f\n", soundBits,sSoundRequestCount,pos[0],pos[1],pos[2]);
        sSoundRequests[sSoundRequestCount].soundBits = soundBits;
        sSoundRequests[sSoundRequestCount].position = pos;
        sSoundRequestCount++;
    }

    if ( g_play_sound_func ) {
        g_play_sound_func(soundBits, pos);
    }
}

extern void create_sound_spawner(uint32_t soundBits)
{
    play_sound(soundBits, gCurrentObject->header.gfx.cameraToObject);
}

extern void cur_obj_play_sound_2(s32 soundMagic) {
    if (gCurrentObject->header.gfx.node.flags & GRAPH_RENDER_ACTIVE) {
        play_sound(soundMagic, gCurrentObject->header.gfx.cameraToObject);
    }
}