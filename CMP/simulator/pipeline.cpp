#include "library.h"
#include "environment.h"

FILE *snapShot;
FILE *report;

int cycle, halt;
INST32 registers[32];
INST32 PC;
INST8 DMemory[MAX_LENGTH],IMemory[MAX_LENGTH];

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
LRUQueue *queuedTLB, *queuedMemory;
setEntry *cacheSet;
resultRecord *resultPTE, *resultTLB, *resultCache;

void initial ();

int main ()
{
	initial ();
	loadImage (IMemory, DMemory);
	return 0;
}

void initial ()
{
	cycle = 0;
	halt = 0;
	for (int i=0; i<32; i++)
		registers[i] = 0;
	PC = 0;
	for (int i=0; i<MAX_LENGTH; i++){
		DMemory[i] = 0;
		IMemory[i] = 0;
	}
	numBlock = 0;
	numSet = 0;
	PTESize = 0;
	TLBSize = 0;
	PPNMAX = 0;
 	PPNIndex = 0;
	TLBIndex = 0;
	memorySize = 0;
	diskSize = 0;
	pageSize = 0;
	cacheSize = 0;
	blockSize = 0;
	nWay = 0;
	for (int i=0; i<1000000; i++) {
		DAddress[i] = 0;
		DCycle[i] = 0;
		IAddress[i] = 0;
	}
	DAddressLength = 0;
	IAddressLength = 0;

	for (int i=0; i<20; i++)
		command[i] = 0;

	PTE = new PTEEntry ();
	TLB = new TLBEntry ();
	cacheBlock = new blockEntry ();
	queuedTLB = new LRUQueue ();
	queuedMemory = new LRUQueue ();
	cacheSet = new setEntry ();
	resultPTE = new resultRecord ();
	resultTLB = new resultRecord ();
	resultCache = new resultRecord ();

	snapShot = fopen ("snapshot.rpt", "w");
	report = fopen ("report.rpt", "w");
}
