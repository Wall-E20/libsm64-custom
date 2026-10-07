#define _CRT_SECURE_NO_WARNINGS 1 // for fopen

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>

#include "../src/libsm64.h"

extern "C" {
#define SDL_MAIN_HANDLED
#include "level.h"
#include "context.h"
#include "renderer.h"
#include "gl33core/gl33core_renderer.h"
#include "gl20/gl20_renderer.h"
}

#include "audio.h"

uint8_t *utils_read_file_alloc( const char *path, size_t *fileLength )
{
    FILE *f = fopen( path, "rb" );

    if( !f ) return NULL;

    fseek( f, 0, SEEK_END );
    size_t length = (size_t)ftell( f );
    rewind( f );
    uint8_t *buffer = (uint8_t*)malloc( length + 1 );
    fread( buffer, 1, length, f );
    buffer[length] = 0;
    fclose( f );

    if( fileLength ) *fileLength = length;

    return buffer;
}

static float read_axis( int16_t val )
{
    float result = (float)val / 32767.0f;

    if( result < 0.2f && result > -0.2f )
        return 0.0f;

    return result > 0.0f ? (result - 0.2f) / 0.8f : (result + 0.2f) / 0.8f;
}

/*
float lerp(float a, float b, float amount)
{
    return a + (b - a) * amount;
}
*/
int main( void )
{
    size_t romSize;
    
    uint8_t *rom = utils_read_file_alloc( "baserom.us.z64", &romSize );

    if( rom == NULL )
    {
        printf("\nFailed to read ROM file \"baserom.us.z64\"\n\n");
        return 1;
    }

    uint8_t *texture = (uint8_t*)malloc( 4 * SM64_TEXTURE_WIDTH * SM64_TEXTURE_HEIGHT );

    sm64_global_terminate();
    sm64_global_init( rom, texture );
    sm64_audio_init(rom);
    sm64_static_surfaces_load( surfaces, surfaces_count );
    int32_t marioId = sm64_mario_create( 300, 7000, 100 );
    sm64_set_mario_action(marioId, 0x0000192A);
    
    free( rom );

    RenderState renderState;
    renderState.mario.index = NULL;
    vec3 cameraPos = { 0, 0, 0 };
    float cameraRot = 0.0f;

    struct Renderer *renderer;

    int major, minor;
#ifdef GL33_CORE
    major = 3; minor = 3;
    renderer = &gl33core_renderer;
#else
    major = 2; minor = 0;
    renderer = &gl20_renderer;
#endif

    context_init( "libsm64", 800, 600, major, minor );
    renderer->init( &renderState, texture );

    struct SM64MarioInputs marioInputs;
    struct SM64MarioState marioState;
    struct SM64MarioGeometryBuffers marioGeometry;

    // interpolation
    float lastPos[3], currPos[3];
    float lastGeoPos[9 * SM64_GEO_MAX_TRIANGLES], currGeoPos[9 * SM64_GEO_MAX_TRIANGLES];

    marioGeometry.position = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
    marioGeometry.color    = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
    marioGeometry.normal   = (float*)malloc( sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES );
    marioGeometry.uv       = (float*)malloc( sizeof(float) * 6 * SM64_GEO_MAX_TRIANGLES );
    marioGeometry.numTrianglesUsed = 0;

    float tick = 0;
    uint32_t lastTicks = SDL_GetTicks();
    int cameramode = 0x01;
    audio_init();
    sm64_set_camera_mode(cameramode, 0);
    sm64_play_music(0, 0x08 | 0x80, 0); // from decomp/include/seq_ids.h: SEQ_LEVEL_WATER | SEQ_VARIATION

    marioState.lodState = 1;
    marioState.lodsOverride = 1;
    sm64_set_mario_water_level(marioId, -4610);
    do
    {
        float dt = (SDL_GetTicks() - lastTicks) / 1000.f;
        lastTicks = SDL_GetTicks();
        tick += dt;
        const Uint8* state = SDL_GetKeyboardState(NULL);
        SDL_GameController *controller = context_get_controller();
        float x_axis, y_axis, x0_axis;

        if (!controller) // keyboard
        {
            
            
            float dir;
            float spd = 0;
            
            if (state[SDL_SCANCODE_W] && state[SDL_SCANCODE_D])
            {
                dir = -M_PI * 0.25f;
                spd = 1;
            }
            else if (state[SDL_SCANCODE_W] && state[SDL_SCANCODE_A])
            {
                dir = -M_PI * 0.75f;
                spd = 1;
            }
            else if (state[SDL_SCANCODE_S] && state[SDL_SCANCODE_D])
            {
                dir = M_PI * 0.25f;
                spd = 1;
            }
            else if (state[SDL_SCANCODE_S] && state[SDL_SCANCODE_A])
            {
                dir = M_PI * 0.75f;
                spd = 1;
            }
            else if (state[SDL_SCANCODE_W])
            {
                dir = -M_PI * 0.5f;
                spd = 1;
            }
            else if (state[SDL_SCANCODE_S])
            {
                dir = M_PI * 0.5f;
                spd = 1;
            }
            else if (state[SDL_SCANCODE_A])
            {
                dir = M_PI;
                spd = 1;
            }
            else if (state[SDL_SCANCODE_D])
            {
                dir = 0;
                spd = 1;
            }
            
            x_axis = cosf(dir) * spd;
            y_axis = sinf(dir) * spd;
            x0_axis = state[SDL_SCANCODE_LSHIFT] ? 1 : state[SDL_SCANCODE_RSHIFT] ? -1 : 0;

            marioInputs.buttonA = state[SDL_SCANCODE_X];
            marioInputs.buttonB = state[SDL_SCANCODE_C];
            marioInputs.buttonZ = state[SDL_SCANCODE_Z];
            marioInputs.buttonL = state[SDL_SCANCODE_LEFT];
            marioInputs.buttonR = state[SDL_SCANCODE_RIGHT];
            marioInputs.buttonU = state[SDL_SCANCODE_UP];
            marioInputs.buttonD = state[SDL_SCANCODE_DOWN];
        }
        else
        {
            x_axis = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_LEFTX ));
            y_axis = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_LEFTY ));
            x0_axis = read_axis( SDL_GameControllerGetAxis( controller, SDL_CONTROLLER_AXIS_RIGHTX ));

            marioInputs.buttonA = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_A );
            marioInputs.buttonB = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_X );
            marioInputs.buttonZ = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER );
            marioInputs.buttonL = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT );
            marioInputs.buttonR = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT );
            marioInputs.buttonU = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_DPAD_UP );
            marioInputs.buttonD = SDL_GameControllerGetButton( controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN );


        }
            if (state[SDL_SCANCODE_R]){
                sm64_set_mario_action(marioId, 0x0100088C);
                sm64_set_camera_mode(cameramode, 0);
            }
            if (state[SDL_SCANCODE_T]){
                sm64_set_mario_action(marioId, 0x04001301);
            }
            if (state[SDL_SCANCODE_C]){
                sm64_mario_interact_cap(marioId,0x00000008, 1000, 1);
            }
        cameraRot += x0_axis * dt * 2;
        cameraPos[0] = marioState.position[0] + 1000.0f * cosf( cameraRot );
        cameraPos[1] = marioState.position[1] + 200.0f;
        cameraPos[2] = marioState.position[2] + 1000.0f * sinf( cameraRot );
        
        vec3 refcam;


        marioInputs.camLookX = marioState.position[0] - refcam[0];
        marioInputs.camLookZ = marioState.position[2] - refcam[2];
        marioInputs.stickX = x_axis;
        marioInputs.stickY = y_axis;
        
        while (tick >= 1.f/30)
        {   
            memcpy(lastPos, currPos, sizeof(currPos));
            memcpy(lastGeoPos, currGeoPos, sizeof(currGeoPos));

            tick -= 1.f/30;

            sm64_mario_tick( marioId, &marioInputs, &marioState, &marioGeometry );
            memcpy(currPos, marioState.position, sizeof(currPos));
            memcpy(currGeoPos, marioGeometry.position, sizeof(currGeoPos));

        }
        //*
        refcam[0] = marioState.camPos[0];
        refcam[1] = marioState.camPos[1];
        refcam[2] = marioState.camPos[2];
        //*/
        //uncomment this and comment the one above to use the non-sm64 camera
        //also be sure to change the focus in renderer->draw
        /*
        refcam[0] = cameraPos[0];
        refcam[1] = cameraPos[1];
        refcam[2] = cameraPos[2];
        /*/

        for (int i=0; i<3; i++) marioState.position[i] = lerp(lastPos[i], currPos[i], tick);
        for (int i=0; i<marioGeometry.numTrianglesUsed*9; i++) marioGeometry.position[i] = lerp(lastGeoPos[i], currGeoPos[i], 1.f);

        renderer->draw( &renderState, refcam, marioState.camFocus, &marioState, &marioGeometry );
    }
    while( context_flip_frame_poll_events() );

    sm64_stop_background_music(sm64_get_current_background_music());
    sm64_global_terminate();
    context_terminate();

    return 0;
}
