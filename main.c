#include "neslib.h"
#define BALL_SPR 0x40

static unsigned char i, j, k;
static unsigned char pad,spr;

//bar position
static unsigned char barX;
static unsigned char barY = 200;
//ball position & velocity
static unsigned char ballStuck = 1;
static int ballX;
static int ballY;
static int velocity;
static int angle;
static int velX, velY;
//next ball position, used for the collision with the bar
static int nbx, nby;
//position of the ball on the bar (x)
static int barBallPos;
static int brickX, brickY;

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

int aproxCos(int a) //(return val * 200)
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

int aproxSin(int a) //(return val * 200)
{
	switch(((a/10)+9)%36)
	{
		case 0:
			return -200;
		case 18:
			return 200;
		default:
			return -aproxCos(a + 90);
	}
}

int abs(int val)
{
	return val < 0 ? val * -1 : val;
}


/*
Background test
*/
static unsigned char updateList[3+32+3+8+1];//3 bytes address and length for 32 tiles, 3 bytes address and length for 8 attributes, end marker
const unsigned char palette[16]={ 0x0f,0x16,0x26,0x36,0x0f,0x18,0x28,0x38,0x0f,0x19,0x29,0x39,0x0f,0x1c,0x2c,0x3c };

#define BGPAL0	0x00	//bin 00000000
#define BGPAL1	0x55	//bin 01010101
#define BGPAL2	0xaa	//bin 10101010
#define BGPAL3	0xff	//bin 11111111

//metatile definitions, one metatile by another, 2 bytes per metatile
const unsigned char metatiles[]={
	0x00,0x00,0x00,0x00,//Empty
	0xD0,0xD1,0xE0,0xE1,//Brick red
	0xD0,0xD1,0xE0,0xE1,//Brick yellow
	0xD0,0xD1,0xE0,0xE1,//Brick green
	0xD0,0xD1,0xE0,0xE1//Brick blue
};

//metatile attributes, define which palette should be used for a metatile

const unsigned char metaattrs[]={
	BGPAL0,
	BGPAL0,
	BGPAL1,
	BGPAL2,
	BGPAL3
};

const unsigned char levelData[]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,//Minimal use line
	0x01,0x00,0x00,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
	0x00,0x00,0x00,0x02,0x03,0x02,0x02,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x01,0x00,0x00,0x03,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x02,0x01,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00//End line
};

void prepareRowUpdate(unsigned char nameRow,unsigned int levelRow)
{
	static unsigned char i,tile,attr,tileOffset,updateNametableOffset,updateAttributeOffset,mask1,mask2,mask3,mask4;
	static unsigned int nameAddress,attributeAddress;
	static const unsigned char *src;

	if(nameRow<30)//calculate tile and attribute row addresses in a nametable
	{
		nameAddress=NAMETABLE_A+(nameRow<<5);
		attributeAddress=NAMETABLE_A+960+((nameRow>>2)<<3);
	}
	else
	{
		nameRow-=30;

		nameAddress=NAMETABLE_C+(nameRow<<5);
		attributeAddress=NAMETABLE_C+960+((nameRow>>2)<<3);
	}

	updateList[0]=MSB(nameAddress)|NT_UPD_HORZ;//set nametable update address
	updateList[1]=LSB(nameAddress);

	updateList[35]=MSB(attributeAddress)|NT_UPD_HORZ;//set attribute table update address
	updateList[36]=LSB(attributeAddress);

	updateNametableOffset=3;//offset in the update list for nametable data
	updateAttributeOffset=3+32+3;//offset for attribute data

	src=&levelData[levelRow<<4];//get offset in the level data

	if(!(nameRow&1)) tileOffset=0; else tileOffset=2;//metatile data offset, 2 for odd row numbers

	if(!(nameRow&2))//select appropriate masks for attribute bytes
	{
		mask1=0xfc;
		mask2=0x03;
		mask3=0xf3;
		mask4=0x0c;
	}
	else
	{
		mask1=0xcf;
		mask2=0x30;
		mask3=0x3f;
		mask4=0xc0;
	}

	for(i=0;i<8;++i)//as two neighborn attributes are grouped into a byte, the loop handles two metatiles in one iteration
	{
		tile=*src++;//get first metatile code

		attr=(updateList[updateAttributeOffset]&mask1)|(metaattrs[tile]&mask2);//get metatile attribute, remember it

		tile=(tile<<2)+tileOffset;//get metatile offset

		updateList[updateNametableOffset+0]=metatiles[tile+0];//put top or bottom half of the first metatile into the update buffer
		updateList[updateNametableOffset+1]=metatiles[tile+1];

		tile=*src++;//get second metatile code

		updateList[updateAttributeOffset++]=(attr&mask3)|(metaattrs[tile]&mask4);//get attribute, add the other attribute, put into update buffer

		tile=(tile<<2)+tileOffset;//get metatile offset

		updateList[updateNametableOffset+2]=metatiles[tile+0];//put top or bottom half of the second metatile into the update buffer
		updateList[updateNametableOffset+3]=metatiles[tile+1];

		updateNametableOffset+=4;
	}
}

void preloadScreen(unsigned int levelY)
{
	static unsigned char i;

	for(i=0;i<30;++i)
	{
		prepareRowUpdate(29-i,levelY>>4);//prepare a row, bottom to top

		flush_vram_update(updateList);

		levelY+=8;
	}
}



void main(void)
{
	pal_spr(palSprites);

	//set initial coords
	barX = 114;
	ballX = 1260;
	ballY = 1930;

	//Test background
	pal_bg(palette);//set background palette from an array
	pal_col(17,0x30);//white color for sprite

	updateList[0]=0x20|NT_UPD_HORZ;//horizontal update sequence, dummy address
	updateList[1]=0x00;
	updateList[2]=32;//length of nametable update sequence

	updateList[35]=0x20|NT_UPD_HORZ;
	updateList[36]=0x00;
	updateList[37]=8;//length of attribute update sequence

	updateList[46]=NT_UPD_EOF;
	preloadScreen(0);
	//BG end

	ppu_on_all();//enable rendering

	while(1)
	{
		ppu_wait_nmi();//wait for next nmi
		
		spr = 0;

		//display metasprite
		//bar
		spr = oam_meta_spr(barX, barY, spr, metaBar);
		//ball
		spr = oam_spr((unsigned char)(ballX/10), (unsigned char)(ballY/10), BALL_SPR, 3, spr);
        
        //spr = oam_meta_spr(0, 15+144, spr, metaBrickBlue);

		//poll pad
		pad = pad_poll(0);

		//Move bar
		if(pad&PAD_LEFT && barX>= 1) 
		{
			barX -= 3;
			if(ballStuck)
			{
				ballX -= 30;
			}
		}
		if(pad&PAD_RIGHT && barX<=222) 
		{
			barX += 3;
			if(ballStuck)
			{
				ballX += 30;
			}
		}

		//Move ball
		if(ballStuck)
		{
			if(pad&PAD_A)
			{
				ballStuck = 0;
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
			if(ballY < 80)//Top
			{
				ballY = 80;
				angle = 360 - angle;
			}
			else if(ballY > 2300)//Bottom
			{
				ballY = 2300;
				angle = 360 - angle;
				//TODO loose
			}
			if(ballX < 0)//Left
			{
				ballX = 0;
				angle = 360 + 180 - angle;
			}
			else if(ballX > 2480)//Right
			{
				ballX = 2480;
				angle = 360 + 180 - angle;
			}
			angle = (angle+720) % 360;

			//bar collision
			//Check collision only when the ball is going down (for the bar)
			if(angle < 180)
			{
				velY = (aproxSin(angle) * velocity) / 200;
				if(ballY + velY*10 > 2000)//Check only when it's close to the bar
				{
					velX = (aproxCos(angle) * velocity) / 200;
					j = abs(velY);
					for(i = 0; i <= MAX(j/20, 1); ++i)//Check for intermediary positions so that the ball do not go through the bar
					{
						nbx = ballX + (velX/j) * i;
						nby = ballY + i;

						//check with the top of the bar
						barBallPos = nbx - barX * 10;
						if(barBallPos+80 > 0 && barBallPos < 320)//Check for x (+80 for the width of the ball)
						{
							if(nby+80 >= 2010 && nby+80 < 2050)//Check for y with 4px error
							{
								angle = 360 - angle;
								ballY = 1920;
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

			velY = (aproxSin(angle) * velocity) / 200;
			if(ballY + velY*10 < 1700)//Check only when near bricks
			{
				velX = (aproxCos(angle) * velocity) / 200;
				j = abs(velY);
				for(i = 0; i <= MAX(j/20, 1); ++i)//Check for intermediary positions so that the ball do not go through the brick
				{
					nbx = ballX + (velX/j) * i;
					nby = ballY + i;

					if(levelData[80+(8-((nby/10-15)/16))*16+nbx/160] != 0x00)//80 = offset for first brick
					{
						angle = 720 - angle;
						ballX = nbx;
						ballY = nby;
						break;
					}
				}
			}

			//Bricks collisions
			

			ballX += (aproxCos(angle) * velocity) / 200;
			ballY += (aproxSin(angle) * velocity) / 200;
		}
	}
}