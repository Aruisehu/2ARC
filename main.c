#include "neslib.h"

#define BALL_SPR 0x40
#define BGPAL0	0x00	
#define BGPAL1	0x55
#define BGPAL2	0xaa
#define BGPAL3	0xff
#define NULL_BG 0xff //part of the tile that is completely black to represents null BG

static unsigned char i, j, k, l;
static unsigned char pad,spr;

//bar position
static unsigned char barX;
const unsigned char barY = 200;
//ball position & velocity
static unsigned char ballStuck = 1;
static short ballX;
static short ballY;
static short velocity;
static short angle;
static short velX, velY;
//next ball position, used for the collision with the bar
static short nbx, nby;
//position of the ball on the bar (x)
static unsigned short barBallPos;
static const unsigned char maskPpuBase = 0x80;
static unsigned char maskPpu;
static unsigned char maskLine;
/*
 *Here is a reprensentation of how the neighborsBrick is supposed to works
 *B represents the position where is the ball and numbers reprensents the brick around him
 *neighborsBrick array is supposed to contain index in levelData of the bricks around the ball
 * _________
 *| 0| 1| 2|
 *|________
 *| 3| B| 4|
 *|________
 *| 5| 6| 7|
 *|________
 *
 */

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

short aproxCos(short a) //(return val * 200)
{
    switch((a/10)%36)
    {
        case 0:
            return 200;
        case 18:
            return -200;
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

short aproxSin(short a) //(return val * 200)
{
    return -aproxCos(a + 90);
}

short abs(short val)
{
    return val < 0 ? val * -1 : val;
}

/*
   Background test
   */
const unsigned char palette[16]={ 0x0f,0x16,0x26,0x36,0x0f,0x18,0x28,0x38,0x0f,0x19,0x29,0x39,0x0f,0x1c,0x2c,0x3c };

//metatile definitions, one metatile by another, 2 bytes per metatile
const unsigned char metatiles[]={
    0x00,0x00,0x00,0x00,//Empty
    0xD0,0xD1,0xD1,0xD2,//base brick
};

//metatile attributes, define which palette should be used for a metatile

const unsigned char metaattrs[]={
    BGPAL0,
    BGPAL0,
    BGPAL1,
    BGPAL2,
    BGPAL3
};

/*static unsigned char levelData[]={
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
};*/

//Each byte in level Data represents one row.
//each bit in that byte determine if there is or not a brick in this position
//ex: 0111 0101 (0x75) means there's no brick in first position, there's one in 2,3,4 position, etc...
//one brick is 32 pixel wide
static unsigned char levelData[]= 
{
    0x00, 0x80, 0xFF, 0xFF,
    0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0xFF, 0xFF, 0xFF,
    0x00, 0xFF, 0xFF, 0xFF,
};

static unsigned char vramBuffer[36];
static unsigned char attrBuffer[68];

void updateAttrs()
{
    //set color for background
    //See https://wiki.nesdev.com/w/index.php/PPU_attribute_tables for more details
    //on how this works
    attrBuffer[0] = 0x23|NT_UPD_HORZ;
    attrBuffer[1] = 0xC0;
    attrBuffer[2] = 64;
    for(l=0; l<16; ++l)
    {
        attrBuffer[l*4 + 3] = BGPAL0;
        attrBuffer[l*4 + 4] = BGPAL1; 
        attrBuffer[l*4 + 5] = BGPAL2;
        attrBuffer[l*4 + 6] = BGPAL3;
    }
    attrBuffer[67] = NT_UPD_EOF;
    set_vram_update(attrBuffer);
    ppu_wait_frame();
}

void updateLineScreen(unsigned char line)
{
    //line is a number comprise beetween 0 and 30
    vramBuffer[0] = MSB(NAMETABLE_A + (line *32))|NT_UPD_HORZ; //When tile is filled, fill the one at its right
    vramBuffer[1] = LSB(NAMETABLE_A + (line *32));
    vramBuffer[2] = 32; //Data length
    for (l=0;l<8;++l)
    {
        maskPpu = maskPpuBase>>l;
        if ((levelData[line]&maskPpu) == maskPpu)
        {
            vramBuffer[l*4 +3]= 0xd0; 
            vramBuffer[l*4 +4]= 0xd1; 
            vramBuffer[l*4 +5]= 0xd1; 
            vramBuffer[l*4 +6]= 0xd2; 
        } else {
            vramBuffer[l*4 +3]= 0x00; 
            vramBuffer[l*4 +4]= 0x00; 
            vramBuffer[l*4 +5]= 0x00; 
            vramBuffer[l*4 +6]= 0x00; 
        }
    }
    vramBuffer[35] = NT_UPD_EOF;

    set_vram_update(vramBuffer);
    ppu_wait_frame();
}

void main(void)
{
    pal_spr(palSprites);

    //set initial coords
    barX = 114;
    ballX = 1260;
    ballY = 1930;

    pal_bg(palette);//set background palette from an array
    pal_col(17,0x30);//white color for sprite

    ppu_on_all();//enable rendering

    updateAttrs();
    for (i=0; i<20; ++i)
    {
        updateLineScreen(i);
    }

    while(TRUE)
    {
        ppu_wait_frame();//wait for next nmi
        spr = 0;


        //display metasprite
        //bar
        spr = oam_meta_spr(barX, barY, spr, metaBar);
        //ball
        spr = oam_spr((unsigned char)(ballX/10), (unsigned char)(ballY/10), BALL_SPR, 3, spr);

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
                angle = 540 - angle + (rand8() % 5);
            }
            else if(ballX > 2480)//Right
            {
                ballX = 2480;
                angle = 540 - angle + (rand8() % 5);
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
                                    angle -= 10 + (rand8() % 5);
                                }
                                else if(barBallPos > 120)
                                {
                                    angle += 10 + (rand8() % 5);
                                }
                            }
                        }
                    }
                }
            }

            //Bricks collisions
            //TODO count remaining bricks to win
            //TODO Fix collision
/*            velY = (aproxSin(angle) * velocity) / 200;
            //calculateBallIndex();
            if(ballY + velY*10 < 1700)//Check only when near bricks
            {
                velX = (aproxCos(angle) * velocity) / 200;
                j = abs(velY);

                for(i = 0; i <= MAX(j/20, 1); ++i)//Check for intermediary positions so that the ball do not go through the brick
                {
                    nbx = ballX + (velX/j) * i;
                    nby = ballY + i;
                    levelDataIndex = 80+(8-((nby/10-15)/16))*16+nbx/160;

                    if(levelData[levelDataIndex] != 0x00)//80 = offset for first brick
                    {
                        angle = 360 - angle;
                        ballX = nbx;
                        ballY = nby;
                        levelData[levelDataIndex] = 0x00;
                        ppu_off();
                        preloadScreen(0,(levelDataIndex / 16) * 2, (levelDataIndex / 16) * 2 + 2);
                        ppu_on_all();
                        break;
                    }
                }
            }*/

            ballX += (aproxCos(angle) * velocity) / 200;
            ballY += (aproxSin(angle) * velocity) / 200;
        }
    }
}
