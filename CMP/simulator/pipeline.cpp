#include <cmath>
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
void modifyCacheValidBits (INST32);
INST32 checkPTE (INST32);

//===================================
//some tools

int getPow (INST32 input)
{
    INST32 i;
    if (input == 1)
        return 0;
    for (i=1; i<=input/2; i++)
        if (pow(2,i) == input)
            return i;

    printf("pow error!");
    exit(1);
    return -1;
}

int pop (LRUQueue* input)
{
    int *queue = input->queue;
    int return_PPN = input->queue[0];
    int i;
    for (i=0; i<input->length - 1; i++)
    {
        queue[i] = queue[i+1];
    }
    input->length -= 1;

    return return_PPN;
}

void push (LRUQueue* input, int data)
{
    int *queue = input->queue;
    int i,j;

    for (i=0; i<input->length; i++)
        if (queue[i] == data)
        {
            for (j=i; j<input->length; j++)
            {
                if (j < input->length - 1)
                    queue[j] = queue[j+1];
                else{
                    input->length -= 1;
                    break;
                }
            }
            break;
        }

    queue[input->length]=data; // add at queue's end
    input->length += 1;
}

int fetchDataToMemory (INST32 input)
{
    INST32 return_PPN;
    INST32 index;
    int i;

    if (PPNIndex < PPNMAX)
    {
        return_PPN = PPNIndex;
        push(queuedMemory, PPNIndex);
        PPNIndex += 1;
    }
    else
    {
        return_PPN = pop(queuedMemory);
        PTE[PPNtoVPN[return_PPN]].validBits = 0;
        for (i=0; i<pageSize; i++)
        {
            index = (return_PPN << getPow (pageSize)) | (i);
            modifyCacheValidBits (index);
        }
        push(queuedMemory,return_PPN);
    }

    return return_PPN;
}

//=========================================

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

int checkTLB(INST32 input) 
{
    int offset = getPow (pageSize);
    INST32 VPN = input >> offset;
    INST32 PPN;
    
    int i;
    int index;
    int hit = 0;
    for (i=0; i<TLBIndex; i++)
    {
        if (TLB[i].validBits == 1 && TLB[i].VPN == VPN)
        {
            PPN = TLB[i].PPN;
            hit = 1;
            break;
        }
    }

    if (hit == 1)
    {
        PPN = TLB[i].PPN;
        push(queuedTLB, i);
        resultTLB->hit += 1;
    }
    else
    {
        PPN = checkPTE(input);
        if (TLB[PPNtoTLBindex[PPN]].PPN == PPN)
            TLB[PPNtoTLBindex[PPN]].validBits = 0;

        if (TLBIndex < TLBSize)
        {
            PPNtoTLBindex[PPN] = TLBIndex;
            TLB[TLBIndex].validBits = 1;
            TLB[TLBIndex].VPN = VPN;
            TLB[TLBIndex].PPN = PPN;
            push(queuedTLB, TLBIndex);
            TLBIndex += 1;
        }
        else
        {
            index = pop(queuedTLB);
            PPNtoTLBindex[PPN] = index;
            push(queuedTLB, index);
            TLB[index].validBits = 1;
            TLB[index].VPN = VPN;
            TLB[index].PPN = PPN;
        }
        resultTLB->miss += 1;
    }
    
    return (PPN << offset) | (input & ( ~(0xFFFFFFFF << getPow (pageSize))));
}

void checkCache(int input)
{
    INST32 tag_index = input / blockSize;
    INST32 tags = tag_index / numSet;
    INST32 index = tag_index % numSet;

    int i;
    int hit = 0;
	int flag = 0;
    int replace_index;

    for (i=0; i<nWay; i++)
        if (cacheBlock[index * nWay + i].validBits == 1    
            && cacheBlock[index * nWay + i].tags == tags)
        {
            hit = 1;
            break;
        }

    if (hit == 1)
    {
        resultCache->hit += 1;
        push(cacheSet[index].LRUArray,i);
    }
    else
    {
        resultCache->miss += 1;
        if (cacheSet[index].index < nWay)
        {
                flag = 1;
				cacheBlock[index * nWay + cacheSet[index].index]. validBits= 1;
                cacheBlock[index * nWay + cacheSet[index].index].tags = tags;
                push(cacheSet[index].LRUArray, cacheSet[index].index);
                cacheSet[index].index += 1;
        }
        else{
            for(i=0; i<nWay; i++){
                if(cacheBlock[index * nWay + i].validBits == 0){
                    flag = 1;
                    cacheBlock[index * nWay + i].validBits = 1;
                    cacheBlock[index * nWay + i].tags = tags;
                    push(cacheSet[index].LRUArray, i);
                    break;
                }
            }
        }
        if(flag == 0){
            replace_index = pop(cacheSet[index].LRUArray);
            cacheBlock[index * nWay + replace_index].validBits = 1;
            cacheBlock[index * nWay + replace_index].tags = tags;
            push(cacheSet[index].LRUArray, replace_index);

        }
    }
}

void modifyCacheValidBits (INST32 input)
{
    INST32 tag_index = input / blockSize;
    INST32 tags= tag_index / numSet;
    INST32 index = tag_index % numSet;
    
    for (int i=0; i<nWay; i++)
        if (cacheBlock[index * nWay + i].validBits == 1
            && cacheBlock[index * nWay + i].tags == tags)
        {
            cacheBlock[index * nWay + i].validBits = 0;
            break;
        }
}

INST32 checkPTE (INST32 index)
{

    INST32 VPN = index >> (getPow (pageSize));
    INST32 PPN;
    
    if (PTE[VPN].validBits == 0)
    {
        PTE[VPN].validBits = 1;
        PPN = fetchDataToMemory(index);
        PTE[VPN].PPN = PPN;
        PPNtoVPN[PPN] = VPN;
        resultPTE->miss += 1;
    }
    else
    {
        push(queuedMemory, PTE[VPN].PPN);
        resultPTE->hit++;
    }

    return PTE[VPN].PPN;
}
