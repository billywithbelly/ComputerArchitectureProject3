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
    
    while (fread(word_buffer, sizeof(char), 4, iImage)) //read a word
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
    
    while (fread(word_buffer, sizeof(char), 4, dImage)) //read a word
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