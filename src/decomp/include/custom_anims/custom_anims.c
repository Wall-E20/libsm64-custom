#include "mia_anim_first_person.raw.h"
#include "mia_anim_idle1.raw.h"
#include "mia_anim_idle2.raw.h"
#include "mia_anim_idle3.raw.h"
#include "mia_anim_jump.raw.h"
#include "mia_anim_run.raw.h"
#include "mia_anim_run_fast.raw.h"
#include "mia_anim_star_dance.raw.h"
#include "custom_anims.h"

int MIA_ANIM_FIRST_PERSON;
int MIA_ANIM_IDLE_1;
int MIA_ANIM_IDLE_2;
int MIA_ANIM_IDLE_3;
int MIA_ANIM_JUMP;
int MIA_ANIM_RUN;
int MIA_ANIM_RUN_FAST;
int MIA_ANIM_STAR_DANCE;

void load_custom_anims(){
    MIA_ANIM_FIRST_PERSON = load_mario_custom_anim_from_data(miaAnimFirstPersonRaw,miaAnimFirstPersonRaw_length);
    MIA_ANIM_IDLE_1 = load_mario_custom_anim_from_data(miaAnimIdle1Raw,miaAnimIdle1Raw_length);
    MIA_ANIM_IDLE_2 = load_mario_custom_anim_from_data(miaAnimIdle2Raw,miaAnimIdle2Raw_length);
    MIA_ANIM_IDLE_3 = load_mario_custom_anim_from_data(miaAnimIdle3Raw,miaAnimIdle3Raw_length);
    MIA_ANIM_JUMP = load_mario_custom_anim_from_data(miaAnimJumpRaw,miaAnimJumpRaw_length);
    MIA_ANIM_RUN = load_mario_custom_anim_from_data(miaAnimRunRaw,miaAnimRunRaw_length);
    MIA_ANIM_RUN_FAST = load_mario_custom_anim_from_data(miaAnimRunFastRaw,miaAnimRunFastRaw_length);
    MIA_ANIM_STAR_DANCE = load_mario_custom_anim_from_data(miaAnimStarDance,miaAnimStarDance_length);
}