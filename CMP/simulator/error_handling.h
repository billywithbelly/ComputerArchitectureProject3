/*
	* author : billywithbelly
	*
	* error handling
	* 
	* write to register zero	: continue
	* number overflow	: continue
	* memery address overflow	: halt
	* data misaligned	: halt
	* 
	* return 1 if occurs, 
	* else return 0;
*/
#ifndef ERROR_HANELING_H
#define ERROR_HANELING_H

#include "objects.h"
#include "environment.h"

//functions
int writeToZero (int);
void numberOverflow (int, int, int);
int memoryAddressOverflow (int, int);
int dataMisaligned (int, int);

int writeToZero (int index)
{
	//register zero should constantly be zero
	//output "Write $0 Error", and then do nothing at this cycle
	if (index == 0)
	{
		registers[0] = 0;
		fprintf (error, "In cycle %d: Write $0 Error\n", numOfCycle);
		//fprintf (snapShot, "In cycle %d: Write $0 Error\n", numOfCycle);
		return 1;
	}
	else
		return 0;
}

void numberOverflow (int adder, int addee, int result)
{
	//when producing two same sign produces a different sign (i'd never thought about judging this way~)
	//so just compare bit no.31!
	if (adder>>31 == addee>>31)
	{
		if (adder >> 31 != result >> 31)
			fprintf (error, "In cycle %d: Number Overflow\n", numOfCycle);
	}
}

int memoryAddressOverflow (int index, int length)
{
	for (int i=0; i<length; i++)
		if (index < 0 || (index + i) < 0 || (index + i) > 1023)
		{
			fprintf (error, "In cycle %d: Address Overflow\n", numOfCycle);
			halt = 1;
			return 1;
		}
	
	return 0;
}

int dataMisaligned (int test, int byte)
{
	if (test%byte)
	{
		fprintf (error, "In cycle %d: Misalignment Error\n", numOfCycle);
		halt = 1;
		return 1;
	}
	else
		return 0;
}

#endif