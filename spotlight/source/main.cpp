#include "tonc.h"

#include "spotlight.h"
#include "numbers.h"

int main(){

    // bg w. prio 1
    REG_BG0CNT = BG_BUILD(0, 15, 0, 0, 1, 0, 0);
    
    //load palettes, tiles, and maps
    memcpy16(&pal_bg_mem[0], numbersPal, numbersPalLen/2);
    LZ77UnCompVram(numbersTiles, &tile_mem[0]);
    memcpy16(&se_mem[15], numbersMap, numbersMapLen/2);

    //load sprite
    memcpy16(&tile_mem[4], spotlightTiles, spotlightTilesLen/2);
    memcpy16(pal_obj_mem, spotlightPal, spotlightPalLen/2);

    oam_init(oam_mem, 128);
    OBJ_ATTR* spotlight = obj_set_attr(
        &oam_mem[0], 
        ATTR0_SQUARE | ATTR0_BLEND | ATTR0_Y(32),
		ATTR1_SIZE_64 | ATTR1_X(32), 
        ATTR2_PRIO(0) | 1
    );

	int x = 32, y = 32;
    obj_set_pos(spotlight, x, y);

    //enable BGs
    REG_DISPCNT = DCNT_BG0 | DCNT_OBJ | DCNT_OBJ_1D | DCNT_MODE0;

    
    int fade = 0;

    REG_BLDALPHA = BLDA_BUILD(0, 0);
    REG_BLDCNT= BLD_BUILD(
		BLD_OBJ,	// Top
		BLD_BG0,	// Bottom
		1
    );
    
    bool active = true;

    while(1){

        if(key_is_down(KEY_DIR)){
            x = clamp(x + key_tri_horz(), -32, 208);
            y = clamp(y + key_tri_vert(), -32, 128);
            obj_set_pos(spotlight, x, y);
        }

        if(key_hit(KEY_A)){
            active = !active;
            active ? obj_unhide(spotlight, ATTR0_AFF) : obj_hide(spotlight);
        }

        obj_copy(obj_mem, spotlight, 1);

        fade = wrap(fade + 1, 0, 512);
        u16 pulse = sin_lut[fade] > 0 ? ((u16)(sin_lut[fade]))>>8 : ((u16)(-1*sin_lut[fade]))>>8;

        REG_BLDALPHA = BLDA_BUILD(pulse+1, 0);

        //poll what keys are down
        key_poll();

        //helps with visual tearing
        vid_vsync();
    }
}