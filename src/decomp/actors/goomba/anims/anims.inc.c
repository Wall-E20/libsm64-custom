/*
 * Goomba animations.
 *
 * data.inc.c defines the anim value/index tables and the struct Animation;
 * table.inc.c defines the goomba_seg8_anims_0801DA4C lookup table. Both are
 * .inc.c because n64decomp includes them into one translation unit -- the
 * struct Animation initializer references the file-static tables directly.
 *
 * These are compiled in, unlike the ROM-loaded Mario animations in
 * load_anim_data.c. segmented_to_virtual() is a no-op in this fork, so the
 * values/index pointers need no relocation.
 */
#include "../../../../decomp/include/types.h"

#include "data.inc.c"
#include "table.inc.c"