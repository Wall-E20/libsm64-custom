#pragma once

#include "../../../decomp/include/types.h"
#include "../../../decomp/include/PR/gbi.h"

/* Display lists defined in model.inc.c and referenced by geo.inc.c. The vertex,
 * light and texture data those lists use is file-static and stays in model.inc.c;
 * only the lists themselves need to be visible outside it. */

extern const Gfx goomba_seg8_dl_0801B2E8[];
extern const Gfx goomba_seg8_dl_0801B560[];
extern const Gfx goomba_seg8_dl_0801B5A0[];
extern const Gfx goomba_seg8_dl_0801B5C8[];
extern const Gfx goomba_seg8_dl_0801B5F0[];
extern const Gfx goomba_seg8_dl_0801B658[];
extern const Gfx goomba_seg8_dl_0801B690[];
extern const Gfx goomba_seg8_dl_0801CE20[];
extern const Gfx goomba_seg8_dl_0801CF78[];
extern const Gfx goomba_seg8_dl_0801D0D0[];
extern const Gfx goomba_seg8_dl_0801D360[];
extern const Gfx goomba_seg8_dl_0801D760[];

/* Animation table, defined in anims/table.inc.c. */
extern const struct Animation *const goomba_seg8_anims_0801DA4C[];

/* Texture pixel data is not compiled in yet (see model.inc.c); these names are
 * only used as texture-set selectors. TODO(goomba): bind real textures. */
extern const u8 goomba_seg8_texture_08019530[];
extern const u8 goomba_seg8_texture_08019D30[];
extern const u8 goomba_seg8_texture_0801A530[];