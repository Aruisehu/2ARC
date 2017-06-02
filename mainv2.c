//this example shows non-looped vertical scroll, with 2x2 metatiles and attributes
//it is close enough to one that would be needed in an actual game
//horizontal mirroring (gives 32x60 nametable layout) is used to hide the changing row
//this example uses about 20% of CPU time

#include "neslib.h"
#include "utils.h"

//Ball sprite
#define BALL_SPR 0x40

//metatile palettes
#define BGPAL0	0x00	//bin 00000000
#define BGPAL1	0x55	//bin 01010101
#define BGPAL2	0xaa	//bin 10101010
#define BGPAL3	0xff	//bin 11111111


static unsigned char update_list[3+32+3+8+1];//3 bytes address and length for 32 tiles, 3 bytes address and length for 8 attributes, end marker

//palette data
const unsigned char palette[16]={ 0x0f,0x16,0x26,0x36,0x0f,0x18,0x28,0x38,0x0f,0x19,0x29,0x39,0x0f,0x1c,0x2c,0x3c };

//MetaSprite for the bar
const unsigned char barMetaSprite[]={
	0,	0,	0xF0,	1,
	8,	0,	0xF1,	1,
	16,	0,	0xF1,	1,
	24,	0,	0xF2,	1,
	128
};

//metatile definitions, one metatile by another, 4 bytes per metatile
const unsigned char metatiles[]={
	0x00,0x00,0x00,0x00,
	0x80,0x81,0x90,0x91,
	0x82,0x83,0x92,0x93,
	0x84,0x85,0x94,0x95,
	0x86,0x87,0x96,0x97
};

//metatile attributes, define which palette should be used for a metatile
const unsigned char metaattrs[]={
	BGPAL0,
	BGPAL0,
	BGPAL1,
	BGPAL2,
	BGPAL3
};

//level data, it is upside down to make it easier to access
unsigned char level_data[]={
	0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x04,
	0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,
	0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,
	0x00,0x00,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x00,0x00,
	0x00,0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x00,0x00,
	0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x00,
	0x03,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x04,
	0x01,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,//Last line, following are for offset
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

//calculate height of the level in pixels
#define LEVEL_HEIGHT	(sizeof(level_data)/16*16)



//this function prepares data for a row update
//also used to fill the screen before scrolling starts
void prepare_row_update(unsigned char name_row,unsigned int level_row)
{
	static unsigned char i,tile,attr,tile_off,updn_off,upda_off,mask1,mask2,mask3,mask4;
	static unsigned int name_adr,attr_adr;
	static const unsigned char *src;

	if(name_row<30)//calculate tile and attribute row addresses in a nametable
	{
		name_adr=NAMETABLE_A+(name_row<<5);
		attr_adr=NAMETABLE_A+960+((name_row>>2)<<3);
	}
	else
	{
		name_row-=30;

		name_adr=NAMETABLE_C+(name_row<<5);
		attr_adr=NAMETABLE_C+960+((name_row>>2)<<3);
	}

	update_list[0]=MSB(name_adr)|NT_UPD_HORZ;//set nametable update address
	update_list[1]=LSB(name_adr);

	update_list[35]=MSB(attr_adr)|NT_UPD_HORZ;//set attribute table update address
	update_list[36]=LSB(attr_adr);

	updn_off=3;//offset in the update list for nametable data
	upda_off=3+32+3;//offset for attribute data

	src=&level_data[level_row<<4];//get offset in the level data

	if(!(name_row&1)) tile_off=0; else tile_off=2;//metatile data offset, 2 for odd row numbers

	if(!(name_row&2))//select appropriate masks for attribute bytes
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

		attr=(update_list[upda_off]&mask1)|(metaattrs[tile]&mask2);//get metatile attribute, remember it

		tile=(tile<<2)+tile_off;//get metatile offset

		update_list[updn_off+0]=metatiles[tile+0];//put top or bottom half of the first metatile into the update buffer
		update_list[updn_off+1]=metatiles[tile+1];

		tile=*src++;//get second metatile code

		update_list[upda_off++]=(attr&mask3)|(metaattrs[tile]&mask4);//get attribute, add the other attribute, put into update buffer

		tile=(tile<<2)+tile_off;//get metatile offset

		update_list[updn_off+2]=metatiles[tile+0];//put top or bottom half of the second metatile into the update buffer
		update_list[updn_off+3]=metatiles[tile+1];

		updn_off+=4;
	}
}



//fill the whole screen with actual level graphics
//it uses the prepare_row function, but copies the data into VRAM using
//flush_vram_update instead of the update system (would take 30 frames otherwise)
void preload_screen(unsigned int level_y)
{
	static unsigned char i;

	for(i=0;i<18;++i)
	{
		prepare_row_update(17-i,level_y>>4);//prepare a row, bottom to top

		flush_vram_update(update_list);

		level_y+=8;
	}
}


//get metatile code for given x,y level position in pixels, relative to current offset in the level
//funciton like this could be used for collision detection purposes
unsigned char checkCollisionAndRemoveBrick(signed short x, signed short y)
{
	//144 = 18 * 8
	unsigned char brick;
	signed int level_y=(144-y);//the level is stored upside down, flip the axis

	if(level_y<0) level_y+=LEVEL_HEIGHT;//loop the level to avoid reading outside the level data

	brick = level_data[((level_y>>4)<<4)|(x>>4)];//level_y/16 is pixels to row, then *16 is row to offset, then x/16 is pixels to column

	if(brick != 0)
	{
		level_data[((level_y>>4)<<4)|(x>>4)] = 0;
		//TODO fix update
		//prepare_row_update(y>>4,y>>4);
		//set_vram_update(update_list);//the update is handled at next NMI
	}

	return brick;
}



//main function, program starts here
void main(void)
{
	static unsigned char pad,tile,sprite_x,sprite_y;
	static int i;//For for loop
	static unsigned char barX = 114;//Current X position of the bar
    static unsigned char barY = 200;//Current Y position of the bar (should never change)
    static unsigned char spr;//Used for sprite drawing
    static unsigned char ballStuck = TRUE;//1 when the ball is on the bar 0 when it's free
    static signed short ballX = 126<<4;//Current ball X position, position are multiplied by 16 to increase precision 
    static signed short ballY = 193<<4;//Current ball Y position, position are multiplied by 16 to increase precision
    static unsigned char ballVelocity;//The ball velocity
    static unsigned char ballDirection;//The ball direction (0 to 15) 0 is upward, 4 is left, 8 is downward, 12 is right
    static signed short velY, velX;//Motion used for collision detection
    static unsigned short aVelY;//Absolute motion of above
    static signed short nbx, nby;//Future ball's position to determinate collision
    static signed short barBallPos;//Position of the ball on the bar
    static unsigned short xb1, xb2, yb1, yb2, xa1, xa2, ya1, ya2;

	pal_spr(palette);//set sprite palette
	pal_bg(palette);//set background palette from an array
	pal_col(17,0x30);//white color for sprite

	update_list[0]=0x20|NT_UPD_HORZ;//horizontal update sequence, dummy address
	update_list[1]=0x00;
	update_list[2]=32;//length of nametable update sequence

	update_list[35]=0x20|NT_UPD_HORZ;
	update_list[36]=0x00;
	update_list[37]=8;//length of attribute update sequence

	update_list[46]=NT_UPD_EOF;

	preload_screen(0);//fill the whole screen with beginning of the level

	sprite_x=128;
	sprite_y=120;

	ppu_on_all();//enable rendering

	delay(60);//delay to show the preloaded part of the level

	while(1)
	{
		ppu_wait_nmi();

		/*
			Inputs
		*/

        //Get the user's inputs
        pad = pad_poll(0);

        //Move bar when left or right is pressed
        if(pad&PAD_LEFT && barX>= 1) 
        {
            barX -= 3;//Move bar 3px to the left
            if(ballStuck == TRUE)
            {
                ballX -= 48;//Move ball 3px to the left (or 48 unit) if it is still on the bar
            }
        }
        if(pad&PAD_RIGHT && barX<=222) 
        {
            barX += 3;//Move bar 3px to the right
            if(ballStuck == TRUE)
            {
                ballX += 48;//Move ball 3px to the right (or 48 unit) if it is still on the bar
            }
        }

        if(ballStuck == TRUE)//When the ball is still on the bar and the realease key is pressed we release it
        {
            if(pad&PAD_A)
            {
                ballStuck = FALSE;
                ballVelocity = 10;//max 50
                if(pad&PAD_LEFT)
                {
                    ballDirection = 2;
                }
                else if(pad&PAD_RIGHT)
                {
                    ballDirection = 14;
                }
                else
                {
                    ballDirection = 0;
                }
            }
        }

        /*
			Ball movement & collisions
        */

        if(ballStuck == FALSE)
        {
        	//Border collisions
        	if(ballY < 128)//Top (8*16)
            {
                ballY = 128;
                ballDirection = getCollisionBallDirection(ballDirection, TRUE);
            }
            else if(ballY > 3680)//Bottom (230*16)
            {
                ballY = 3680;
                ballDirection = getCollisionBallDirection(ballDirection, TRUE);
            }
            if(ballX < 0)//Left
            {
                ballX = 0;
                ballDirection = getCollisionBallDirection(ballDirection, FALSE);
            }
            else if(ballX > 3968)//Right (248*16)
            {
                ballX = 3968;
                ballDirection = getCollisionBallDirection(ballDirection, FALSE);
            }
            //Bar collisions
            else if(ballDirection >= 4 && ballDirection <= 12)//Only when the ball is going down
            {
            	velY = getYMotionFromDirection(ballDirection, ballVelocity);
                if(ballY + velY > 3150)//Check only when it's close to the bar
                {
                    velX = getXMotionFromDirection(ballDirection, ballVelocity);
                    aVelY = ABS(velY);
                    for(i = 0; i <= MAX(aVelY/20, 1); ++i)//Check for intermediary positions so that the ball do not go through the bar
                    {
                        nbx = ballX + (velX * i)/aVelY ;
                        nby = ballY + i;

                        //check with the top of the bar
                        barBallPos = nbx - (barX << 4);
                        if(barBallPos+128 > 0 && barBallPos < 512)//Check for x, +128 (8px * 16) for the width of the ball, 512 for the width of the bar ((32px*16)
                        {
                            if(nby+128 >= 3200 && nby+128 < 3280)//Check for y with 5px error (between 200px*16 and 205px*16)
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, TRUE);
                                ballY = 3056; //3056 -> (200px-9px)*16, y position of the bar minus the ball height +1
                                //Friction
                                if(barBallPos > 192)//192 -> 256-64 (middle of the paddle - middle of the ball)
                                {
                                	ballDirection = ballDirection == 0 ? 15 : ballDirection == 13 ? 13 :  ballDirection - 1;//We don't want the ball to be horizontal
                                    //angle -= 10 + (rand8() % 5);
                                }
                                else if(barBallPos < 192)
                                {
                                	ballDirection = ballDirection == 15 ? 0 : ballDirection == 3 ? 3 : ballDirection + 1;//We don't want the ball to be horizontal
                                   // angle += 10 + (rand8() % 5);
                                }
                            }
                        }
                    }
                }
            }

            //Bricks collision
            xa1 = ballX>>4;
            ya1 = ballY>>4;
            xa2 = xa1 + 8;
            ya2 = ya1 + 8;
			tile=checkCollisionAndRemoveBrick(xa1,ya1);//Collide with brick on ball's top left corner
			if(tile)
			{
				xb1 = xa1>>4<<4;
				xb2 = xb1+16;
				yb1 = ya1>>4<<4;
				yb2 = yb1+16;
			}
			if(!tile) 
			{
				tile=checkCollisionAndRemoveBrick(xa2,ya1);//Collide with brick on ball's top right corner
				if(tile)
				{
					xb1 = xa2>>4<<4;
					xb2 = xb1+16;
					yb1 = ya1>>4<<4;
					yb2 = yb1+16;
				}
            }
            if(!tile) 
			{
				tile=checkCollisionAndRemoveBrick(xa1,ya2);//Collide with brick on ball's bottom left corner
				if(tile)
				{
					xb1 = xa1>>4<<4;
					xb2 = xb1+16;
					yb1 = ya2>>4<<4;
					yb2 = yb1+16;
				}
            }
            if(!tile) 
			{
				tile=checkCollisionAndRemoveBrick(xa2,ya2);//Collide with brick on ball's bottom right corner
				if(tile)
				{
					xb1 = xa2>>4<<4;
					xb2 = xb1+16;
					yb1 = ya2>>4<<4;
					yb2 = yb1+16;
				}
            }
            if(tile)//If we collide with any brick we look where the ball is and bounce accordingly
            {
            	if(ballDirection == 0 || ballDirection == 8)
            	{
            		ballDirection = getCollisionBallDirection(ballDirection, TRUE);
            	}
            	else if(ballDirection == 4 || ballDirection == 12)
            	{
					ballDirection = getCollisionBallDirection(ballDirection, FALSE);
            	}
            	else if (ballDirection > 4 && ballDirection < 12 && ((xa1 >= xb1 && xa2 <= xb2) || (ya2 >= yb1 && ya1 < yb1 && ABS(ya2-yb1) < MIN(ABS(xa1-xb1), ABS(xa2-xb2)))))//Top
            	{
            		ballDirection = getCollisionBallDirection(ballDirection, TRUE);
            	}
            	else if(ballDirection > 0 && ballDirection < 8 && ((ya1 >= yb1 && ya2 <= yb2) || (xa1 <= xb2 && xa2 > xb2 && ABS(xa1-xb2) < MIN(ABS(ya1-yb1), ABS(ya2-yb2)))))//Right
            	{
					ballDirection = getCollisionBallDirection(ballDirection, FALSE);
            	}
            	else if((ballDirection < 4 || ballDirection > 12) && ((xa1 >= xb1 && xa2 <= xb2) || (ya1 <= yb2 && ya2 > yb1 && ABS(ya1-yb2) < MIN(ABS(xa1-xb1), ABS(xa2-xb2)))))//Bottom
            	{
					ballDirection = getCollisionBallDirection(ballDirection, TRUE);
            	}
            	else if(ballDirection > 8 && ((ya1 >= yb1 && ya2 <= yb2) || (xa2 >= xb1 && xa1 < xb1 && ABS(xa2-xb1) < MIN(ABS(ya1-yb1), ABS(ya2-yb2)))))//Left
            	{
					ballDirection = getCollisionBallDirection(ballDirection, FALSE);
            	}
            	else 
            	{
            		//Problem
            	}
            }

			//}

        	//We move the ball according to its direction and velocity
            ballX += getXMotionFromDirection(ballDirection, ballVelocity);
            ballY += getYMotionFromDirection(ballDirection, ballVelocity);
        }

		/*
		    Drawing
		*/

		spr = 0;

		spr = oam_meta_spr(barX, barY, spr, barMetaSprite);//Draw the bar at its coordinates
		spr = oam_spr((unsigned char)(ballX>>4), (unsigned char)(ballY>>4), BALL_SPR, 3, spr);//Draw the ball at its coordinates (converted from units to pixels)
	}
}
