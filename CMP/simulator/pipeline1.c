#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define Iimage "iimage.bin"
#define Dimage "dimage.bin"

#define SNAPSHOT "snapshot.rpt"
#define REPORT "report.rpt"

#define op_func  0x00

#define op_addi  0x08
#define op_lw    0x23
#define op_lh    0x21
#define op_lhu   0x25
#define op_lb    0x20
#define op_lbu   0x24
#define op_sw    0x2B
#define op_sh    0x29
#define op_sb    0x28
#define op_lui   0x0F
#define op_andi  0x0C
#define op_ori   0x0D
#define op_nori  0x0E
#define op_slti  0x0A
#define op_beq   0x04
#define op_bne   0x05

#define op_j     0x02
#define op_jal   0x03

#define op_halt  0x3F

#define func_add  0x20
#define func_sub  0x22
#define func_and  0x24
#define func_or   0x25
#define func_xor  0x26
#define func_nor  0x27
#define func_nand 0x28
#define func_slt  0x2A
#define func_sll  0x00
#define func_srl  0x02
#define func_sra  0x03
#define func_jr   0x08

typedef unsigned int u32;
typedef unsigned char u8;

struct PTE_entry{
    int validBit;
    u32 PPN;
}*PTE;

struct TLB_entry{
    int validBit;
    u32 VPN;
    u32 PPN;
}*TLB;

struct block_entry{
    int valid_bit;
    u32 tag;
} *cache_block;

struct LRU_queue
{
    int queue[1100];
    int length;
} LRU_memory,LRU_TLB;

struct set_entry{
    int index;
    struct LRU_queue LRU_array;
} *cache_set;

struct reslt
{
    int hit;
    int miss;
}PTE_result={0,0},TLB_result={0,0},cache_result={0,0};


void load_i();
void load_d();
int getSlice(u32,int,int);
int getSignedSlice(u32,int,int);
u32 LMemory(int,int,int);
void SMemory(int,int,u32);

void initial();
void cycle();
u32 retrieveInstr(u32);
void exe_instr(u32);
void printSnapShot(int);

void R_instr(u32);
void ADD(int,int,int);
void SUB(int,int,int);
void AND(int,int,int);
void OR(int,int,int);
void XOR(int,int,int);
void NOR(int,int,int);
void NAND(int,int,int);
void SLT(int,int,int);
void SLL(int,int,int);
void SRL(int,int,int);
void SRA(int,int,int);
void JR(int);

void J(u32);
void JAL(u32);

void I_instr(u32);
void ADDI(int,int,int);
void LW(int,int,int);
void LH(int,int,int);
void LHU(int,int,int);
void HU(int,int,int);
void LB(int,int,int);
void LBU(int,int,int);
void SW(int,int,int);
void SH(int,int,int);
void SB(int,int,int);
void LUI(int,int);
void ANDI(int,int,int);
void ORI(int,int,int);
void NORI(int,int,int);
void SLTI(int,int,int);
void BEQ(int,int,int);
void BNE(int,int,int);

void numOverflow(int,int,int);
int memOverflow(int,int);
int misalign(int,int);
int zeroReg(int);

int fetchDataToMemory(u32);
void push(struct LRU_queue *,int);
int pop(struct LRU_queue *);
int getPow(u32);

u32 checkTLB(u32);
u32 checkPTE(u32);
void checkCache(u32);
void changeCacheValidBit(u32);
void printReport();
//global variable
FILE *snapShot,*error,*report;
int cycleNum=0,halt=0;
u32 registers[32],instr_num=0,mem_num=0;
u32 PC;
u8 D_Memory[2000],I_Memory[2000];

int num_block,num_set;
int PTE_size,TLB_size;
int PPN_MAX;
int PPN_index;
int TLB_index;
u32 *PPNtoVPN,*PPNtoTLBindex;

int memory_size;
int disk_size;
int page_size;

int cache_size;
int block_size;
int n_way;

u32 D_address[1000000];
u32 D_cycle[1000000];
u32 I_address[1000000];
int D_address_length;
int I_address_length;

int command[20];

int main (int argc, char* argv[])
{
    int i;
    if(argc==11)
    {
        for(i=1;i<argc;i++)
        {
            command[i]=atoi(argv[i]);
        }
    }

    snapShot=fopen(SNAPSHOT,"w");
    //error=fopen(ERROR,"w");
    report=fopen(REPORT,"w");

    initial();
    load_i();
    load_d();
    //printf("PC = 0x%08x\n",PC);
    printSnapShot(cycleNum++);
    while(halt==0)
        cycle();
    printReport();
    return 0;
}


void initial()
{
    int i=0;
    PC=0;
    D_address_length=0;
    I_address_length=0;
    for (i=0;i<32;i++)
        registers[i]=0;
    for (i=0;i<2000;i++)
    {
        I_Memory[i]=0;
        D_Memory[i]=0;
    }
}

void cycle()
{
    memOverflow(PC,1);
    misalign(PC,4);
    if (halt==1)
        return;
    u32 instruction=retrieveInstr(PC);
    //printf("excuting [%d] 0x%08x ...\n",cycleNum,instruction);
    PC=PC+4;
    exe_instr(instruction);

    registers[0]=0;
    if (halt!=1)
        printSnapShot(cycleNum++);
}

u32 retrieveInstr(u32 PC)
{
    int i;
    u32 instrction=0;
    for (i=0;i<=3;i++)
    {
        instrction=instrction<<8;
        instrction=instrction|I_Memory[PC+i];
    }
    if (cycleNum==19)
        printf("(%d) PC = %d, 0x%08x\n",cycleNum,PC,instrction);
    return instrction;
}

void exe_instr(u32 instruction)
{
    int opcode=getSlice(instruction,31,26);

    if (opcode==0x00)
        R_instr(instruction);
    else if(opcode==0x02)
        J(getSlice(instruction,25,0));
    else if (opcode==0x03)
        JAL(getSlice(instruction,25,0));
    else if (opcode==0x3f)
    {
        halt=1;
        return;
    }
    else
        I_instr(instruction);
}

void R_instr(u32 instrction)
{
    int func=getSlice(instrction,5,0);
    int rs=getSlice(instrction,25,21);
    int rt=getSlice(instrction,20,16);
    int rd=getSlice(instrction,15,11);
    int shamt = getSlice(instrction,10,6);

    if (func==func_add)
        ADD(rs,rt,rd);
    else if (func==func_sub)
        SUB(rs,rt,rd);
    else if (func==func_and)
        AND(rs,rt,rd);
    else if (func==func_or)
        OR(rs,rt,rd);
    else if (func==func_xor)
        XOR(rs,rt,rd);
    else if (func==func_nor)
        NOR(rs,rt,rd);
    else if (func==func_nand)
        NAND(rs,rt,rd);
    else if (func==func_slt)
        SLT(rs,rt,rd);
    else if (func==func_sll)
        SLL(rt,rd,shamt);
    else if (func==func_srl)
        SRL(rt,rd,shamt);
    else if (func==func_sra)
        SRA(rt,rd,shamt);
    else if (func==func_jr)
        JR(rs);
    else
        printf("0x%08x is an undefined instruction!\n",instrction);

}

void I_instr(u32 instrction)
{
    int immediate=getSlice(instrction,15,0);
    int signed_imm=getSignedSlice(instrction,15,0);
    int rs=getSlice(instrction,25,21);
    int rt=getSlice(instrction,20,16);
    int opcode=getSlice(instrction,31,26);

    if (opcode==op_addi)
        ADDI(rs,rt,signed_imm);
    else if(opcode==op_lw)
        LW(rs,rt,signed_imm);
    else if(opcode==op_lh)
        LH(rs,rt,signed_imm);
    else if(opcode==op_lhu)
        LHU(rs,rt,signed_imm);
    else if(opcode==op_lb)
        LB(rs,rt,signed_imm);
    else if(opcode==op_lbu)
        LBU(rs,rt,signed_imm);
    else if(opcode==op_sw)
        SW(rs,rt,signed_imm);
    else if(opcode==op_sh)
        SH(rs,rt,signed_imm);
    else if(opcode==op_sb)
        SB(rs,rt,signed_imm);
    else if(opcode==op_lui)
        LUI(rt,immediate);
    else if(opcode==op_andi)
        ANDI(rs,rt,immediate);
    else if(opcode==op_ori)
        ORI(rs,rt,immediate);
    else if(opcode==op_nori)
        NORI(rs,rt,immediate);
    else if(opcode==op_slti)
        SLTI(rs,rt,signed_imm);
    else if(opcode==op_beq)
        BEQ(rs,rt,signed_imm);
    else if(opcode==op_bne)
        BNE(rs,rt,signed_imm);
}

void ADD(int rs,int rt,int rd)
{
    int zero_reg = zeroReg(rd);
    numOverflow(registers[rs],registers[rt],registers[rs]+registers[rt]);
    if (zero_reg==0)
        registers[rd]=registers[rs]+registers[rt];
}

void SUB(int rs,int rt,int rd)
{
    int tempA=registers[rs];
	int tempB=registers[rt];
	registers[rd] = registers[rs]-registers[rt];

	zeroReg(rd);
	numOverflow(tempA,-tempB,tempA-tempB);
}

void AND(int rs,int rt,int rd)
{
    if (zeroReg(rd)==1)
        return;
    registers[rd]=registers[rs]&registers[rt];
}

void OR(int rs,int rt,int rd)
{
    if (zeroReg(rd)==1)
        return;
    registers[rd]=registers[rs]|registers[rt];
}

void XOR(int rs,int rt,int rd)
{
    if (zeroReg(rd)==1)
        return;
    registers[rd]=registers[rs]^registers[rt];
}

void NOR(int rs,int rt,int rd)
{
    if (zeroReg(rd)==1)
        return;
    registers[rd]=~(registers[rs]|registers[rt]);
}

void NAND(int rs,int rt,int rd)
{
    if (zeroReg(rd)==1)
        return;
    registers[rd]=~(registers[rs]&registers[rt]);
}

void SLT(int rs,int rt,int rd)
{
    if (zeroReg(rd)==1)
        return;
    //signed
    if ((signed int)registers[rs]<(signed int)registers[rt])
        registers[rd]=1;
    else
        registers[rd]=0;
}

void SLL(int rt,int rd,int shamt)
{
	int z=0;
    if(rt==0 && rd==0 && shamt==0){
        z=0;
    }
    else{
        z=zeroReg(rd);
    }
//    if (zeroReg(rd)==1)
  //      return;

	if(z==1){return;}
    else{
		if (shamt==32)
			registers[rd]=0x00000000;
		else if (shamt==0)
			registers[rd]=registers[rt];
		else
			registers[rd]=registers[rt]<<shamt;
	}

}

void SRL(int rt,int rd,int shamt)
{
    if (zeroReg(rd)==1)
        return;
    if (shamt==32)
        registers[rd]=0x00000000;
    else if (shamt==0)
        registers[rd]=registers[rt];
    else
        registers[rd]=registers[rt]>>shamt;
}

void SRA(int rt,int rd,int shamt)
{
    int sign=0;
    if (zeroReg(rd)==1)
        return;
    if (shamt==32)
        registers[rd]=0x00000000;
    else if (shamt==0)
        registers[rd]=registers[rt];
    else{
        if (registers[rt]>>31==1)
            sign=1;
        registers[rd]=registers[rt]>>shamt;
        if (sign&&shamt!=0)
            registers[rd]=registers[rd]|(unsigned int)(0xffffffff<<(32-shamt));
    }
}

void JR(int rs)
{
    PC=registers[rs];
}

void J(u32 address)
{
    PC=(getSlice(PC,31,28)<<26)|(address*4);
}

void JAL(u32 address)
{
    registers[31]=PC;
    PC=(getSlice(PC,31,28)<<28)|(address*4);
}
void ADDI(int rs,int rt,int immediate)
{
    int zero_reg=zeroReg(rt);
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    if (zero_reg==0)
        registers[rt]=registers[rs]+immediate;
}

void LW(int rs,int rt,int immediate)
{
    //printf("LW $%d = %d($%d)\n",rt,immediate,rs);
    int z=zeroReg(rt);
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    memOverflow(registers[rs]+immediate,4);
    misalign(registers[rs]+immediate,4);

    if (halt==0 && z==0)
        registers[rt]=LMemory(registers[rs]+immediate,4,0);
}

void LH(int rs,int rt,int immediate)
{
    int z=zeroReg(rt);
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    memOverflow(registers[rs]+immediate,2);
    misalign(registers[rs]+immediate,2);

    if (halt==0 && z==0)
        registers[rt]=LMemory(registers[rs]+immediate,2,1);
}

void LHU(int rs,int rt,int immediate)
{
    int z=zeroReg(rt);
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    memOverflow(registers[rs]+immediate,2);
    misalign(registers[rs]+immediate,2);

    if (halt==0 && z==0)
        registers[rt]=LMemory(registers[rs]+immediate,2,0);
}

void LB(int rs,int rt,int immediate)
{
    int z=zeroReg(rt);
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    memOverflow(registers[rs]+immediate,1);

    if (halt==0 && z==0)
        registers[rt]=LMemory(registers[rs]+immediate,1,1);
}

void LBU(int rs,int rt,int immediate)
{
    int z=zeroReg(rt);
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    memOverflow(registers[rs]+immediate,1);

    if (halt==0 && z==0)
        registers[rt]=LMemory(registers[rs]+immediate,1,0);
}

void SW(int rs,int rt,int immediate)
{
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    memOverflow(registers[rs]+immediate,4);
    misalign(registers[rs]+immediate,4);

    if (halt==0)
        SMemory(registers[rs]+immediate,4,registers[rt]);

}

void SH(int rs,int rt,int immediate)
{
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    memOverflow(registers[rs]+immediate,2);
    misalign(registers[rs]+immediate,2);

    if (halt==0)
        SMemory(registers[rs]+immediate,2,registers[rt]);

}

void SB(int rs,int rt,int immediate)
{
    numOverflow(registers[rs],immediate,registers[rs]+immediate);
    memOverflow(registers[rs]+immediate,1);

    if (halt==0)
        SMemory(registers[rs]+immediate,1,registers[rt]);
}

void LUI(int rt,int immediate)
{
    if (zeroReg(rt)==1)
        return;
    registers[rt]=immediate<<16;
}

void ANDI(int rs,int rt,int immediate)
{
    if (zeroReg(rt)==1)
        return;
    registers[rt]=registers[rs]&immediate;
}

void ORI(int rs,int rt,int immediate)
{
    if (zeroReg(rt)==1)
        return;
    registers[rt]=registers[rs]|immediate;
}

void NORI(int rs,int rt,int immediate)
{
    if (zeroReg(rt)==1)
        return;
    registers[rt]=~(registers[rs]|immediate);
}

void SLTI(int rs,int rt,int immediate)
{
    if (zeroReg(rt)==1)
        return;
    if((signed int)registers[rs]<(signed int)immediate){
        registers[rt]=1;
    }
    else{
        registers[rt]=0;
    }
}

void BEQ(int rs,int rt,int immediate)
{
    if(registers[rt]==registers[rs])
    {
        numOverflow(PC,(immediate<<2),PC+4*immediate);
        PC=PC+4*immediate;
    }
}

void BNE(int rs,int rt,int immediate)
{
    if(registers[rt]!=registers[rs])
    {
        numOverflow(PC,(immediate<<2),PC+4*immediate);
        PC=PC+4*immediate;
    }
}

int getSlice(u32 instrction,int from,int to) //unsigned slice
{
    return (instrction<<(31-from))>>(32-(from-to+1));
}

int getSignedSlice(u32 instrction,int from,int to) // signed slice
{
    int sign=(instrction<<(31-from))>>31;
    int slice=(instrction<<(31-from))>>(32-(from-to+1));

    if (sign==1)
    {
        slice=slice|(0xffffffff<<(from-to+1));
    }

    return slice;
}

u32 LMemory(int memoryLocation,int byteNum,int sign)
{
    int temp=0,i;
    int MSB;

    //fprintf(datarpt,"%d\n",memoryLocation);
    D_address[D_address_length++]=memoryLocation;
    /*if (memOverflow(memoryLocation,byteNum)==1)
        return 0;*/

    MSB=D_Memory[memoryLocation]>>7;

    for (i=0;i<byteNum;i++)
    {
        if (cycleNum==19)
            printf("mem[%d] = 0x%02x\n",memoryLocation+i,D_Memory[memoryLocation+i]);
        temp=(temp<<8);
        temp=temp|D_Memory[memoryLocation+i];
    }

    if (sign==1&&MSB==1&&byteNum!=4)
    {
        temp=temp|(0xffffffff<<(8*byteNum));
    }
    //printf("MEM[0x%08x] = 0x%08x\n",memoryLocation,temp);
    return temp;
}

void SMemory(int memoryLocation,int byteNum,u32 data)
{
    int i;
    //fprintf(datarpt,"%d\n",memoryLocation);
    D_address[D_address_length++]=memoryLocation;
    /*if (memOverflow(memoryLocation,byteNum)==1)
        return;*/

    for(i=byteNum-1;i>=0;i--)
    {
        printf("store to mem[%d]\n",memoryLocation+i);
        D_Memory[memoryLocation+i]=(u8)(data<<24>>24);
        data=data>>8;
    }
}

void printSnapShot(int cycleNum)
{
    int i=0;
    fprintf(snapShot,"cycle %d\n",cycleNum);
    for (i=0;i<32;i++)
    {
        fprintf(snapShot,"$%02d: 0x%08X\n",i,registers[i]);
    }
    fprintf(snapShot,"PC: 0x%08X\n",PC);
    I_address[I_address_length++]=PC;
    fprintf(snapShot,"\n\n");
}

int zeroReg(int reg_index)
{
    if (reg_index==0)
    {
        registers[0]=0;
        printf("Write $0 error in cycle: %d\n",cycleNum);
        return 1;
    }
    else
        return 0;
}

int misalign(int test,int byte)
{
    //printf("Misalign = %d /%d\n",test,byte);
    if (test%byte!=0)
    {
        printf("Misalignment error in cycle: %d\n",cycleNum);
        halt=1;
        return 1;
    }
    else
        return 0;
}

void numOverflow(int a,int b,int result)
{
    if (a>>31==b>>31)
        if (a>>31!=result>>31)
        {
            printf("Number overflow in cycle: %d\n",cycleNum);
        }
}

int  memOverflow(int index,int length)
{
    int i;

    for (i=0;i<length;i++)
    {
        if ((index+i)<0||(index+i)>1023)
        {
            printf("Address overflow in cycle: %d\n",cycleNum);
            halt=1;
            return 1;
        }
    }

        return 0;
}



void printReport()
{
    int count;
    int tmp;
    int ICache_hits, ICache_misses,
        DCache_hits, DCache_misses,
        ITLB_hits, ITLB_misses,
        DTLB_hits, DTLB_misses,
        IPageTable_hits, IPageTable_misses,
        DPageTable_hits, DPageTable_misses;
    for(count=0;count<2;count++)
    {
        if(count==0)
        {
            //I ³W®æ
			if(command[1]==0)memory_size=64;else memory_size=(command[1]);
            disk_size=1024;
            if(command[3]==0)page_size=8;else page_size=(command[3]);
            if(command[5]==0)cache_size=16;else cache_size=(command[5]);
            if(command[6]==0)block_size=4;else block_size=(command[6]);
            if(command[7]==0)n_way=4;else n_way=(command[7]);

            /*memory_size=32;
            disk_size=1024;
            page_size=16;
            cache_size=16;
            block_size=4;
            n_way=4;*/
            printf("I_memory_size = %d\n",memory_size);
            printf("I_disk_size = %d\n",disk_size);
            printf("I_page_size = %d\n",page_size);
            printf("I_cache_size = %d\n",cache_size);
            printf("I_block_size = %d\n",block_size);
            printf("I_n_way = %d\n",n_way);
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

void checkCache(u32 p_add)
{
    u32 tag_index = p_add/block_size;
    u32 tag= tag_index/num_set;
    u32 index = tag_index%num_set;

    //printf("tag_index = %d (%d,%d) ",tag_index,tag,index);

    int hit=0;
    int i;
	int flag = 0;
    int replace_index;

    for (i=0;i<n_way;i++)
        if (cache_block[index*n_way+i].valid_bit==1&&cache_block[index*n_way+i].tag==tag)
        {
            hit=1;
            break;
        }

    if (hit==1)
    {
        //printf("[Cache  hit]  ");
        cache_result.hit++;
        push(&cache_set[index].LRU_array,i);
    }
    else
    {
         //printf("[Cache miss]  ");
        cache_result.miss++;
        if (cache_set[index].index<n_way)
        {
                flag = 1;
				cache_block[index*n_way+cache_set[index].index].valid_bit=1;
                cache_block[index*n_way+cache_set[index].index].tag=tag;
                push(&cache_set[index].LRU_array,cache_set[index].index);
                cache_set[index].index++;
        }
        else{
            for(i=0;i<n_way;i++){
                if(cache_block[index*n_way+i].valid_bit == 0){
                    flag = 1;
                    cache_block[index*n_way+i].valid_bit = 1;
                    cache_block[index*n_way+i].tag = tag;
                    push(&cache_set[index].LRU_array, i);
                    break;
                }
            }
        }
        if(flag == 0){
            replace_index = pop(&cache_set[index].LRU_array);
            cache_block[index*n_way+replace_index].valid_bit = 1;
            cache_block[index*n_way+replace_index].tag = tag;
            push(&cache_set[index].LRU_array, replace_index);

        }
    }
}

void changeCacheValidBit(u32 p_add)
{
    //printf("\n>>check if [%d] is in Cache\n\n",p_add);

    u32 tag_index = p_add/block_size;
    u32 tag= tag_index/num_set;
    u32 index = tag_index%num_set;
    int i=0;

    for (i=0;i<n_way;i++)
        if (cache_block[index*n_way+i].valid_bit==1&&cache_block[index*n_way+i].tag==tag)
        {
            cache_block[index*n_way+i].valid_bit=0;
            break;
        }
}


u32 checkPTE(u32 address) //return PPN
{

    u32 VPN=address>>(getPow(page_size));

    //printf("[index/size] = [%u/%u]\n",VPN,PTE_size);
    u32 PPN;
    if (PTE[VPN].validBit==0) //PTE miss
    {
        //printf("  [PTE miss] ");
        PTE[VPN].validBit=1;
        PPN=fetchDataToMemory(address); /**if LRU, change one valid bit to zero*/
        PTE[VPN].PPN=PPN;
        PPNtoVPN[PPN]=VPN;
        /**fetch data to cache
         *
         *
         *
         */
        PTE_result.miss++;
    }
    else //PTE hit
    {
        //printf("  [PTE  hit] ");
		push(&LRU_memory, PTE[VPN].PPN);
        PTE_result.hit++;
        //refresh LRU
    }

    return PTE[VPN].PPN;
}

u32 checkTLB(u32 address)
{

    int offset_bit=getPow(page_size);
    u32 VPN=address>>offset_bit;
    u32 PPN;
    int i;
    int index;
    int hit=0;
    for (i=0;i<TLB_index;i++)
    {
        if (TLB[i].validBit==1&&TLB[i].VPN==VPN)
        {
            PPN=TLB[i].PPN;
            hit=1;
            break;
        }
    }

    if (hit==1)
    {
        //printf("  [ TLB hit] ");
        PPN=TLB[i].PPN;
        push(&LRU_TLB,i);
        TLB_result.hit++;
    }
    else
    {
        //printf("  [TLB miss] ");
        PPN = checkPTE(address);
        /*for (i=0;i<TLB_index;i++)
            if (TLB[i].PPN==PPN)
                TLB[i].validBit=0;*/
        if (TLB[PPNtoTLBindex[PPN]].PPN==PPN)
            TLB[PPNtoTLBindex[PPN]].validBit=0;

        if (TLB_index<TLB_size)
        {
            PPNtoTLBindex[PPN]=TLB_index;
            TLB[TLB_index].validBit=1;
            TLB[TLB_index].VPN=VPN;
            TLB[TLB_index].PPN=PPN;
            push(&LRU_TLB,TLB_index);
            TLB_index++;
        }
        else
        {
            //LRU
            index=pop(&LRU_TLB);
            PPNtoTLBindex[PPN]=index;
            push(&LRU_TLB,index);
            TLB[index].validBit=1;
            TLB[index].VPN=VPN;
            TLB[index].PPN=PPN;
        }
        TLB_result.miss++;
    }
    //printf("PPN = %d,0x%08X\n",(PPN),(address&(~(0xFFFFFFFF<<offset_bit))));
    return (PPN<<offset_bit)|(address&(~(0xFFFFFFFF<<getPow(page_size))));
}


int fetchDataToMemory(u32 address) //will return PPN
{
    u32 return_PPN;
    u32 p_add;
    int i;

    if (PPN_index<PPN_MAX)
    {
        return_PPN=PPN_index;
        push(&LRU_memory,PPN_index);
        PPN_index++;
    }
    else
    {
        //LRU
        return_PPN=pop(&LRU_memory);
        PTE[PPNtoVPN[return_PPN]].validBit=0;/**if LRU, change one valid bit to zero*/
        for (i=0;i<page_size;i++)
        {
            p_add=(return_PPN<<getPow(page_size))|(i);
            changeCacheValidBit(p_add);
        }
        push(&LRU_memory,return_PPN);
    }

    return return_PPN;
}
//void push (int *array,int length,int data)
int pop(struct LRU_queue *LRU_queue)
{
    int *queue = LRU_queue->queue;
    int return_PPN = queue[0]; //replay PPN at queue[0]
    int i;
    for (i=0;i<LRU_queue->length-1;i++) //¾ã­Óqueue©¹0²¾°Ê¤@®æ
    {
        queue[i]=queue[i+1];
    }
    LRU_queue->length--;

    return return_PPN;
}

void push(struct LRU_queue *LRU_queue,int data)
{
    int *queue = LRU_queue->queue;
    int i,j;

    for (i=0;i<LRU_queue->length;i++) //if data hit and already exist, delete it
        if (queue[i]==data)
        {
            for (j=i;j<LRU_queue->length;j++)
            {
                if (j<LRU_queue->length-1)
                    queue[j]=queue[j+1];
                else{
                    LRU_queue->length--;
                    break;
                }
            }
            break;
        }

    queue[LRU_queue->length]=data; // add at queue's end
    LRU_queue->length++;
}


int getPow(u32 input)
{
    u32 i;
    if (input==1)
        return 0;
    for (i=1;i<=input/2;i++)
        if (pow(2,i)==input)
            return i;

    printf("pow error!");
    exit(1);
    return -1;
}

void load_i()
{
    FILE *fp;
    int n=0;
    u32 sum;
    int n_instr=0;
    int i;
    int pc_done=0;
    u8 buff[4];

    fp = fopen(Iimage, "rb");
    if (!fp)
    {
        halt=1;
        printf(Iimage" not found.\n");
        return;
    }
    while(fread(buff, sizeof(char),4, fp))
    {
        sum=0;
        for (i=0; i<=3;i++)
        {
            sum=sum<<8;
            sum=sum|buff[i];
        }
        //printf("%02d = 0x%02x%02x%02x%02x [0x%08X] \n",n++,buff[3], buff[2],buff[1], buff[0],sum);
        if (pc_done==0)
        {
            PC=sum;
            pc_done=1;
        }
        else if (instr_num==0)
        {
            instr_num=sum;
        }
        else
        {
            instr_num--;
            for (i=0; i<4;i++)
            {
                I_Memory[PC+n_instr++]=buff[i];
            }
            if (instr_num<=0)
                break;
        }
    }
}

void load_d()
{
    FILE *fp;
    int n=0;
    u32 sum;
    int n_mem=0;
    int i;
    int sp_done=0;
    u8 buff[4];

    fp = fopen(Dimage, "rb");
    if (!fp)
    {
        halt=1;
        printf(Dimage" not found.\n");
        return;
    }
    while(fread(buff, sizeof(char),4, fp))
    {
        sum=0;
        for (i=0; i<=3;i++)
        {
            sum=sum<<8;
            sum=sum|buff[i];
        }
        //printf("%02d(%d) = 0x%02x%02x%02x%02x [%010u] \n",n++,mem_num,buff[3], buff[2],buff[1], buff[0],sum);
        if (sp_done==0)
        {
            registers[29]=sum;
            sp_done=1;
        }
        else if (mem_num==0)
        {
            mem_num=sum;
        }
        else
        {
            mem_num--;
            for (i=0; i<4;i++)
            {
                D_Memory[n_mem++]=buff[i];
            }
            if(mem_num<=0)
                break;

        }
    }
    printf("toatal %d data\n",n_mem);
}
