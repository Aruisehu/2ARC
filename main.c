//this example shows metasprite use, two pads polling,
//and simple collision detection that changes palette

#include "neslib.h"


//variables

static unsigned char i, j;
static unsigned char pad,spr;
static unsigned char ball_stuck = 1;

//bar position
static unsigned char bar_x;
static unsigned char bar_y = 200;
//ball position & velocity
static int ball_x;
static int ball_y;
static int velocity;
static int angle;

//next ball position, used for the collision with the bar
static int nbx, nby;
//position of the ball on the bar (x)
static int barBallPos;

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

int aproxcos(int angle) //(return * 1000)
{
	switch(angle%360)
	{
		case 0:
		case 180:
			return 1000;
		case 90:
		case 270:
			return 0;
		case 10:
		case 350:
			return 984;
		case 170:
		case 190:
			return -984;
		case 20:
		case 340:
			return 939;
		case 160:
		case 200:
			return -939;
		case 30:
		case 330:
			return 866;
		case 150:
		case 210:
			return -866;
		case 40:
		case 320:
			return 766;
		case 140:
		case 220:
			return -766;
		case 50:
		case 310:
			return 642;
		case 130:
		case 230:
			return -642;
		case 60:
		case 300:
			return 500;
		case 120:
		case 240:
			return -500;
		case 70:
		case 290:
			return 342;
		case 110:
		case 250:
			return -342;
		case 80:
		case 280:
			return 173;
		case 100:
		case 260:
			return 173;
		default:
			return aproxcos((angle/10)*10);
	}
	return 0;
}
int aproxsin(int angle) //(return * 1000)
{
	return aproxcos(angle + 90);
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

	while(1)
	{
		ppu_wait_frame();//wait for next frame
		
		spr=0;

		//display metasprite
			
		//bar
		spr=oam_meta_spr(bar_x,bar_y,spr,metaBar);
		//ball
		spr=oam_spr((unsigned char)(ball_x/10), (unsigned char)(ball_y/10),0x40,3,spr);

		//poll pad and change coordinates
		pad=pad_poll(0);

		//Move bar
		if(pad&PAD_LEFT &&bar_x>= 1) 
		{
			bar_x-=1;
			if(ball_stuck)
			{
				ball_x-=10;
			}
		}
		if(pad&PAD_RIGHT&&bar_x<=222) 
		{
			bar_x+=1;
			if(ball_stuck)
			{
				ball_x+=10;
			}
		}

		//Move ball
		if(ball_stuck)
		{
			if(pad&PAD_A)
			{
				ball_stuck = 0;
				velocity = 8;
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
			if(ball_y  < 1600)//Top
			{
				ball_y = 1600;//DEBUG (/20)
				angle = 180-angle;
			}
			if(ball_y  > 2300)//bottom
			{
				ball_y = 2300;
				angle = 180 - angle;
				//TODO loose
			}
			if(ball_x  < 0)//Left
			{
				ball_x = 0;
				angle = 360 - angle;
			}
			if(ball_x  > 2480)//Right
			{
				ball_x = 2480;
				angle = 360 - angle;
			}
			//angle = (angle+720) % 360;

			//bar collision
			//Check collision only when the ball is going down (for the bar)
			if(angle < 180)
			{
				j = abs((aproxsin(angle)*velocity)/1000) > abs((aproxcos(angle)*velocity)/1000) ? abs((aproxsin(angle)*velocity)/1000) : abs((aproxcos(angle)*velocity)/1000);
				for(i = 0; i <= MAX(j/10, 1); i++)
				{
					nbx = ball_x + (((aproxcos(angle)*velocity)/1000)/j)*i;
					nby = ball_y + (((aproxsin(angle)*velocity)/1000)/j)*i;

					//check with the top of the bar
					barBallPos = nbx-bar_x*10;
					if(barBallPos+80 > 0 && barBallPos < 320)//Check for x (+80 for the width of the ball)
					{
						if(nby+80 >= 2010 && nby+80 < 2030)//Check for y with 2px error
						{
							angle = 180 - angle;
							ball_y = 1930;
							//Friction
							if(barBallPos < 160)
							{
								angle += 10;
							}
							else if(barBallPos > 160)
							{
								angle -= 10;
							}
						}
						//Sides
						else if(nby+80 > 2020 && nby < 2100)
						{
							if(barBallPos+80 < 10)//Left
							{
								angle = 360 - angle;
							}
							else if(barBallPos > 310)//Right
							{
								angle = 360 - angle;
							}
						}
					}
				}
			}

			//TODO check brick collision

			ball_x += (aproxcos(angle)*velocity)/1000;
			ball_y += (aproxsin(angle)*velocity)/1000;
		}
	}
}