//this example shows metasprite use, two pads polling,
//and simple collision detection that changes palette

#include "neslib.h"



//variables

//static unsigned char i;
static unsigned char pad,spr;
//static unsigned char touch;
static unsigned char frame;
static unsigned char ball_stuck = 1;

//bar position
static unsigned char bar_x;
static unsigned char bar_y = 200;
//ball position & velocity
static unsigned char ball_x;
static unsigned char ball_y;
static char ball_vel_x;
static char ball_vel_y;

//Bar metasprite
//x, y, sprite, palette
const unsigned char metaBar[]={
	0,	0,	0xF0,	1,
	8,	0,	0xF1,	1,
	16,	0,	0xF1,	1,
	24,	0,	0xF2,	1,
	128
};

//Color palette

const unsigned char palSprites[16]={
	0x0f,0x17,0x27,0x37,
	0x0f,0x11,0x21,0x31,
	0x0f,0x15,0x25,0x35,
	0x0f,0x19,0x29,0x39
};



void main(void)
{
	pal_spr(palSprites);

	ppu_on_all();//enable rendering

	//set initial coords
	
	bar_x=114;
	ball_x = bar_x + 12;
	ball_y = 193;
	ball_vel_x = 0;
	ball_vel_y = 0;
	//init other vars
	
	//touch=0;//collision flag
	frame=0;//frame counter

	//now the main loop

	while(1)
	{
		ppu_wait_frame();//wait for next TV frame

		//flashing color for touch
		
		//i=frame&1?0x30:0x2a;

		//pal_col(17,touch?i:0x21);//set first sprite color
		//pal_col(21,touch?i:0x26);//set second sprite color

		//process players
		
		spr=0;

		///for(i=0;i<1;++i)
		//{
			//display metasprite
			
			//bar
			spr=oam_meta_spr(bar_x,bar_y,spr,metaBar);
			//ball
			spr=oam_spr(ball_x,ball_y,0x40,3,spr);

			//poll pad and change coordinates
			
			pad=pad_poll(0);

			if(pad&PAD_LEFT &&bar_x>=  3) 
			{
				bar_x-=3;
				if(ball_stuck)
				{
					ball_x-=3;
				}
			}
			if(pad&PAD_RIGHT&&bar_x<=222) 
			{
				bar_x+=3;
				if(ball_stuck)
				{
					ball_x+=3;
				}
			}
			if(ball_stuck)
			{
				if(pad&PAD_UP)
				{
					ball_stuck = 0;
					ball_vel_y = -1;
				}
			}
			else
			{
				//TODO check collision
				ball_x += ball_vel_x;
				ball_y += ball_vel_y;
			}
			//if(pad&PAD_DOWN &&cat_y[i]<212) cat_y[i]+=1;
		//}

		//check for collision for a smaller bounding box
		//metasprite is 24x24, collision box is 20x20
		
		/*if(!(cat_x[0]+22< cat_x[1]+2 ||
		     cat_x[0]+ 2>=cat_x[1]+22||
	         cat_y[0]+22< cat_y[1]+2 ||
		     cat_y[0]+ 2>=cat_y[1]+22)) touch=1; else touch=0;*/

		frame++;
	}
}