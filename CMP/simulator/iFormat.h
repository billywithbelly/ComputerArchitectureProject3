/*
	* author : billywithbelly
	*
	* getting instructions for main.cycle.instructionDecoder
	* realizing all i format functions
*/
#ifndef IFORMAT_H
#define IFORMAT_H

#include <iostream>

#include "objects.h"
#include "environment.h"
#include "error_handling.h"

//define opcodes
#define ADDI 0x08
#define ADDIU 0x09
#define LW 0x23
#define LH 0x21
#define LHU 0x25
#define LB 0x20
#define LBU 0x24
#define SW 0x2b
#define SH 0x29
#define SB 0x28
#define LUI 0x0f
#define ANDI 0x0c
#define ORI 0x0d
#define NORI 0x0e
#define SLTI 0x0a
#define BEQ 0x04
#define BNE 0x05
#define BGTZ 0x07

using namespace std;

INST32 loadMemory (int, int, int);
void saveMemory (int, int, INST32);

//functions
void f_addi (int rs, int rt, int imm)
{
	int d = writeToZero (rt);
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	if (d == 0)
		registers[rt] = registers[rs] + imm;
}
void f_addiu (int rs, int rt, int imm)
{
	if (writeToZero (rt))
		return ;
	else
		registers[rt] = registers[rs] + imm;	
}
void f_lw (int rs, int rt, int imm)
{
	int d = writeToZero (rt);
	//since we have to print all errors, place them in correct patterns
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	//four byte per word
	memoryAddressOverflow (registers[rs] + imm, 4);
	dataMisaligned (registers[rs] + imm, 4);
	if (d)
		return ;
	else if (!halt)
		registers[rt] = loadMemory (registers[rs] + imm, 4, 0);
}
void f_lh (int rs, int rt, int imm)
{
	int d = writeToZero (rt);
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	//two byte per half word
	memoryAddressOverflow (registers[rs] + imm, 2);
	dataMisaligned (registers[rs] + imm, 2);
	if (d)
		return ;
	else if (!halt)
		registers[rt] = loadMemory (registers[rs] + imm, 2, 1);
}
void f_lhu (int rs, int rt, int imm)
{
	int d = writeToZero (rt);
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	//unsigned
	memoryAddressOverflow (registers[rs] + imm, 2);
	dataMisaligned (registers[rs] + imm, 2);
	if (d)
		return ;
	else if (!halt)
		registers[rt] = loadMemory (registers[rs] + imm, 2, 0);
}
void f_lb (int rs, int rt, int imm)
{
	int d = writeToZero (rt);
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	memoryAddressOverflow (registers[rs] + imm, 1);
	dataMisaligned (registers[rs] + imm, 1);
	if (d)
		return ;
	else if (!halt)
		registers[rt] = loadMemory (registers[rs] + imm, 1, 1);

}
void f_lbu (int rs, int rt, int imm)
{
	int d = writeToZero (rt);
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	memoryAddressOverflow (registers[rs] + imm, 1);
	dataMisaligned (registers[rs] + imm, 1);
	if (d)
		return ;
	else if (!halt)
		registers[rt] = loadMemory (registers[rs] + imm, 1, 0);

}
void f_sw (int rs, int rt, int imm)
{	
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	memoryAddressOverflow (registers[rs] + imm, 4);
	dataMisaligned (registers[rs] + imm, 4);
	if (!halt)
		saveMemory (registers[rs] + imm, 4, registers[rt]);

}
void f_sh (int rs, int rt, int imm)
{
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	memoryAddressOverflow (registers[rs] + imm, 2);
	dataMisaligned (registers[rs] + imm, 2);
	
	if (!halt)
		saveMemory (registers[rs] + imm, 2, registers[rt]);
}
void f_sb (int rs, int rt, int imm)
{
	numberOverflow (registers[rs], imm, registers[rs] + imm);
	memoryAddressOverflow (registers[rs] + imm, 1);
	
	if (!halt)
		saveMemory (registers[rs] + imm, 1, registers[rt]);
}
void f_lui (int rs, int rt, int imm)
{
	if (writeToZero (rt))
		return ;
	else
		registers[rt] = imm << 16;
}
void f_andi (int rs, int rt, int imm)
{	
	if (writeToZero (rt))
		return ;
	else
		registers[rt] = registers[rs] & imm;
}
void f_ori (int rs, int rt, int imm)
{
	if (writeToZero (rt))
		return ;
	else
		registers[rt] = registers[rs] | imm;
}
void f_nori (int rs, int rt, int imm)
{
	if (writeToZero (rt))
		return ;
	else
		registers[rt] = ~ (registers[rs] | imm);
}
void f_slti (int rs, int rt, int imm)
{
	if (writeToZero (rt))
		return ;
	else
	{
		if ((signed int)registers[rs] < (signed int)imm)
			registers[rt] = 1;
		else
			registers[rt] = 0;
	}
}
void f_beq (int rs, int rt, int imm)
{
	if (registers[rt] == registers[rs])
	{
		numberOverflow (PC, (imm << 2), PC + 4 * imm);
		PC += 4 * imm;
	}
}
void f_bne (int rs, int rt, int imm)
{
	if (registers[rt] != registers[rs])
	{
		numberOverflow (PC, (imm << 2), PC + 4 * imm);
		PC += 4 * imm;
	}
}
void f_bgtz (int rs, int rt, int imm)
{
	if (registers[rt] > 0)
	{
		numberOverflow (PC, (imm << 2), PC + 4 * imm);
		PC += 4 * imm + 4;
	}
}

//decoding functions
void iFunctionDecoder (int opcode, int rs, int rt, int imm, int signed_imm)
{
	if (opcode == ADDI)
		f_addi (rs, rt, signed_imm);
	else if (opcode == ADDIU)
		f_addiu (rs, rt, imm);
	else if (opcode == LW)
		f_lw (rs, rt, signed_imm);
	else if (opcode == LH)
		f_lh (rs, rt, signed_imm);
	else if (opcode == LHU)
		f_lhu (rs, rt, signed_imm);
	else if (opcode == LB)
		f_lb (rs, rt, signed_imm);
	else if (opcode == LBU)
		f_lbu (rs, rt, signed_imm);
	else if (opcode == SW)
		f_sw (rs, rt, signed_imm);
	else if (opcode == SH)
		f_sh (rs, rt, signed_imm);
	else if (opcode == SB)
		f_sb (rs, rt, signed_imm);
	else if (opcode == LUI)
		f_lui (rs, rt, imm);
	else if (opcode == ANDI)
		f_andi (rs, rt, imm);
	else if (opcode == ORI)
		f_ori (rs, rt, imm);
	else if (opcode == NORI)
		f_nori (rs, rt, imm);
	else if (opcode == SLTI)
		f_slti (rs, rt, signed_imm);
	else if (opcode == BEQ)
		f_beq (rs, rt, signed_imm);
	else if (opcode == BNE)
		f_bne (rs, rt, signed_imm);
	else if (opcode == BGTZ)
		f_bgtz (rs, rt, signed_imm);
}

//load memory
INST32 loadMemory (int location, int byte, int sign)
{
	int tmpt = 0;
	int basis;
	
	DAddress[DAddressLength++] = location;
	basis = DMemory[location] >> 7;
	for (int i=0; i<byte; i++)
	{
		//get word
		tmpt = tmpt << 8;
		tmpt = tmpt | DMemory[location + i];
	}
	if (sign && basis)
	{
		if (byte != 4)
			tmpt = tmpt | (0xffffffff << (byte * 8));
	}
	
	return tmpt;
}

//save memory
void saveMemory (int location, int byte, INST32 object)
{
	//input byte to control all save functions
	DAddress[DAddressLength++] = location;
	for (int i=byte - 1; i>=0; i--)
	{
		int tmpt = 0;
		tmpt = object << 24;
		tmpt = tmpt >> 24;
		DMemory[location + i] = (INST8) tmpt;
		object = object >> 8;
	}
}

#endif