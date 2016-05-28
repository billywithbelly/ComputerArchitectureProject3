#include <iostream>
#include <cstdio>

#include "environment.h"
#include "library.h"

void loadImage (INST32 PC, INST32 registers[32], INST8 IMemory[MAX_LENGTH], INST8 DMemory[MAX_LENGTH])
{
    FILE *iImage, *dImage;
    iImage = fopen ("iimage.bin", "rb");
	dImage = fopen ("dimage.bin", "rb");
	
	INST32 instruction;
	INST8 word_buffer[4];
	int state = 0;
	int instruction_in_Memory = 0;
	int numOfInst = 0;
	
	if (!iImage)
	{
		printf ("iimage not found...\n");
		return;
	}
	
	while (fread(word_buffer, sizeof(char), 4, iImage))	//read a word
		//4 bytes at once
	{
		instruction = 0;
		for (int i=0; i<4; i++)//big endian
		{
			instruction = instruction << 8;
			instruction = instruction | word_buffer[i];
		}
		//printf ("%08x\n", instruction);
		//there are three states
		switch (state)
		{
			//其實發現有點不懂 break 在 switch 裡面的意義
			//
			case 0:
				//first four bytes for PC
				PC = instruction;
				//printf ("%08x\n", instruction);
				state = 1;
				break;
			case 1:
				//next four be number of words
				numOfInst = instruction;
				//printf ("%08x\n", instruction);
				state = 2;
				break;
			case 2:
				//read instruction
				numOfInst -= 1;
				//printf ("%02x%02x%02x%02x\n", word_buffer[0], word_buffer[1], word_buffer[2], word_buffer[3]);
				for (int i=0; i<4; i++)
					IMemory[PC + instruction_in_Memory++] = word_buffer[i];
					//filling the IMemory sequentially from PC
				//return when finishing the instruction
				if (numOfInst <= 0)
					break;
					//return;
				break;
		}
	}
	state = 0;
	instruction_in_Memory = 0;
	numOfInst = 0;
	
	if (!dImage)
	{
		printf ("Dimage not found...\n");
		return;
	}
	
	while (fread(word_buffer, sizeof(char), 4, dImage))	//read a word
		//4 bytes at once
	{
		instruction = 0;
		for (int i=0; i<4; i++)
		{
			instruction = instruction << 8;
			instruction = instruction | word_buffer[i];
		}
		switch (state)
		{
			case 0:
				registers[29] = instruction;
				//printf ("--------------\n%08x\n", instruction);
				state = 1;
				//$sp done
				break;
			case 1:
				numOfInst = instruction;
				//printf ("%08x\n", instruction);
				state = 2;
				break;
			case 2:
				//read instruction
				numOfInst -= 1;
				//printf ("%08x\n", instruction);
				for (int i=0; i<4; i++)
					DMemory[instruction_in_Memory++] = word_buffer[i];
					//filling the DMemory sequentially from 0
				//return when finishing the instruction
				if (numOfInst <= 0)
					return;
				break;
		}
	}
	return ;
}

void produceSnapShot (INST32 PC, FILE *snapShot, INST32 registers[32], int cycle)
{
    fprintf(snapShot,"cycle %d\n",cycle);
    
    for (int i=0; i<32; i++)
    {
        fprintf(snapShot,"$%02d: 0x%08X\n", i, registers[i]);
    }
    fprintf(snapShot,"PC: 0x%08X\n", PC);
}

void produceReport (int command[20], int memorySize, int diskSize, int pageSize, 
	int cacheSize, int blockSize, int nWay)
{
    int count;
    int tmp;
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
            //D ³W®æ
            if(command[2]==0)memory_size=32;else memory_size=(command[2]);
            disk_size=1024;
            if(command[4]==0)page_size=16;else page_size=(command[4]);
            if(command[8]==0)cache_size=16;else cache_size=(command[8]);
            if(command[9]==0)block_size=4;else block_size=(command[9]);
            if(command[10]==0)n_way=1;else n_way=(command[10]);
            /*memory_size=32;
            disk_size=1024;
            page_size=16;
            cache_size=16;
            block_size=4;
            n_way=1;*/

            printf("D_memory_size = %d\n",memory_size);
            printf("D_disk_size = %d\n",disk_size);
            printf("D_page_size = %d\n",page_size);
            printf("D_cache_size = %d\n",cache_size);
            printf("D_block_size = %d\n",block_size);
            printf("D_n_way = %d\n",n_way);
        }
        int i;

        PTE_size=disk_size/page_size;
        TLB_size=PTE_size/4;
        PPN_MAX=memory_size/page_size;
        PPN_index=0;
        TLB_index=0;

        num_block=cache_size/block_size;
        num_set=num_block/n_way;

        PTE = malloc(sizeof(struct PTE_entry)*PTE_size);
        TLB = malloc(sizeof(struct TLB_entry)*TLB_size);
        PPNtoTLBindex= malloc(sizeof(u32)*TLB_size);
        PPNtoVPN=malloc(sizeof(u32)*PPN_MAX);
        memset(PTE,0,sizeof(struct PTE_entry)*PTE_size);
        memset(TLB,0,sizeof(struct TLB_entry)*TLB_size);
        memset(PPNtoTLBindex,0,sizeof(u32)*TLB_size);
        memset(PPNtoVPN,0,sizeof(u32)*PPN_MAX);

        cache_block=malloc(sizeof(struct block_entry)*num_block);
        cache_set=malloc(sizeof(struct set_entry)*num_set);
        memset (cache_block,0,sizeof(struct block_entry)*num_block);
        memset (cache_set,0,sizeof(struct set_entry)*num_set);

        PTE_result.hit = 0;
        PTE_result.miss = 0;
        TLB_result.hit = 0;
        TLB_result.miss = 0;
        cache_result.hit = 0;
        cache_result.miss = 0;

        memset (&LRU_memory,0,sizeof(struct LRU_queue));
        memset (&LRU_TLB,0,sizeof(struct LRU_queue));

        if(count==0)
        {
            for (i=0;i<I_address_length;i++)
            {
                tmp=checkTLB(I_address[i]);
                checkCache(tmp);
                //printf("V_add -> P_add = 0x%08x -> 0x%08x",I_address[i],tmp);
                //putchar('\n');
            }
            ICache_hits = cache_result.hit;
            ICache_misses = cache_result.miss;
            ITLB_hits = TLB_result.hit;
            ITLB_misses = TLB_result.miss;
            IPageTable_hits = PTE_result.hit;
            IPageTable_misses = PTE_result.miss;
        }
        else
        {
            for (i=0;i<D_address_length;i++)
            {
                tmp=checkTLB(D_address[i]);
                checkCache(tmp);
                //printf("()V_add -> P_add = 0x%08x -> 0x%08x",D_address[i],tmp);
                //putchar('\n');
            }
            DCache_hits = cache_result.hit;
            DCache_misses = cache_result.miss;
            DTLB_hits = TLB_result.hit;
            DTLB_misses = TLB_result.miss;
            DPageTable_hits = PTE_result.hit;
            DPageTable_misses = PTE_result.miss;
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

}

INST32 instructionFetcher (INST32 PC, INST8 IMemory[MAX_LENGTH])
{
	INST32 instrction = 0;
    for (int i=0; i<32; i++)
    {
        instrction = instrction << 8;
        instrction = instrction | IMemory[PC+i];
    }
    return instrction;
}

void instructionPractice (INST32 instruction)
{
	int OPCode = instructionFragment(instruction, 31, 26);
	
	if (OPCode == RFORMAT){
		
	}
	else if (OPCode == J) {
		
	}
	else if (OPCode == JAL) {
		
	}
	else if (OPCode == HALT) {
		
	}
	else {
		
	}
}

int instructionFragment (INST32 instruction, int head, int tail)
{
    return (instruction << (31 - head)) >> (31 - (head - tail));
}