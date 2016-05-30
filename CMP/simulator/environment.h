/*
	* author : billywithbelly
	*
	* environment :
	* environment settings
	* I/O
	* background works
	* probably the hardest part....
	* 
	* initialization
	* load_I
	* load_D
	* producingSnapShot
	* instructionfetcher
	* slicingInstructions
	* cycle
*/
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <iostream>
#include <cstdio>

#include "objects.h"
#include "error_handling.h"
#include "iFormat.h"
#include "rFormat.h"
#include "jFunctions.h"

using namespace std;

//functions
void initialization ();
void load_I ();
void load_D ();
void producingSnapShot (int);
INST32 instructionFetcher (INST32);
int instructionfragmentation (INST32, int, int);
int signedInstructionFragmentation (INST32, int, int);
void instructionPracticer (INST32 instruction);
void cycle ();

//initialization
void initialize ()
{
	PC = 0;
	DAddressLength = 0;
	IAddressLength = 0;
	for (int i=0; i<1000000; i++)
	{
		DAddress[i] = 0;
		IAddress[i] = 0;
	}
	for (int i=0; i<32; i++)
		registers[i] = 0;
	for (int i=0; i<MAX_LENGTH; i++)
	{
		IMemory[i] = 0;
		DMemory[i] = 0;
	}
	return ;
}

//load iimage
void load_I ()
{
	FILE *file;
	INST32 instructions;
	INST8 word_buffer[4];
	int state = 0;
	int instruction_in_IMemory = 0;
	int numOfInst = 0;
	
	/*
		sum = 0;
		for (int i=0; i<3; i++)
			sum += i;
		
		* size of sum and i are words
		* PC initially be zero, $sp initially be 0x400
	*/
	
	//open iimage
	file = fopen (toI, "rb");
	//return if no iimage, and the put error message
	if (!file)
	{
		halt = 1;
		fprintf (error, "iimage not found...\n");
		return;
	}
	
	while (fread(word_buffer, sizeof(char), 4, file))	//read a word
		//4 bytes at once
	{
		instructions = 0;
		for (int i=0; i<4; i++)//big endian
		{
			//getting instructions by shifting and doing or operation
			//because i want an entire 4 byte word PC / inst_num
			instructions = instructions << 8;
			instructions = instructions | word_buffer[i];
		}
		
		//there are three states
		switch (state)
		{
			//其實發現有點不懂 break 在 switch 裡面的意義
			//
			case 0:
				//first four bytes for PC
				PC = instructions;
				//printf ("%08x\n", instructions);
				state = 1;
				break;
			case 1:
				//next four be number of words
				numOfInst = instructions;
				//printf ("%08x\n", instructions);
				state = 2;
				break;
			case 2:
				//read instructions
				numOfInst -= 1;
				//printf ("%08x\n", instructions);
				for (int i=0; i<4; i++)
					IMemory[PC + instruction_in_IMemory++] = word_buffer[i];
					//filling the IMemory sequentially from PC
				//return when finishing the instructions
				if (numOfInst <= 0)
					break;
					//return;
				break;
		}
	}
	
	return ;
}

//load dimage
void load_D ()
{
	FILE *file;
	INST32 instructions;
	INST8 word_buffer[4];
	int state = 0;
	int instruction_in_DMemory = 0;
	int numOfMem = 0;
	
	//open Dimage
	file = fopen (toD, "rb");
	//return if no Dimage, and the put error message
	if (!file)
	{
		halt = 1;
		//fprintf (error, "Dimage not found...\n");
		return;
	}
	
	while (fread(word_buffer, sizeof(char), 4, file))	//read a word
		//4 bytes at once
	{
		instructions = 0;
		for (int i=0; i<4; i++)
		{
			instructions = instructions << 8;
			instructions = instructions | word_buffer[i];
		}
		switch (state)
		{
			case 0:
				registers[29] = instructions;
				//printf ("--------------\n%08x\n", instructions);
				state = 1;
				//$sp done
				break;
			case 1:
				numOfMem = instructions;
				//printf ("%08x\n", instructions);
				state = 2;
				break;
			case 2:
				//read instructions
				numOfMem -= 1;
				//printf ("%08x\n", instructions);
				for (int i=0; i<4; i++)
					DMemory[instruction_in_DMemory++] = word_buffer[i];
					//filling the DMemory sequentially from 0
				//return when finishing the instructions
				if (numOfMem <= 0)
					return;
				break;
		}
	}
		
	return ;
}

//snapShot
void producingSnapShot ()
{
	fprintf (snapShot, "cycle %d\n", numOfCycle);
	for (int i=0; i<32; i++)
		fprintf (snapShot, "$%02d: 0x%08X\n", i, registers[i]);
	IAddress[IAddressLength++] = PC;
	fprintf (snapShot, "PC: 0x%08X\n\n\n", PC);// end-line three times?
}

//decoding the instructions
//originally doing decoding, now fetching for next instructions
INST32 instructionFetcher (INST32 PC)
{
	INST32 instruction = 0;
	
	//get the new instruction
	//from memory
	for (int i=0; i<4; i++)
	{
		instruction = instruction << 8;
		instruction = instruction | IMemory[PC + i];
	}

	return instruction;
}

//practicing the instructions
void instructionPracticer (INST32 instruction)
{
	int opcode;
	opcode = instructionfragmentation (instruction, 31, 26);

	//do functions finally
	//for debug
	//i formats' opcodes are so complicated...
	if (opcode == 0x00)
	{
		//fprintf (error, "rFormat\n");
		//r format
		int rs = instructionfragmentation (instruction, 25, 21);
		int rt = instructionfragmentation (instruction, 20, 16);
		int rd = instructionfragmentation (instruction, 15, 11);
		int shamt = instructionfragmentation (instruction, 10, 6);
		int func = instructionfragmentation (instruction, 5, 0);
		rFunctionDecoder (rs, rt, rd, shamt, func);
	}
	else if (opcode == 0x02)
	{
		//jump
		//PC += 4;
		INST32 tmpt1 = instructionfragmentation (PC, 31, 28);
		tmpt1 = tmpt1 << 26;
		//address : 
		INST32 tmpt2 = instructionfragmentation (instruction, 25, 0);
		jump (tmpt1, tmpt2);
		
	}
	else if (opcode == 0x03)
	{
		//jal
		//PC += 4;
		registers[31] = PC;
		int tmpt1 = instructionfragmentation (PC, 31, 28);
		tmpt1 = tmpt1 << 28;
		int tmpt2 = instructionfragmentation (instruction, 25, 0);
		jump (tmpt1, tmpt2);
		
	}
	else if (opcode == 0x3f)
	{
		halt = 1;
		return ;
	}
	else
	{
		//i format
		int opcode = instructionfragmentation (instruction, 31, 26);
		int rs = instructionfragmentation (instruction, 25, 21);
		int rt = instructionfragmentation (instruction, 20, 16);
		int imm = instructionfragmentation (instruction, 15, 0);
		int signed_imm = signedInstructionFragmentation (instruction, 15, 0);
		iFunctionDecoder (opcode, rs, rt, imm, signed_imm);
	}

}

//slicingInstructions
int instructionfragmentation (INST32 instruction, int head, int tail)
{
	INST32 tmpt = instruction;
	tmpt = tmpt << (31 - head);
	tmpt = tmpt >> (31-head+tail);
	
	return tmpt;
}

int signedInstructionFragmentation (INST32 instruction, int head, int tail)
{
	int sign = instruction << (31 - head);
	sign = sign >> 31;
	int fragment = instruction << (31 - head);
	fragment = fragment >> (31 - head + tail);
	
	if (sign == 1)
		fragment = fragment | (0xffffffff << (head - tail + 1));
	
	return fragment;
}

//cycle
void cycle ()
{
	INST32 instruction = 0;
	//numOfCycle += 1;
	//basic error handling
	memoryAddressOverflow (PC, 1);
	dataMisaligned (PC, 4);
	if (numOfCycle == 500000)
	{
		halt = 1;
		printf ("Number of cycles > 500,000\n");
	}
	//if halt, return~
	if (halt)
		return;
	
	instruction = instructionFetcher (PC);
	PC += 4;
	instructionPracticer (instruction);
	registers[0] = 0;
	if (!halt)
		producingSnapShot ();
	numOfCycle += 1;
}

#endif