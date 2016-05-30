/*
	* author : billywithbelly
	*
	* getting instructions for main.cycle.instructionDecoder
	* realizing all r format functions
	* 
	* this is f*king tiring
*/
#ifndef RFORMAT_H
#define RFORMAT_H

#include <iostream>

#include "objects.h"
#include "environment.h"
#include "error_handling.h"

//defining function codes
#define ADD 0X20
#define ADDU 0x21
#define SUB 0x22
#define AND 0x24
#define OR 0x25
#define XOR 0x26
#define NOR 0x27
#define NAND 0x28
#define SLT 0x2A
#define SLL 0x00
#define SRL 0x02
#define SRA 0x03
#define JR 0x08

using namespace std;

//funcitons
void f_add (int rs, int rt, int rd)
{
	int d = writeToZero(rd);
	numberOverflow (registers[rs], registers[rt], registers[rs] + registers[rt]);
	registers[rd] = registers[rs] + registers[rt];
}
void f_addu (int rs, int rt, int rd)
{
	int d = writeToZero (rd);
	registers[rd] = registers[rs] + registers[rt];
}
void f_sub (int rs, int rt, int rd)
{
	int a = registers[rs];
	int b = registers[rt];
	int d = writeToZero (rd);
	numberOverflow (a, -b, a-b);
	if (d)
		return;
	registers[rd] = registers[rs] - registers[rt];
}
void f_and (int rs, int rt, int rd)
{
	if (writeToZero (rd))
		return ;
	registers[rd] = registers[rs] & registers[rt];
}
void f_or (int rs, int rt, int rd)
{
	if (writeToZero (rd))
		return ;
	registers[rd] = registers[rs] | registers[rt];
}
void f_xor (int rs, int rt, int rd)
{
	if (writeToZero(rd))
		return ;
	registers[rd] = registers[rs] ^ registers[rt];
}
void f_nor (int rs, int rt, int rd)
{
	if (writeToZero (rd))
		return ;
	registers[rd] = ~ (registers[rs] | registers[rt]);
}
void f_nand (int rs, int rt, int rd)
{
	if (writeToZero (rd))
		return ;
	registers[rd] = ~ (registers[rs] & registers[rt]);
}

void f_slt (int rs, int rt, int rd)
{
	if (writeToZero (rd))
		return ;
	if ((signed int)registers[rs] < (signed int)registers[rt])
		registers[rd] = 1;
	else
		registers[rd] = 0;
}
void f_sll (int rt, int rd, int shamt)
{
	int d = 0;
	
	if (rt == 0 && rd == 0 && shamt == 0)
		d = 0;
	else if (writeToZero (rd))
		return ;
	
	
	if (shamt == 32)
		registers[rd] = 0;
	else if (shamt == 0)
		registers[rd] = registers[rt];
	else
		registers[rd] = registers[rt] << shamt;
		
}
void f_srl (int rt, int rd, int shamt)
{
	if (writeToZero(rd))
		return ;
	else 
	{
		if (shamt == 32)
			registers[rd] = 0;
		else if (shamt == 0)
			registers[rd] = registers[rt];
		
		else
			registers[rd] = registers[rt] >> shamt;
	}
}
void f_sra (int rt, int rd, int shamt)
{
	int sign = 0;
	
	if (writeToZero(rd))
		return ;
	else
	{
		if (shamt == 32)
			registers[rd] = 0x00000000;
		else if (shamt == 0)
			registers[rd] = registers[rt];
		else
		{
			int tmpt = registers[rt] >> 31;
			if (tmpt)
				sign = 1;
			registers[rd] = registers[rt] >> shamt;
			if (sign != 0 && shamt != 0)
				registers[rd] = registers[rd] | (unsigned int) (0xffffffff << (31 - shamt));
		}
	}
}
void f_jr (int rs, int rt, int rd)
{
	PC = registers[rs];
}

//decoding functions
void rFunctionDecoder (int rs, int rt, int rd, int shamt, int func)
{
	if (func == ADD)
		f_add (rs, rt, rd);
	else if (func == ADDU)
		f_addu (rs, rt, rd);
	else if (func == SUB)
		f_sub (rs, rt, rd);
	else if (func == AND)
		f_and (rs, rt, rd);
	else if (func == OR)
		f_or (rs, rt, rd);
	else if (func == XOR)
		f_xor (rs, rt, rd);
	else if (func == NOR)
		f_nor (rs, rt, rd);
	else if (func == NAND)
		f_nand (rs, rt, rd);
	else if (func == SLT)
		f_slt (rs, rt, rd);
	else if (func == SLL)
		f_sll (rt, rd, shamt);
	else if (func == SRL)
		f_srl (rt, rd, shamt);
	else if (func == SRA)
		f_sra (rt, rd, shamt);
	else if (func == JR)
		f_jr (rs, rt, rd);
	else
		printf ("undefined function\n");
}
#endif