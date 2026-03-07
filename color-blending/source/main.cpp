#include "tonc.h"

#include "yellow.h"
#include "blue.h"

int main(){

    REG_BG0CNT = BG_BUILD(0, 15, 0, 0, 1, 0, 0);
    REG_BG1CNT = BG_BUILD(2, 31, 0, 0, 1, 0, 0);
    
    //load palettes
    memcpy16(&pal_bg_mem[0], yellowPal, yellowPalLen/2);
    memcpy16(&pal_bg_mem[16], bluePal, bluePalLen/2);

    //load tiles
    LZ77UnCompVram(yellowTiles, &tile_mem[0]);
    LZ77UnCompVram(blueTiles, &tile_mem[2]);
    
    //load images
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < 32; j++){
            se_mem[15][(i*32)+j] = SE_PALBANK(0) | yellowMap[(i*32)+j];
            se_mem[31][(i*32)+j] = SE_PALBANK(1) | blueMap[(i*32)+j];
        }
    }

    //enable BGs
    REG_DISPCNT = DCNT_BG0 | DCNT_BG1 | DCNT_MODE0;

    REG_BLDCNT= BLD_BUILD(
		BLD_BG0,	// Top
		BLD_BG1,			// Bottom
		1
    );	

    REG_BLDALPHA = BLD_BUILD(8,8,1);


    while(1){};
}