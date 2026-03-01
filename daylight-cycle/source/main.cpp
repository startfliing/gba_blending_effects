#include "tonc.h"

#include "day.h"
#include "night.h"

int main(){

    //day = BG0 pal[0] sbb 16 cbb 0
    //night = BG1 pal[1] sbb 17 cbb 1
    REG_BG0CNT = BG_BUILD(0, 15, 0, 0, 1, 0, 0);
    REG_BG1CNT = BG_BUILD(2, 31, 0, 0, 1, 0, 0);
    
    //load palettes
    memcpy16(&pal_bg_mem[0], dayPal, dayPalLen/2);
    memcpy16(&pal_bg_mem[16], nightPal, nightPalLen/2);

    //load tiles
    LZ77UnCompVram(dayTiles, &tile_mem[0]);
    LZ77UnCompVram(nightTiles, &tile_mem[2]);
    
    //load images
    memcpy16(&se_mem[15], dayMap, dayMapLen/2);
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < 32; j++){
            se_mem[31][(i*32)+j] = SE_PALBANK(1) | nightMap[(i*32)+j];
        }
    }

    //enable BGs
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_MODE0;


    //setup blending
	u32 eva=0, evb= 0;

    REG_BLDCNT= BLD_BUILD(
		BLD_BG0,	// Top
		BLD_BG1,			// Bottom
		1
    );	

    //mode 1 standard blending
    //initial eva 0 evb 0
    //0 - 15 : evb+
    //16 - 31 : evb- eva+
    //32 - 47 : evb+
    //48 - 63 : evb-
    //64 - 79 : evb+ eva-
    //80 - 95 : evb-

    int frameCt = 0;
    int dayCycle = 0;

    while(1){

        if(frameCt >= 16){
            dayCycle++;

            if(dayCycle <= 16){
                evb++;
            }else if(dayCycle <= 32){
                evb--;
                eva++;
            }else if(dayCycle <= 48){
                evb++;
            }else if(dayCycle <= 64){
                evb--;
            }else if(dayCycle <= 80){
                evb++;
                eva--;
            }else if(dayCycle <= 96){
                evb--;
            }else{
                dayCycle = 0;
            }

            frameCt = 0;
        }else{
            frameCt++;
        }

        //midday
        if(key_hit(KEY_A)){
            frameCt = 0;
            dayCycle = 48;
            evb = 16;
            eva = 16;
        }

        //midnight
        if(key_hit(KEY_B)){
            frameCt = 0;
            dayCycle = 0;
            evb = 0;
            eva = 0;
        }


		// Update blend weights
		REG_BLDALPHA= BLDA_BUILD(eva, evb);
        
        //poll what keys are down
        key_poll();

        //helps with visual tearing
        vid_vsync();
    }
}