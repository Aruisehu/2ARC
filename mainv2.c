//this example shows non-looped vertical scroll, with 2x2 metatiles and attributes
//it is close enough to one that would be needed in an actual game
//horizontal mirroring (gives 32x60 nametable layout) is used to hide the changing row
//this example uses about 20% of CPU time

#include "utils.h"


//palette data
const unsigned char palette[16]={ 0x0f,0x16,0x26,0x36,0x0f,0x18,0x28,0x38,0x0f,0x19,0x29,0x39,0x0f,0x1c,0x2c,0x3c};
const unsigned char maskPpuBase = 0x80;
static unsigned char currentLevel = 0;

//MetaSprite for the bar
const unsigned char barMetaSprite[]={
    0,	0,	0xF0,	1,
    8,	0,	0xF1,	1,
    16,	0,	0xF1,	1,
    24,	0,	0xF2,	1,
    128
};

static unsigned char emptyLevelData[] =
{
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

static unsigned char firstLevelData[] =
{
    0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

static unsigned char secondLevelData[] =
{
    0x00, 0x00, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xEF, 0xE0, 0xEF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

static unsigned char* allLevels[] =
{
    firstLevelData, secondLevelData
};
//Each byte in level Data represents one row.
//each bit in that byte determine if there is or not a brick in this position
//ex: 0111 0101 (0x75) means there's no brick in first position, there's one in 2,3,4 position, etc...
//one brick is 32 pixel wide
static unsigned char levelData[30];

// number of brick in all level. Used to
static unsigned char bricksInLevel[] = {144, 137};

static unsigned char destroyedBrickCount = 0;

static unsigned char winBuffer[10+1]={
    MSB(NTADR_A(12,12))|NT_UPD_HORZ,LSB(NTADR_A(12,12)),7,'Y'-0x20,'O'-0x20,'U'-0x20,' '-0x20,'W'-0x20,'I'-0x20,'N'-0x20,
    NT_UPD_EOF
};

static unsigned char looseBuffer[12+1]={
    MSB(NTADR_A(11,12))|NT_UPD_HORZ,LSB(NTADR_A(11,12)),9,'G'-0x20,'A'-0x20,'M'-0x20,'E'-0x20,' '-0x20,'O'-0x20,'V'-0x20, 'E'-0x20, 'R'-0x20,
    NT_UPD_EOF
};

void updateAttrs()
{
    //set color for background
    //See https://wiki.nesdev.com/w/index.php/PPU_attribute_tables for more details
    //on how this works
    static unsigned char attrBuffer[68];
    static unsigned char l;
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

void setTextAttrs()
{
    static unsigned char attrBuffer[68];
    static unsigned char l, k;

    attrBuffer[0] = 0x23|NT_UPD_HORZ;
    attrBuffer[1] = 0xC0;
    attrBuffer[2] = 64;
    for(l=0; l<16; ++l)
    {
        for(k=3; k<7; ++k)
        {
            attrBuffer[l*4 + k] = BGPAL0;
        }
    }
    attrBuffer[67] = NT_UPD_EOF;
    set_vram_update(attrBuffer);
    ppu_wait_frame();
}

void updateLineBricks(unsigned int line)
{
    //line is a number comprise beetween 0 and 30
    static unsigned char vramBuffer[36];
    static unsigned char l;
    static unsigned char maskPpu;
    vramBuffer[0] = MSB(NAMETABLE_A + (line<<5))|NT_UPD_HORZ; //When tile is filled, fill the one at its right
    vramBuffer[1] = LSB(NAMETABLE_A + (line<<5));
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
        }
        else
        {
            vramBuffer[l*4 +3]= 0x00;
            vramBuffer[l*4 +4]= 0x00;
            vramBuffer[l*4 +5]= 0x00;
            vramBuffer[l*4 +6]= 0x00;
        }
    }
    vramBuffer[35] = NT_UPD_EOF;

    set_vram_update(vramBuffer);
}


//get metatile code for given x,y level position in pixels, relative to current offset in the level
//funciton like this could be used for collision detection purposes
unsigned char checkCollisionAndRemoveBrick(signed short x, signed short y)
{
    //144 = 18 * 8
    static unsigned char brick, brickLine, brickBit, mask;

    brickLine = y>>3; // divide by 8
    brickBit = x>>5; // divide by 32

    mask = maskPpuBase>>brickBit;
    brick = levelData[brickLine]&mask;
    if(brick == mask && brickLine < sizeof(levelData))
    {
        mask = ~mask; //Invert mask
        levelData[brickLine] = levelData[brickLine]&mask;
        updateLineBricks(brickLine);
    }
    else
    {
        brick = NULL;
    }

    return brick;
}

// update the score and change the level if necessary. Return true in this case
unsigned char updateBrickCount()
{
    static int i;
    static unsigned char levelBuffer[10+1]={
        MSB(NTADR_A(12,12))|NT_UPD_HORZ,LSB(NTADR_A(12,12)),7,'L'-0x20,'E'-0x20,'V'-0x20,'E'-0x20,'L'-0x20,' '-0x20,'X'-0x20,
        NT_UPD_EOF
    };
    destroyedBrickCount++;
    if(destroyedBrickCount == bricksInLevel[currentLevel])
    {
        currentLevel++;
        if(currentLevel < 2)
        {
            setTextAttrs();//Set text attributes
            levelBuffer[9] = (currentLevel + 1 + 0x10); // magic number to convert unsigned level int to AISI char
            set_vram_update(levelBuffer);

            delay(60);
            memcpy(levelData, allLevels[currentLevel], 30);
            updateAttrs();
            for(i = 0; i < 30; ++i)
            {
                updateLineBricks(i);
                ppu_wait_frame();
            }
            destroyedBrickCount = 0;

        }
        else
        {
            setTextAttrs();//Set text attributes
            set_vram_update(winBuffer);
            delay(60);
            updateAttrs();
        }

        return TRUE;
    }
    return FALSE;
}

void userLost()
{
    static int i;
    static unsigned char levelBuffer[12+1]={
        MSB(NTADR_A(11,12))|NT_UPD_HORZ,LSB(NTADR_A(11,12)),9,' '-0x20, 'L'-0x20,'E'-0x20,'V'-0x20,'E'-0x20,'L'-0x20,' '-0x20,'1'-0x20, ' '-0x20,//Space at the begining and the end to fully erase game over
        NT_UPD_EOF
    };

    //Clear screen of remaining bricks
    memcpy(levelData, emptyLevelData, 30);
    for(i = 0; i < 30; ++i)
    {
        updateLineBricks(i);
        ppu_wait_frame();
    }

    setTextAttrs();//Set attribute for text
    set_vram_update(looseBuffer);//Show game over

    delay(400);
    set_vram_update(levelBuffer);//Show level 1
    delay(100);

    //Reset vars
    currentLevel = 0;
    destroyedBrickCount = 0;

    //Load level 1
    memcpy(levelData, allLevels[currentLevel], 30);
    updateAttrs();
    for(i = 0; i < 30; ++i)
    {
        updateLineBricks(i);
        ppu_wait_frame();
    }
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
    static unsigned char superBallCount = 0;
    static unsigned char sballBonus = FALSE;//Super ball bonus is active
    static unsigned char sballX, sballY;//Super ball bonus pos
    static unsigned char sbb_sprite = TRUE;//Super ball bonus sprite

    pal_spr(palette);//set sprite palette
    pal_bg(palette);//set background palette from an array
    pal_col(17,0x30);//white color for sprite

    sprite_x=128;
    sprite_y=120;

    memcpy(levelData, allLevels[currentLevel], 30);

    ppu_on_all();//enable rendering
    updateAttrs();
    for (i=0; i <30; ++i)
    {
        updateLineBricks(i);
        ppu_wait_frame();
    }

    delay(60);//delay to show the preloaded part of the level

    while(1)
    {
        ppu_wait_frame();

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
                ballVelocity = 20;//max 50
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
                // reset all values when we loose

                userLost();

                barX = 114;
                barY = 200;
                ballStuck = TRUE;
                ballX = 126<<4;
                ballY = 193<<4;
                ballVelocity = 0;
                ballDirection = 0;
                sballBonus = FALSE;
                superBallCount = 0;
                continue; // skip move of the ball when the level change
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
                xb1 = xa1>>5<<5;
                xb2 = xb1+32;
                yb1 = ya1>>3<<3;
                yb2 = yb1+8;
            }
            if(!tile)
            {
                tile=checkCollisionAndRemoveBrick(xa2,ya1);//Collide with brick on ball's top right corner
                if(tile)
                {
                    xb1 = xa2>>5<<5;
                    xb2 = xb1+32;
                    yb1 = ya1>>3<<3;
                    yb2 = yb1+8;
                }
            }
            if(!tile)
            {
                tile=checkCollisionAndRemoveBrick(xa1,ya2);//Collide with brick on ball's bottom left corner
                if(tile)
                {
                    xb1 = xa1>>5<<5;
                    xb2 = xb1+32;
                    yb1 = ya2>>3<<3;
                    yb2 = yb1+8;
                }
            }
            if(!tile)
            {
                tile=checkCollisionAndRemoveBrick(xa2,ya2);//Collide with brick on ball's bottom right corner
                if(tile)
                {
                    xb1 = xa2>>5<<5;
                    xb2 = xb1+32;
                    yb1 = ya2>>3<<3;
                    yb2 = yb1+8;
                }
            }
            if(tile)//If we collide with any brick we look where the ball is and bounce accordingly
            {
                if(rand8() > 230 && sballBonus == FALSE && superBallCount == 0)
                {
                    sballBonus = TRUE;
                    sballX = xb1+((xb2-xb1)>>1)-4;
                    sballY = yb1+((yb2-yb1)>>1)-4;
                }
                if(superBallCount > 0)
                {
                    superBallCount = superBallCount - 1;
                }
                else
                {
                    switch(ballDirection)
                    {
                        case 0:
                        case 8:
                            ballDirection = getCollisionBallDirection(ballDirection, TRUE);
                            break;
                        case 4:
                        case 12://Should not happend
                            ballDirection = getCollisionBallDirection(ballDirection, FALSE);
                            break;
                        case 1:
                        case 2:
                        case 3://Bottom right corner
                            if(xa2 > xb2 && (ya2 <= yb2 || ABS(xa1-xb2) < ABS(ya1-yb2)))//Right side; condition: (not totally over the brick && totally on the side of the brick || more on the side than the bottom)
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, FALSE);
                                ballX = xb2<<4;
                            }
                            else//Bottom side
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, TRUE);
                                ballY = yb2<<4;
                            }
                            break;
                        case 5:
                        case 6:
                        case 7://Top right corner
                            if(xa2 > xb2 && (ya1 >= yb1 || ABS(xa1-xb2) < ABS(ya2-yb1)))//Right side; condition: (not totally over the brick && totally on the side of the brick || more on the side than the top)
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, FALSE);
                                ballX = xb2<<4;
                            }
                            else//Top side
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, TRUE);
                                ballY = (yb1-8)<<4;
                            }
                            break;
                        case 9:
                        case 10:
                        case 11://Top left corner
                            if(xa1 < xb1 &&(ya1 >= yb1 || ABS(xa2-xb1) < ABS(ya2-yb1)))//Left side; condition: (not totally over the brick && totally on the side of the brick || more on the side than the top)
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, FALSE);
                                ballX = (xb1-8)<<4;
                            }
                            else//Top side
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, TRUE);
                                ballY = (yb1-8)<<4;
                            }
                            break;
                        case 13:
                        case 14:
                        case 15://Bottom left corner
                            if(xa1 < xb1 && (ya2 <= yb2 || ABS(xa2-xb1) < ABS(ya1-yb2)))//Left side; condition: (not totally over the brick && totally on the side of the brick || more on the side than the bottom)
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, FALSE);
                                ballX = (xb1-8)<<4;
                            }
                            else//Bottom side
                            {
                                ballDirection = getCollisionBallDirection(ballDirection, TRUE);
                                ballY = yb2<<4;
                            }
                            break;
                        default:
                            //The fuck? this is not possible
                            break;
                    }
                    if(updateBrickCount())
                    {
                        // reset all values when the level change
                        barX = 114;
                        barY = 200;
                        ballStuck = TRUE;
                        ballX = 126<<4;
                        ballY = 193<<4;
                        ballVelocity = 0;
                        ballDirection = 0;
                        sballBonus = FALSE;
                        superBallCount = 0;
                    }
                }
            }

            //We move the ball according to its direction and velocity
            ballX += getXMotionFromDirection(ballDirection, ballVelocity);
            ballY += getYMotionFromDirection(ballDirection, ballVelocity);
        }

        if(sballBonus == TRUE)
        {
            sballY += 1;
            if(sballY% 4 == 0)
            {
                sbb_sprite = (sbb_sprite + 1) % 2;
            }
            if(sballY > 250)
            {
                sballBonus = FALSE;
                sballX = -8;
            }
            if(sballY > 200 && sballY < 208)
            {
                if(sballX+8 > barX && sballX < barX+32)
                {
                    sballBonus = FALSE;
                    sballX = 0;
                    sballY = -8;
                    // enable superball for 8 to 24 block destruction (superball don't change its direction when hiting block)
                    superBallCount = rand8()%16 + 8;
                }
            }
        }

        /*
           Drawing
           */
        spr = 0;

        spr = oam_meta_spr(barX, barY, spr, barMetaSprite);//Draw the bar at its coordinates
        spr = oam_spr((unsigned char)(ballX>>4), (unsigned char)(ballY>>4), BALL_SPR, (superBallCount > 0 ? 0 : 3), spr);//Draw the ball at its coordinates (converted from units to pixels)
        spr = oam_spr(sballX, sballY, sbb_sprite == 0 ? SBALL_B_SPR : SBALL_B_SPR_2, 1, spr);
    }
}
