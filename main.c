#include "neslib.h"
#define BALL_SPR 0x40

static unsigned char i, j;
static unsigned char pad,spr;

//bar position
static unsigned char bar_x;
static unsigned char bar_y = 200;
//ball position & velocity
static unsigned char ball_stuck = 1;
static int ball_x;
static int ball_y;
static int velocity;
static int angle;
static int velx, vely;
//next ball position, used for the collision with the bar
static int nbx, nby;
//position of the ball on the bar (x)
static int barBallPos;

//Bar metasprite
//x, y, sprite, palette
const unsigned char metaBar[]={
	0,	0,	0xF0,	1,
	8,	0,	0xF1,	1,
	16,	0,	0xF1,	1,
	24,	0,	0xF2,	1,
	128
};

//Bricks metasprites
//x, y, sprite, palette
const unsigned char metaBrickBlue[]={
	0,	0,	0xE0,	1,
	8,	0,	0xE1,	1,
	16,	0,	0xE1,	1,
	24,	0,	0xE2,	1,
	128
};

//Color palette
const unsigned char palSprites[16]={
	0x0f,0x17,0x27,0x37,
	0x0f,0x11,0x21,0x31,
	0x0f,0x15,0x25,0x35,
	0x0f,0x19,0x29,0x39
};

int aproxcos(int a) //(return val * 200)
{
	switch((a/10)%36)
	{
		case 0:
		case 18:
			return 200;
		case 9:
		case 27:
			return 0;
		case 1:
		case 35:
			return 197;
		case 17:
		case 19:
			return -197;
		case 2:
		case 34:
			return 188;
		case 16:
		case 20:
			return -188;
		case 3:
		case 33:
			return 173;
		case 15:
		case 21:
			return -173;
		case 4:
		case 32:
			return 153;
		case 14:
		case 22:
			return -153;
		case 5:
		case 31:
			return 128;
		case 13:
		case 23:
			return -128;
		case 6:
		case 30:
			return 100;
		case 12:
		case 24:
			return -100;
		case 7:
		case 29:
			return 68;
		case 11:
		case 25:
			return -68;
		case 8:
		case 28:
			return 35;
		case 10:
		case 26:
			return 35;
		default:
			return 0;
	}
	return 0;
}

int aproxsin(int a) //(return val * 200)
{
	switch(((a/10)+9)%36)
	{
		case 0:
			return -200;
		case 18:
			return 200;
		default:
			return -aproxcos(a + 90);
	}
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
	bar_x = 114;
	ball_x = 1260;
	ball_y = 1930;

	while(1)
	{
		ppu_wait_frame();//wait for next frame
		
		spr = 0;

		//display metasprite
		//bar
		spr = oam_meta_spr(bar_x, bar_y, spr, metaBar);
		//ball
		spr = oam_spr((unsigned char)(ball_x/10), (unsigned char)(ball_y/10), BALL_SPR, 3, spr);
        
        spr = oam_meta_spr(0x20, 0x20, spr, metaBrickBlue);

		//poll pad
		pad = pad_poll(0);

		//Move bar
		if(pad&PAD_LEFT && bar_x>= 1) 
		{
			bar_x -= 3;
			if(ball_stuck)
			{
				ball_x -= 30;
			}
		}
		if(pad&PAD_RIGHT && bar_x<=222) 
		{
			bar_x += 3;
			if(ball_stuck)
			{
				ball_x += 30;
			}
		}

		//Move ball
		if(ball_stuck)
		{
			if(pad&PAD_A)
			{
				ball_stuck = 0;
				velocity = 10;//max 50
				if(pad&PAD_LEFT)
				{
					angle = 230;
				}
				else if(pad&PAD_RIGHT)
				{
					angle = 310;
				}
				else
				{
					angle = 270;
				}
			}
		}
		else
		{	
			//window collision
			if(ball_y < 80)//Top
			{
				ball_y = 80;
				angle = 360 - angle;
			}
			else if(ball_y > 2300)//Bottom
			{
				ball_y = 2300;
				angle = 360 - angle;
				//TODO loose
			}
			if(ball_x < 0)//Left
			{
				ball_x = 0;
				angle = 360 + 180 - angle;
			}
			else if(ball_x > 2480)//Right
			{
				ball_x = 2480;
				angle = 360 + 180 - angle;
			}
			angle = (angle+720) % 360;

			//bar collision
			//Check collision only when the ball is going down (for the bar)
			if(angle < 180)
			{
				vely = (aproxsin(angle) * velocity) / 200;
				if(ball_y + vely*10 > 2000)
				{
					velx = (aproxcos(angle) * velocity) / 200;
					j = abs(vely);
					for(i = 0; i <= MAX(j/20, 1); ++i)
					{
						nbx = ball_x + (velx/j) * i;
						nby = ball_y + i;

						//check with the top of the bar
						barBallPos = nbx - bar_x * 10;
						if(barBallPos+80 > 0 && barBallPos < 320)//Check for x (+80 for the width of the ball)
						{
							if(nby+80 >= 2010 && nby+80 < 2050)//Check for y with 4px error
							{
								angle = 360 - angle;
								ball_y = 1920;
								//Friction
								if(barBallPos < 120)//120 -> 160-40 (middle of the paddle - middle of the ball)
								{
									angle -= 10;
								}
								else if(barBallPos > 120)
								{
									angle += 10;
								}
							}
						}
					}
				}
			}

			//TODO check brick collision

			ball_x += (aproxcos(angle) * velocity) / 200;
			ball_y += (aproxsin(angle) * velocity) / 200;
		}
	}
}