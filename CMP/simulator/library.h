#ifndef LIBRARY_H
#define LIBRARY_H

#include <iostream>

#define RFORMAT 0X00
#define MAX_LENGTH 1023

//define i format instructions
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

//define j format instructions
#define J 0x02
#define JAL 0x03

//define r format instructions
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

//define halt
#define HALT 0x3F

typedef unsigned int INST32;
typedef unsigned char INST8;

class PTEEntry
{
public:
	int validBits;
	INST32 PPN;

	PTEEntry ()
	{
		validBits = 0;
		PPN = 0X00000000;
	}
};

class TLBEntry
{
public:
	int validBits;
	INST32 VPN;
	INST32 PPN;

	TLBEntry ()
	{
		validBits = 0;
		VPN = 0X00000000;
		PPN = 0X00000000;
	}
};

class blockEntry
{
public:
	int validBits;
	INST32 tags;

	blockEntry ()
	{
		validBits = 0;
		tags = 0X00000000;
	}
};

class LRUQueue
{
public:
	int queue[MAX_LENGTH];
	int length;

	LRUQueue ()
	{
		for (int i=0; i<MAX_LENGTH; i++)
			queue[i] = 0;
		length = 0;
	}
};

class setEntry
{
public:
	int index;
	LRUQueue *LRUArray;

	setEntry ()
	{
		index = 0;
		LRUArray = new LRUQueue();
	}
};

class resultRecord
{
public:
	int hit;
	int miss;

	resultRecord ()
	{
		hit = 0;
		miss = 0;
	}
};

#endif