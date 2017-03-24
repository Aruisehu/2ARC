//this example shows metasprite use, two pads polling,
//and simple collision detection that changes palette

#include "neslib.h"


//variables

static unsigned char i;
static unsigned char pad,spr;
//static unsigned char touch;
static unsigned char frame;
static unsigned char ball_stuck = 1;

//bar position
static unsigned char bar_x;
static unsigned char bar_y = 200;
//ball position & velocity
static int ball_x;
static int ball_y;
static int ball_vel_x;
static int ball_vel_y;

static unsigned char friction = 5;

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

//Check if the ball collide with the bar for all position between where it is and the position at the next frame
unsigned char ball_collide_bar_y()
{
	for(i = ball_y/10; i < (ball_y+ball_vel_y)/10; i += 2)
	{
		if(i >= 193 && i < 200)
		{
			return 1;
		}
	}
	return 0;
}

int abs(int val)
{
	return val < 0 ? val * -1 : val;
}

void main(void)
{
	pal_spr(palSprites);

	ppu_on_all();//enable rendering

	//set initial coords
	
	bar_x=114;
	ball_x = 1260;
	ball_y = 1930;
	ball_vel_x = 0;
	ball_vel_y = 0;
	//init other vars
	
	//touch=0;//collision flag
	frame=0;//frame counter

	//now the main loop

	while(1)
	{
		ppu_wait_frame();//wait for next TV frame
		
		spr=0;

		//display metasprite
			
		//bar
		spr=oam_meta_spr(bar_x,bar_y,spr,metaBar);
		//ball
		spr=oam_spr((unsigned char)(ball_x/10), (unsigned char)(ball_y/10),0x40,3,spr);

		//poll pad and change coordinates
		pad=pad_poll(0);

		//Move bar
		if(pad&PAD_LEFT &&bar_x>= 4) 
		{
			bar_x-=4;
			if(ball_stuck)
			{
				ball_x-=40;
			}
		}
		if(pad&PAD_RIGHT&&bar_x<=222) 
		{
			bar_x+=4;
			if(ball_stuck)
			{
				ball_x+=40;
			}
		}

		//Move ball
		if(ball_stuck)
		{
			if(pad&PAD_UP)
			{
				ball_stuck = 0;
				if(pad&PAD_LEFT)
				{
					ball_vel_y = -6;
					ball_vel_x = -6;
				}
				else if(pad&PAD_RIGHT)
				{
					ball_vel_y = -6;
					ball_vel_x = 6;
				}
				else
				{
					ball_vel_y = -9;
					ball_y -= 10;
				}
			}
		}
		else
		{	
			//window collision
			if(ball_y + ball_vel_y < 80)
			{
				ball_y = 80;
				ball_vel_y *= -1;
			}
			if(ball_x + ball_vel_x < 0)
			{
				ball_x = 0;
				ball_vel_x *= -1;
			}
			if(ball_x + ball_vel_x > 2480)
			{
				ball_x = 2480;
				ball_vel_x *= -1;
			}
			//bar collision
			if(ball_x/10 >= bar_x && ball_x/10 <= bar_x+24)
			{
				if(ball_collide_bar_y())
				{
					ball_vel_y *= -1;
					ball_y = 1930;

					//Friction
					if(pad&PAD_LEFT)
					{
						ball_vel_y += abs(ball_vel_x-friction) < abs(ball_vel_x) ? -friction : friction;
						ball_vel_x -= friction;
					}
					else if(pad&PAD_RIGHT)
					{
						ball_vel_y += abs(ball_vel_x-friction) < abs(ball_vel_x) ? -friction : friction;
						ball_vel_x += friction;
					}
				}
			}

			//TODO check brick collision

			ball_x += ball_vel_x;
			ball_y += ball_vel_y;
		}
		frame++;
	}
}