/*
	* author : billywithbelly
	* 
	* Computer Architecture project 1, simulator
	* main structure
	* 
	* basic design :
	* a while loop running cycles, and do functions in cycles
*/

//c++ libraries
#include <iostream>
#include <string>

//my finctions
#include "environment.h"
#include "objects.h"
#include "messy.h"


using namespace std;

int main (int argc, char *argv[])
{
	if (argc == 11)
		for (int i=0; i<argc; i++)
			command[i] = atoi(argv[i]);
	//the main file is the structure of my design.
	snapShot = fopen (toS, "w");
	error = fopen (toE, "w");
	report = fopen("report.rpt", "w");
	initialize ();
	load_I ();
	load_D ();
	producingSnapShot ();
	numOfCycle += 1;
	while (!halt)//a cycle() funtion be put in a while loop to control and practice every cycle
		cycle ();

	produceReport ();

	return 0;
}