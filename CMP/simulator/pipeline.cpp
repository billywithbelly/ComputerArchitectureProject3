#include "library.h"
#include "environment.h"

FILE *snapShot;
FILE *report;

int cycle;
bool halt;
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
void singleCycle ();
void produceReport();

int checkTLB (INST32);
void checkCache(int);

int main ()
{
    initial ();
    loadImage (PC, registers, IMemory, DMemory);
    produceSnapShot(PC, snapShot, registers, cycle++);
    
    while (!halt)
        singleCycle ();
        
    
    return 0;
}

void initial ()
{
    cycle = 0;
    halt = false;
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

void singleCycle ()
{
    if (!halt)
        return;
    INST32 instruction = instructionFetcher(PC, IMemory);
    PC = PC+4;
    instructionPractice(instruction);

    registers[0]=0;
    if (!halt)
        produceSnapShot(PC, snapShot, registers, cycle++);
}

void produceReport ()
{
    int count;
    int tmpt;
    int ICache_hits, ICache_misses,
        DCache_hits, DCache_misses,
        ITLB_hits, ITLB_misses,
        DTLB_hits, DTLB_misses,
        IPageTable_hits, IPageTable_misses,
        DPageTable_hits, DPageTable_misses;
    
    for(count=0; count<2; count++)
    {
        if(count == 0)
        {
            //memory size
            if(command[1] == 0)
                memorySize=64;
            else 
                memorySize=(command[1]);
            
            //disk size
            diskSize = 1024;
            
            //page size
            if(command[3] == 0)
                pageSize = 8;
            else 
                pageSize = command[3];
            
            //cache size
            if(command[5] == 0)
                cacheSize = 16;
            else 
                cacheSize = command[5];
                
            //block size
            if(command[6] == 0)
                blockSize = 4;
            else 
                blockSize = command[6];
                
            //n ways
            if(command[7] == 0)
                nWay = 4;
            else 
                nWay = command[7];
/*
            printf("I_memory_size = %d\n",memory_size);
            printf("I_disk_size = %d\n",disk_size);
            printf("I_page_size = %d\n",page_size);
            printf("I_cache_size = %d\n",cache_size);
            printf("I_block_size = %d\n",block_size);
            printf("I_n_way = %d\n",n_way);
*/
        }
        else
        {
            if(command[2] == 0)
                memorySize = 32;
            else 
                memorySize = command[2];
            
            diskSize = 1024;
            
            if(command[4] == 0)
                pageSize = 16;
            else 
                pageSize = command[4];
            
            if(command[8] == 0)
                cacheSize = 16;
            else
                cacheSize = command[8];
                
            if(command[9] == 0)
                blockSize = 4;
            else
                blockSize = command[9];
            
            if(command[10] == 0)
                nWay = 1;
            else
                nWay = command[10];
/*
            printf("D_memory_size = %d\n",memory_size);
            printf("D_disk_size = %d\n",disk_size);
            printf("D_page_size = %d\n",page_size);
            printf("D_cache_size = %d\n",cache_size);
            printf("D_block_size = %d\n",block_size);
            printf("D_n_way = %d\n",n_way);
*/
        }
        
        PTESize = diskSize / pageSize;
        TLBSize = PTESize / 4;
        PPNMAX = memorySize / pageSize;
        PPNIndex = 0;
        TLBIndex = 0;

        numBlock = cacheSize / blockSize;
        numSet = numBlock / nWay;

        PTE = new PTEEntry[PTESize] ();
        TLB = new TLBEntry[TLBSize] ();
        PPNtoTLBindex = new INST32[TLBSize];
        PPNtoVPN = new INST32[PPNMAX];
        
        for (int i=0; i<TLBSize; i++)
            PPNtoTLBindex[i] = 0;
        for (int i=0; i<PPNMAX; i++)
            PPNtoVPN[i] = 0;
        
        cacheBlock = new blockEntry[numBlock] ();
        cacheSet = new setEntry[numSet] ();

        queuedMemory = new LRUQueue();
        queuedTLB = new LRUQueue();
        
        if(count == 0)
        {
            for (int i=0; i<IAddressLength;i++)
            {
                tmpt = checkTLB(IAddress[i]);
                checkCache(tmpt);
                //printf("V_add -> P_add = 0x%08x -> 0x%08x",I_address[i],tmp);
                //putchar('\n');
            }
            ICache_hits = resultCache->hit;
            ICache_misses = resultCache->miss;
            ITLB_hits = resultTLB->hit;
            ITLB_misses = resultTLB->miss;
            IPageTable_hits = resultPTE->hit;
            IPageTable_misses = resultPTE->miss;
        }
        else
        {
            for (int i=0; i<DAddressLength;i++)
            {
                tmpt = checkTLB(DAddress[i]);
                checkCache(tmpt);
            }
            DCache_hits = resultCache->hit;
            DCache_misses = resultCache->miss;
            DTLB_hits = resultTLB->hit;
            DTLB_misses = resultTLB->miss;
            DPageTable_hits = resultPTE->hit;
            DPageTable_misses = resultPTE->miss;
        }
    }
    
    fprintf(report,"ICache :\n");
    fprintf(report,"# hits: %d\n",ICache_hits);
    fprintf(report,"# misses: %d\n\n",ICache_misses);

    fprintf(report,"DCache :\n");
    fprintf(report,"# hits: %d\n",DCache_hits);
    fprintf(report,"# misses: %d\n\n",DCache_misses);

    fprintf(report,"ITLB :\n");
    fprintf(report,"# hits: %d\n",ITLB_hits);
    fprintf(report,"# misses: %d\n\n",ITLB_misses);

    fprintf(report,"DTLB :\n");
    fprintf(report,"# hits: %d\n",DTLB_hits);
    fprintf(report,"# misses: %d\n\n",DTLB_misses);

    fprintf(report,"IPageTable :\n");
    fprintf(report,"# hits: %d\n",IPageTable_hits);
    fprintf(report,"# misses: %d\n\n",IPageTable_misses);

    fprintf(report,"DPageTable :\n");
    fprintf(report,"# hits: %d\n",DPageTable_hits);
    fprintf(report,"# misses: %d\n\n",DPageTable_misses);
}

int checkTLB(INST32) 
{
    return 0;
}
void checkCache(int)
{
    
}