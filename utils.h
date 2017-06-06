
#include "neslib.h"

#define ABS(x1) (x1<0?x1*-1:x1)

//return the value to add to he ball X position
signed short getXMotionFromDirection(unsigned char direction, unsigned char velocity);

signed short getYMotionFromDirection(unsigned char direction, unsigned char velocity);

unsigned char getCollisionBallDirection(unsigned char direction, unsigned char surfaceIsHorizontal);
