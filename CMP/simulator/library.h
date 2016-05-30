#ifndef LIBRARY_H
#define LIBRARY_H

#define MAX_LENGTH 1024

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
	LRUQueue LRUArray;

	setEntry ()
	{
		index = 0;
		//&LRUArray = new LRUQueue();
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