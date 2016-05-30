/*
	* author : billywithbelly
	*
	* objects:
	* a header that contains all members
*/
#ifndef OBJECTS_H
#define OBJECTS_H

#include "library.h"

#define toI "iimage.bin"
#define toD "dimage.bin"
#define toS "snapshot.rpt"
#define toE "error_dump.rpt"


//file read/write
FILE *snapShot, *error, *PCrpt, *Datarpt, *report;
int numOfCycle = 0;
int halt = 0;
INST32 registers[32];
INST32 PC;
INST8 DMemory[MAX_LENGTH], IMemory[MAX_LENGTH];


int numBlock,numSet;
int PTESize,TLBSize;
int PPNMAX;
int PPNIndex;
int TLBIndex;
INST32 *PPNtoVPN,*PPNtoTLBindex;

int memorySize;
int diskSize;
int pageSize;

int cacheSize;
int blockSize;
int nWay;

INST32 DAddress[1000000];
INST32 DCycle[1000000];
INST32 IAddress[1000000];
int DAddressLength;
int IAddressLength;

int command[20];

PTEEntry *PTE;
TLBEntry *TLB;
blockEntry *cacheBlock;
LRUQueue queuedTLB, queuedMemory;
setEntry *cacheSet;
resultRecord *resultPTE, *resultTLB, *resultCache;


#endif