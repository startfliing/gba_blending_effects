#include "tonc.h"

#include "terminal.hpp"

#include "image.h"
#include "doduo.h"
#include "shiny.h"
#include "waves.h"

char* modes[4] = {
    "No Blending",
    "Standard Blending",
    "Fade to White",
    "Fade to Black"
};

void printStatus(int mode, int eva, int evb, int ey){
    VBlankIntrWait();
    Terminal::log("L/R: %%", modes[mode]);
    Terminal::log("</>: Top Weight: %%/16", eva/8);
    Terminal::log("v/^: Bottom Weight: %%/16", evb/8);
    Terminal::log("B/A: Fade Weight: %%/16", ey/8);
}

int main(){

    irq_init(NULL);
	irq_add(II_VBLANK, NULL);

    //enable Border BG
    u8 cbb = 0;
    u8 sbb = 16;
    REG_BG0CNT = BG_BUILD(cbb, sbb, 0, 0, 1, 0, 0);
    REG_BG2CNT = BG_BUILD(cbb, sbb+2, 0, 0, 2, 0, 0);

    //load palette
    memcpy16(pal_bg_mem, imagePal, imagePalLen/2);

    //load tiles
    LZ77UnCompVram(imageTiles, tile_mem[cbb]);
    memcpy16(&tile_mem[cbb][32], wavesTiles, sizeof(TILE)/2);
    
    //load image
    memcpy16(&se_mem[sbb], imageMap, imageMapLen/2);

    for(int i = 0; i < 14; i++){
        for(int j = 0; j < 30; j++){
            se_mem[sbb+2][(32*i)+j] = 32;
        }
    }

    //load sprites
    memcpy16(&tile_mem[4], doduoTiles, doduoTilesLen/2);

    memcpy16(pal_obj_mem, doduoPal, doduoPalLen/2);
    memcpy16(&pal_obj_mem[16], shinyPal, shinyPalLen/2);

    oam_init(oam_mem, 128);

    obj_set_attr(&oam_mem[0], ATTR0_SQUARE | ATTR0_BLEND | ATTR0_Y(24),
		ATTR1_SIZE_64 | ATTR1_X(32), 0);

    obj_set_attr(&oam_mem[1], ATTR0_SQUARE | ATTR0_Y(24),
		ATTR1_SIZE_64 | ATTR1_X(112), 0 | ATTR2_PALBANK(1));

    //enable Text BG
    REG_BG1CNT = Terminal::setCNT(1, cbb+1, sbb+1);
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D;

    int mode=0;
	// eva, evb and ey are .4 fixeds
	// eva is full, evb and ey are empty
	u32 eva=0, evb= 0, ey=0;

    REG_BLDCNT= BLD_BUILD(
		BLD_OBJ,	// Top
		BLD_BG2,			// Bottom
		mode);	
    
    printStatus(mode, eva, evb, ey);

    int animation = 0;
    int currAnimationTile = 0;

    while(1){

        if(key_hit(KEY_START)){
            Terminal::reset();
        }

        animation++;

        eva += key_tri_horz();
		evb -= key_tri_vert();
		ey  += key_tri_fire();

		mode += bit_tribool(key_hit(-1), KI_R, KI_L);

		// clamp to allowable ranges
		eva = clamp(eva, 0, 0x81);
		evb = clamp(evb, 0, 0x81);
		ey  = clamp(ey , 0, 0x81);
		mode= clamp(mode, 0, 4);


        // Update blend mode
		BFN_SET(REG_BLDCNT, mode, BLD_MODE);

		// Update blend weights
		REG_BLDALPHA= BLDA_BUILD(eva/8, evb/8);
		REG_BLDY= BLDY_BUILD(ey/8);

        printStatus(mode, eva, evb, ey);

        if(animation == 20){
            animation = 0;
            currAnimationTile = wrap(currAnimationTile + 1, 0, 8);
            memcpy16(&tile_mem[cbb][32], &wavesTiles[currAnimationTile*8], sizeof(TILE)/2);
        }

        //update random nunmber
        qran();

        //poll what keys are down
        key_poll();

    }
}