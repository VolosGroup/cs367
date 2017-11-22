#include <stdio.h>
#include <stdlib.h>


int Pop(int), Push(int), Call(int), Ret(int), Jump(int);
int IRmov(int), RRmov(int), RMmov(int), MRmov(int);
int OPx(int);
int printall(int), printreg(int),printmem(int);
int getBitSequence(int arrbin[]);
void printMemory(int startBlock, int endBlock);
int functionCount = 0;

char SF = 0x00;
char ZF = 0x00;
char OF = 0x00;

extern char memory[] ;
extern int PC ;

long int regs[16];

char *regname[15] = {"%rax","%rcx","%rdx","%rbx","%rsp","%rbp",
                "%rsi","%rdi","%r8","%r9","%r10","%r11","%r12","%r13","%r14"};



void
execute(int PC) {
   int done = 0;
   char byte; 
   char opcode;
int ind=0x0;
  //while( ind>=0x0 && ind<0x40 ) { printf("mem[%x]=%lx\n",ind,(unsigned char)memory[ind]); ind++; } exit(1);
   while (!done) {
      byte = memory[PC];
      opcode = (byte >> 4)&0xf;
   switch (opcode) {
	 case 0: printf("\tHalt\n",PC); 
		done = 1; break;  //halt
	 case 1: PC++; break;   // nop
	 case 2: PC = RRmov(PC);
      /* rmmovq is 2 0
cmovle is 2 1
cmovl is 2 2
cmove is 2 3
cmovne is 2 4 
cmovge is 2 5
cmovg is 2 6.
*/
		 break;
	 case 3: PC = IRmov(PC);
		 break;
	 case 4: PC = RMmov(PC);
		 break;
	 case 5: PC = MRmov(PC);
		 break;
	 case 6: PC = OPx(PC);
		 break;
	 case 7:  PC = Jump(PC);
		  break;
	 case 8: PC = Call(PC);
		 break;
	 case 9: PC = Ret(PC);
		 break;
	 case 10: PC = Push(PC);
		  break;
	 case 11: PC = Pop(PC);
		  break;
   case 12: PC = printall(PC);
		   break;
	 case 13: PC = printreg(PC);
		  break;
	 case 14: PC = printmem(PC);
		  break;
	 default:
	 	printf("# unknown op at PC=0x%x\n",PC); PC++;
      }
   }
}


int 
printreg(int PC) {
   PC++;
   int rA = (unsigned char)memory[PC];
   rA = rA >> 4;
   printf("\t%s = 0x%x\n",regname[rA],regs[rA]);   
   return PC+1;
}

int
printall (int PC) {
   
   int cnt=0;
   printf("\tprintall\n\n\t******************************\n");
   while(cnt<15){
      printf("\t%s = %x\t",regname[cnt],regs[cnt]);
      cnt++;
      printf("\t%s = %x\n",regname[cnt],regs[cnt]);
      cnt++;
   }
   printf("\t******************************\n\n");
   
   return PC+1;
}

/**
 * printmem disp(reg) – 10 byte instruction 0xE0, 0xrA:F, 8 byte displacement – print out
 * the 8 byte constant stored at address contents(rA) + displacement.
 */
int
printmem(int PC) {
 //printmem A(%r14)
  PC++;
  int rA = (unsigned char) memory[PC];
  rA = rA >> 4;
  PC++;

  int ind= PC+7;
  unsigned char bits[8];
  int index=0;
  for( ; ind >= PC; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }
   int displacement = getHexValue(bits,8);
   
   int newDisplacement = displacement + regs[rA];
   
   int startBlock = newDisplacement;
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      //printf("byte16 = %x\n",byte16[cnt]);
      startBlock++;
   }
   
   printf("\tprintmem %x(%s)\n", displacement, *(regname+rA) );
   
   return PC + 8;
} 

int
IRmov(int PC) {

   PC++;
   unsigned char r=memory[PC];
   unsigned int rB=r&0xf;

   PC++; 

   int ind = PC+7;
   unsigned char bits[8];
   int index = 0;
   for( ; ind >= PC ; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }

   regs[rB] = getHexValue(bits,8) + 0;
   printf("\tirmovq $0x%x, %s\n", regs[rB], *(regname+rB));
   return PC+8;
}

int
RRmov(int PC) {
   
   unsigned char t1 = memory[PC];
   unsigned int fn = t1&0xf;
   
   unsigned char r = memory[PC+1];
   unsigned int rB = r&0xf;
   r >>= 4;
   unsigned int rA = r&0xf;
   
   int answer = regs[rB] - regs[rA];
   if (answer==0x00) ZF=0x01;
   else ZF=0x00;
   if (answer<0x0) SF=0x01;
   else SF=0x00;
   
   switch(fn){
      
      case 0:
         regs[rB] = regs[rA];
         break;
      case 1:
         if ( (SF^OF)|ZF ) regs[rB] = regs[rA];
         break;
      case 2:
         if ( SF^OF ) regs[rB] = regs[rA];
         break;
      case 3:
         if ( ZF ) regs[rB] = regs[rA];
         break;
      case 4:
         if ( ~ZF ) regs[rB] = regs[rA];
         break;
      case 5:
         if ( ~(SF^OF)  ) regs[rB] = regs[rA];
         break;
      case 6:
         if ( ~(SF^OF)&~ZF ) regs[rB] = regs[rA];
         break;
      
   }
   
   printf("\trrmovq %s, %s\n", regname[rA], regname[rB]);
   
   return PC+2;
}

int
RMmov(int PC) {
   
   unsigned char r = memory[PC+1];
   unsigned int rB = r&0xf;
   r >>= 4;
   unsigned int rA = r&0xf;
   PC+=2;
   
   int ind = PC+7;
   unsigned char bits[8];
   int index = 0;
   for( ; ind >= PC ; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }
   
   unsigned int displacement = getHexValue(bits,8);
   int currentBlock = displacement + regs[rB];
   int endBlock = displacement + 7;
   int data = regs[rA];
   
   //writes content of register to blocks of memory (address of displacment + )
   while(currentBlock <= endBlock ){
      memory[currentBlock] = data&0xff;
      data >>= 8;
      currentBlock++;
   }
   
   printf("\trmmovq %s, 0x%x(%s)\n", regname[rA], currentBlock, regname[rB]);

   return PC+8;
}

int
MRmov(int PC) {
   
   unsigned char r = memory[PC+1];
   unsigned int rB = r&0xf;
   r >>= 4;
   unsigned int rA = r&0xf;
   PC+=2;
   
   int ind = PC+7;
   unsigned char bits[8];
   int index = 0;
   for( ; ind >= PC ; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }
   
   
  
   unsigned int displacement = getHexValue(bits,8);
   unsigned int initialDisplacement = displacement;
   displacement += regs[rB];
   ind = displacement + 0x7;
   unsigned char storedValue[8];
   index = 0;
   for( ; ind >= displacement ; ind-- ){
    int b = memory[ind]&0xff;
    storedValue[index] = (unsigned char) b&0xff;
    index++;
   }
   regs[rA] = getHexValue(storedValue,8);
   
   printf("\tmrmovq %x(%s), %s\n", initialDisplacement, regname[rB], regname[rA]);
   
   
   
   return PC+8;
}

int
Jump(int PC ) {
   
   
   unsigned char r = memory[PC];
   char fn = r&0xf;
   
   int startBlock = PC+1;
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      //printf("byte16 = %x\n",byte16[cnt]);
      startBlock++;
   }
   
   int jmpAddress = getHexValue(byte16,8);
   switch(fn){
      case 0:
         PC = jmpAddress;
         printf("\tjmp %x\n",jmpAddress);
         break;
      case 1:
         if ( (SF^OF)|ZF ) PC = jmpAddress; //update PC to address
         else PC = PC + 9;
         printf("\tjle %x\n",jmpAddress);
         break;
      case 2:
         if ( SF^OF ) PC = jmpAddress;
         else PC = PC + 9;
         printf("\tjl %x\n",jmpAddress);
         break;
      case 3:
         if ( ZF ) PC = jmpAddress;
         else PC = PC + 9;
         printf("\tje %x\n",jmpAddress);
         break;
      case 4:
         if ( ~ZF ) PC = jmpAddress;
         else PC = PC + 9;
         printf("\tjne %x\n",jmpAddress);
         break;
      case 5:
         if ( ~(SF^OF)  ) PC = jmpAddress;
         else PC = PC + 9;
         printf("\tjge %x\n",jmpAddress);
         break;
      case 6:
         if ( ~(SF^OF)&~ZF ) PC =  jmpAddress;
         else PC = PC + 9;
         printf("\tjg %x\n",jmpAddress);
         break;
   }
	return PC;
}

int
Call(int PC) {
   // push next instruction onto stack, then get the next 8 bytes whcih will be the next instruction
   
   int nextInstruction = PC+9;
   /**
    * pushes next instruction onto stack
    *
    */
   char stackTop = regs[4]; // get address of stack from %rsp
   //int newaddy = 0x50; // next instruction for call
   
   int newTop = stackTop - 0x8;
   regs[4] = newTop;
   
   
   // write next instruction onto stack 
   int currentBlock = newTop;
   while(currentBlock <= newTop+0x7){
      memory[currentBlock] = (unsigned char) nextInstruction&0xff;
      nextInstruction >>=8;
      currentBlock++;
   }
   
   
   int startBlock = PC+1;
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      startBlock++;
   }
   functionCount++;
   printf("\tcall fun%d\n",functionCount);
   return getHexValue(byte16,8);
}

int
Ret(int PC) {
   PC++;
   int rsp = regs[4];
   int startBlock = rsp;
   
   printf("\tret\n");
   regs[4]+=0x8;
   return memory[startBlock];
}

int
Push(int PC) {
   
   //done
   
   unsigned char r = memory[PC+1];
   r >>= 4;
   int rA = r&0xf;
   int data = regs[rA];
   
   char stackTop = regs[4]; // get address of stack from %rsp
   //int newaddy = 0x50; // next instruction for call
   
   int newTop = stackTop - 0x8;
   regs[4] = newTop;
   
   
   //write to newtop
   //writes content of register to blocks of memory (address of displacment + )
   int currentBlock = newTop;
   while(currentBlock <= newTop+0x7){
      memory[currentBlock] = (unsigned char) data&0xff;
      data >>=8;
      currentBlock++;
   }
   
   /* reads from last inserted value, this is to verify read latest stack value was correct
   int startBlock = newTop;
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      startBlock++;
   }
   */
   
   printf("\tpushq %s\n",*(regname+rA));
  return PC+2;
}

int
Pop(int PC) {
   
   unsigned char r = memory[PC+1];
   r >>= 4;
   int rA = r&0xf;
   
   int rsp = regs[4];
   int startBlock = rsp;
   
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      //printf("byte16 = %x\n",byte16[cnt]);
      startBlock++;
   }
   
   regs[4] = rsp + 0x8;
   
   printf("\tpopq %s\n",*(regname+rA));
   
  return PC+2;
}


int
OPx(int PC) {
   
   
   unsigned char r = memory[PC];
   int operation = r&0xf;
   r = memory[PC+1];
   
   unsigned int rB = r&0xf;
   r >>= 4;
   unsigned int rA = r&0xf;
   
   unsigned int answer = 0;
   
   /*
    *
    * Need Sign flag code
    *
    *
    */
   switch(operation) {
      case 0: // addq
         answer = regs[rA] + regs[rB];
         regs[rB] = answer;
         if (regs[rB] - regs[rA] == 0x0) ZF=0x01;
         else ZF=0x00;
         if (regs[rB] - regs[rA]<0x0) SF=0x01;
         else SF=0x00;
         printf("\taddq %s, %s\n", regname[rA], regname[rB]);
         break;
      case 1: // subq
         answer = regs[rB] - regs[rA];
         regs[rB] = answer;
         if (answer==0x00) ZF=0x01;
         else ZF=0x00;
         if (regs[rB] - regs[rA]<0x0) SF=0x01;
         else SF=0x00;
         printf("\tsubq %s, %s\n", regname[rA], regname[rB]);
         break;
      case 2: // andq
         answer = regs[rB] & regs[rA];
         regs[rB] = answer;
         if (regs[rB] - regs[rA] == 0x0) ZF=0x01;
         else ZF=0x00;
         if (regs[rB] - regs[rA]<0x0) SF=0x01;
         else SF=0x00;
         printf("\tandq %s, %s\n", regname[rA], regname[rB]);
         break;
      case 3: // xorq
         answer = regs[rB] ^ regs[rA];
         regs[rB] = answer;
         if (regs[rB] - regs[rA] == 0x0) ZF=0x01;
         else ZF=0x00;
         if (regs[rB] - regs[rA]<0x0) SF=0x01;
         else SF=0x00;
         printf("\txorq %s, %s\n", regname[rA], regname[rB]);
         break;
   }
   
   return PC+2;
}

int getHexValue(unsigned char *ptr, int bitCount){
  int i=1;
  int val=(*(ptr) << 8) | *(ptr++);

  while(i<bitCount){
   val=(val << 8) | *(ptr++);
   i++;
  }
  return val;
}

void printMemory(int startBlock, int endBlock){
   int ind = startBlock;
   while( ind >= startBlock && ind<=endBlock){
      printf("Mem[%x] = %x\n",ind,memory[ind]);
      ind++;
   }
   
}

/*
char *getByte(int startBlock){
   unsigned char byte16[8];
   int cnt = startBlock + 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      startBlock++;
   }
   return byte16;
}

*/

