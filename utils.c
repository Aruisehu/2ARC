#include "utils.h"

signed short getXMotionFromDirection(unsigned char direction, unsigned char velocity)
{
	switch(direction)
	{
		case 0:
		case 8:
			return 0;
		case 1:
		case 7:
			return (signed short)(((signed short)-98*(signed short)velocity) >> (signed short)8);
		case 2:
		case 6:
			return (signed short)(((signed short)-181*(signed short)velocity) >> (signed short)8);
		case 3:
		case 5:
			return (signed short)(((signed short)-236*(signed short)velocity) >> (signed short)8);
		case 4:
			return -(signed short)velocity;
		case 9:
		case 15:
			return (signed short)(((signed short)98*(signed short)velocity) >> (signed short)8);
		case 10:
		case 14:
			return (signed short)(((signed short)181*(signed short)velocity) >> (signed short)8);
		case 11:
		case 13:
			return (signed short)(((signed short)236*(signed short)velocity) >> (signed short)8);
		case 12:
			return (signed short)velocity;
		default:
			return 0;
	}
}

signed short getYMotionFromDirection(unsigned char direction, unsigned char velocity)
{
	switch(direction)
	{
		case 4:
		case 12:
			return 0;
		case 0:
			return -(signed short)velocity;
		case 1:
		case 15:
			return (signed short)(((signed short)-236*(signed short)velocity) >> (signed short)8);
		case 2:
		case 14:
			return (signed short)(((signed short)-181*(signed short)velocity) >> (signed short)8);
		case 3:
		case 13:
			return (signed short)(((signed short)-98*(signed short)velocity) >> (signed short)8);
		case 5:
		case 11:
		    return (signed short)(((signed short)98*(signed short)velocity) >> (signed short)8);
	    case 6:
	    case 10:
		    return (signed short)(((signed short)181*(signed short)velocity) >> (signed short)8);
		case 7:
		case 9:
			return (signed short)(((signed short)236*(signed short)velocity) >> (signed short)8);
		case 8:
			return (signed short)velocity;
	}
}

unsigned char getCollisionBallDirection(unsigned char direction, unsigned char surfaceIsHorizontal)
{
	switch(direction)
	{
		case 0:
			return 8;
		case 1:
			return surfaceIsHorizontal == TRUE ? 7 : 15;
		case 2:
			return surfaceIsHorizontal == TRUE ? 6 : 14;
		case 3:
			return surfaceIsHorizontal == TRUE ? 5 : 13;
		case 4:
			return 12;
		case 5:
			return surfaceIsHorizontal == TRUE ? 3 : 11;
		case 6:
			return surfaceIsHorizontal == TRUE ? 2 : 10;
		case 7:
			return surfaceIsHorizontal == TRUE ? 1 : 9;
		case 8:
			return 0;
		case 9:
			return surfaceIsHorizontal == TRUE ? 15 : 7;
		case 10:
			return surfaceIsHorizontal == TRUE ? 14 : 6;
		case 11:
			return surfaceIsHorizontal == TRUE ? 13 : 5;
		case 12:
			return 4;
		case 13:
			return surfaceIsHorizontal == TRUE ? 11 : 3;
		case 14:
			return surfaceIsHorizontal == TRUE ? 10 : 2;
		case 15:
			return surfaceIsHorizontal == TRUE ? 9 : 1;
		default:
			return 0;
	}
}