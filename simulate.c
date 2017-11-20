#include <stdio.h>
#include <stdlib.h>


int Pop(int), Push(int), Call(int), Ret(int), Jump(int);
int IRmov(int), RRmov(int), RMmov(int), MRmov(int);
int OPx(int);
int printall(int), printreg(int),printmem(int);
int getBitSequence(int arrbin[]);

char SF = 0x00;
char ZF = 0x00;

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
int ind=0;
  while(ind<200) { printf("mem[%x]=%lx\n",ind,(unsigned char)memory[ind]); ind++; }
   while (!done) {
      byte = memory[PC];
      opcode = (byte >> 4)&0xf;
   switch (opcode) {
	 case 0: printf("Halting at instruction 0x%x\n",PC); 
		done = 1; break;  //halt
	 case 1: PC++; break;   // nop
	 case 2: PC = RRmov(PC);
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
   printf("%s = %x\n",regname[rA],regs[rA]);   
   return PC+1;
}

int
printall (int PC) {
   
   int cnt=0;
   printf("******************************\n");
   while(cnt<15){
      printf("%s = %x\t",regname[cnt],regs[cnt]);
      cnt++;
      printf("%s = %x\n",regname[cnt],regs[cnt]);
      cnt++;
   }
   printf("******************************\n");
   
   return PC+1;
}

/**
 * printmem disp(reg) – 10 byte instruction 0xE0, 0xrA:F, 8 byte displacement – print out
 * the 8 byte constant stored at address contents(rA) + displacement.
 */
int
printmem(int PC) {

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
   printf("\tprintmem value: %x\n",rA+displacement);

   return PC + 8;
} 

int
IRmov(int PC) {

   PC++;
   unsigned char r=memory[PC];
   int rB=r&0xf;

   PC++; 

   int ind = PC+7;
   unsigned char bits[8];
   int index = 0;
   for( ; ind >= PC ; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }

   regs[rB] = getHexValue(bits,8);
   printf("Register: %s\nValue:%x\n",*(regname+rB),regs[rB]);
   return PC+8;
}

int
RRmov(int PC) {
   return PC+1;
}

int
RMmov(int PC) {
   return PC+1;
}

int
MRmov(int PC) {
   
   unsigned char r = memory[PC+1];
   
   int rB = r&0xf;
   r >>= 4;
   int rA = r&0xf;
   PC+=2;
   
   int ind = PC+7;
   unsigned char bits[8];
   int index = 0;
   for( ; ind >= PC ; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }
   
   
   
   int displacement = getHexValue(bits,8);
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
   
   
   printall(PC-1);
   
   
   
   
   return PC+10;
}

int
Jump(int PC ) {
	return PC+1;
}

int
Call(int PC) {
   return PC + 1;
}

int
Ret(int PC) {
    return PC+1;
}

int
Push(int PC) {
  return PC+1;
}

int
Pop(int PC) {
  return PC+1;
}


int
OPx(int PC) {
   return PC+1;
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

