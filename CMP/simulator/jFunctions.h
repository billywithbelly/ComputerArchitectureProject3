/*
	* author : billywithbelly
	*
	* getting instructions for main.cycle.instructionDecoder
	* realizing all j format functions
*/
#ifndef JFUNCTIONS_H
#define JFUNCTIONS_H

#include <iostream>

#include "objects.h"
#include "environment.h"
#include "error_handling.h"

using namespace std;

//decoding functions
void jump (INST32 tmpt1, INST32 tmpt2)
{
	tmpt2 = tmpt2 << 2;
	PC = tmpt1 | tmpt2;
}

#endif
