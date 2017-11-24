/*
 * Omar Zairi
 * CS 367
 * Y86 Simulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int Pop(int), Push(int), Call(int), Ret(int), Jump(int);
int IRmov(int), RRmov(int), RMmov(int), MRmov(int);
int OPx(int);
int printall(int), printreg(int),printmem(int);
int getBitSequence(int arrbin[]);
void printMemory(int startBlock, int endBlock);

char SF = 0x00;
char ZF = 0x00;
char OF = 0x00;

extern char memory[] ;
extern int PC ;
extern char ** funcs ;

long int regs[16];

char *regname[15] = {"%rax","%rcx","%rdx","%rbx","%rsp","%rbp",
                "%rsi","%rdi","%r8","%r9","%r10","%r11","%r12","%r13","%r14"};
   

// string array to store the different functions
char **funcs;                  
int ccc = 0; // counter
int inCall = 0; // checks to see if an instruction is in a call function
int funcsCount = 0; // keeps track of function count
               



void
execute(int PC) {
   int done = 0;
   char byte; 
   char opcode;
   int ind=0x0;
   
   // initialize the function strings
   funcs =  (char**) malloc (10);  //allocating array of two string
   for(int i=0;i<10;i++) funcs[i] = (char*) malloc (200);
   
   while (!done) {
      byte = memory[PC];
      opcode = (byte >> 4)&0xf;
   switch (opcode) {
	 case 0: printf("\thalt\n",PC); 
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
   
   // display functions
   int y=0;
   while(y<funcsCount && strlen(funcs[y]) > 4){
      printf("%s\n",funcs[y]);
      y++;
   }
   
}


// Print register
int 
printreg(int PC) {
   
   // get rA
   PC++;
   int rA = (unsigned char)memory[PC];
   rA = rA >> 4;
   
   char buffer[30];
   sprintf(buffer, "printreg %s  # = 0x%x\n", regname[rA],regs[rA] );

   // if in function call strcat to respective function call
   if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
   else  printf("\tprintreg %s   #  = 0x%x\n",regname[rA],regs[rA]);   

    
   return PC+1;
}

// print all registers
int
printall (int PC) {
   
   // if in function call strcat to respective function call
   if ( inCall == 1 ) { strcat( funcs[funcsCount] , "printall\n"); }
   else  printf("\tprintall\n");
   
   printf("\n\t******************************\n");
   int cnt=0;
   while(cnt<15){
      printf("\t%s = %x\t",regname[cnt],regs[cnt]);
      cnt++;
      printf("\t%s = %x\n",regname[cnt],regs[cnt]);
      cnt++;
   }
   printf("\t******************************\n\n");
   
   return PC+1;
}

int
printmem(int PC) {

  // get rA
  PC++;
  int rA = (unsigned char) memory[PC];
  rA = rA >> 4;
  PC++;

  // get displacement
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
   
   
   // read from new displacement
   int startBlock = newDisplacement;
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      startBlock++;
   }
   
   // if in function call strcat to respective function call
   char buffer[30];
   sprintf( buffer, "printmem %x(%s)\n", displacement, *(regname+rA) );

   if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
   else  printf("\tprintmem %x(%s)\n", displacement, *(regname+rA) );

   
   return PC + 8;
} 

int
IRmov(int PC) {
   
   // get rB
   PC++;
   unsigned char r=memory[PC];
   unsigned int rB=r&0xf;

   PC++; 
   
   // get 8 byte value of the immediate value
   int ind = PC+7;
   unsigned char bits[8];
   int index = 0;
   for( ; ind >= PC ; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }

   // move value to register
   regs[rB] = getHexValue(bits,8) + 0;
   
   // if in function call strcat to respective function call
   char buffer[30];
   sprintf( buffer, "irmovq $0x%x, %s\n", regs[rB], *(regname+rB) );
   
   if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
   else  printf("\tirmovq $0x%x, %s\n", regs[rB], *(regname+rB));
   
   
   return PC+8;
}

int
RRmov(int PC) {
   
   // get fn
   unsigned char t1 = memory[PC];
   unsigned int fn = t1&0xf;
   
   // get rB and rA
   unsigned char r = memory[PC+1];
   unsigned int rB = r&0xf;
   r >>= 4;
   unsigned int rA = r&0xf;
   
   // compare rA and rB
   int answer = regs[rB] - regs[rA];
   if (answer==0x00) ZF=0x01;
   else ZF=0x00;
   if (answer<0x0) SF=0x01;
   else SF=0x00;
   
   switch(fn){
      
      case 0: // equal
         regs[rB] = regs[rA];
         break;
      case 1: // less than or equal
         if ( (SF^OF)|ZF ) regs[rB] = regs[rA];
         break;
      case 2: // less than
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
   
   // if in function call strcat to respective function call
   char buffer[30];
   sprintf( buffer, "rrmovq %s, %s\n", regname[rA], regname[rB] );
   
   if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
   else  printf("\trrmovq %s, %s\n", regname[rA], regname[rB]);
   
   return PC+2;
}

int
RMmov(int PC) {
   
   // get rA and rB
   unsigned char r = memory[PC+1];
   unsigned int rB = r&0xf;
   r >>= 4;
   unsigned int rA = r&0xf;
   PC+=2;
   
   // get value of displacement
   int ind = PC+7;
   unsigned char bits[8];
   int index = 0;
   for( ; ind >= PC ; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }
   
   //  get displacement + rB 
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

   // if in function call strcat to respective function call
   char buffer[30];
   sprintf( buffer, "rmmovq %s, 0x%x(%s)\n", regname[rA], currentBlock, regname[rB] );
   
   if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
   else  printf("\trmmovq %s, 0x%x(%s)\n", regname[rA], currentBlock, regname[rB]);
   
   return PC+8;
}

int
MRmov(int PC) {
   
   // get rB and rA
   unsigned char r = memory[PC+1];
   unsigned int rB = r&0xf;
   r >>= 4;
   unsigned int rA = r&0xf;
   PC+=2;
   
   // get displacement value
   int ind = PC+7;
   unsigned char bits[8];
   int index = 0;
   for( ; ind >= PC ; ind-- ){
    int b = memory[ind]&0xff;
    bits[index] = (unsigned char) b&0xff;
    index++;
   }
   
   // get data from memory at displacement value
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
   
   // update register
   regs[rA] = getHexValue(storedValue,8);

   // if in function call strcat to respective function call
   char buffer[30];
   sprintf( buffer, "mrmovq %x(%s), %s\n", initialDisplacement, regname[rB], regname[rA]);
   
   if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
   else  printf("\tmrmovq %x(%s), %s\n", initialDisplacement, regname[rB], regname[rA]);
   
   return PC+8;
}

int
Jump(int PC ) {
   
   // get fn
   unsigned char r = memory[PC];
   char fn = r&0xf;
   
   // get address for jump
   int startBlock = PC+1;
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      startBlock++;
   }
   
   char buffer[30];
   
   // check fn, for the correct condition
   // then jump to address or next instruction
   int jmpAddress = getHexValue(byte16,8);
   switch(fn){
      case 0:
         PC = jmpAddress;
         
         sprintf( buffer, "jmp %x\n", jmpAddress);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tjmp %x\n",jmpAddress);
         
         break;
      case 1:
         if ( (SF^OF)|ZF ) PC = jmpAddress; //update PC to address
         else PC = PC + 9;
         sprintf( buffer, "jle %x\n", jmpAddress);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tjle %x\n",jmpAddress);
         
         break;
      case 2:
         if ( SF^OF ) PC = jmpAddress;
         else PC = PC + 9;
         sprintf( buffer, "jl %x\n", jmpAddress);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tjl %x\n",jmpAddress);
         
         break;
      case 3:
         if ( ZF ) PC = jmpAddress;
         else PC = PC + 9;
         sprintf( buffer, "je %x\n", jmpAddress);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tje %x\n",jmpAddress);
         
         break;
      case 4:
         if ( ~ZF ) PC = jmpAddress;
         else PC = PC + 9;
         sprintf( buffer, "jne %x\n", jmpAddress);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tjne %x\n",jmpAddress);
         
         break;
      case 5:
         if ( ~(SF^OF)  ) PC = jmpAddress;
         else PC = PC + 9;
         sprintf( buffer, "jge %x\n", jmpAddress);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tjge %x\n",jmpAddress);
         
         break;
      case 6:
         if ( ~(SF^OF)&~ZF ) PC =  jmpAddress;
         else PC = PC + 9;
         sprintf( buffer, "jg %x\n", jmpAddress);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tjg %x\n",jmpAddress);
         
         break;
   }
	return PC;
}

int
Call(int PC) {
   
   int nextInstruction = PC+9; 

   char stackTop = regs[4]; // get address of stack from %rsp
   
   int newTop = stackTop - 0x8;
   regs[4] = newTop;
   
   
   // write next instruction onto stack 
   int currentBlock = newTop;
   while(currentBlock <= newTop+0x7){
      memory[currentBlock] = (unsigned char) nextInstruction&0xff;
      nextInstruction >>=8;
      currentBlock++;
   }
   
   // get address for call
   int startBlock = PC+1;
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      startBlock++;
   }
   
   // if in function call strcat to respective function call
   char buf[30];
   sprintf(buf,"Fun%d : \t",funcsCount);
   
   if ( inCall == 0 ) { inCall = 1; strcpy(funcs[funcsCount],buf); }
   
   
   
   printf("\tcall fun%d\n",funcsCount);
   return getHexValue(byte16,8);
}

int
Ret(int PC) {
   
   PC++;
   
   // get rsp 
   int rsp = regs[4];
   int startBlock = rsp;
   regs[4]+=0x8;
   
   // if this instruction is in a call, strcat the respective function
   // else print ret
   if ( inCall == 1 ){
      strcat(funcs[funcsCount],"\tret");
      inCall = 0 ;
   } else printf("\tret\n");
   
   funcsCount++;
   
   return memory[startBlock];
}

int
Push(int PC) {
   
   // get rA and data from register
   unsigned char r = memory[PC+1];
   r >>= 4;
   int rA = r&0xf;
   int data = regs[rA];
   
   char stackTop = regs[4]; // get address of stack from %rsp
   
   // set new stack top
   int newTop = stackTop - 0x8;
   
   // update rsp
   regs[4] = newTop;
   
   
   //write to newtop
   //writes content of register to blocks of memory (address of displacment + )
   int currentBlock = newTop;
   while(currentBlock <= newTop+0x7){
      memory[currentBlock] = (unsigned char) data&0xff;
      data >>=8;
      currentBlock++;
   }
   
   char buffer[30];
   sprintf( buffer, "pushq %s\n", *(regname+rA) );
   
   if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
   else  printf("\tpushq %s\n",*(regname+rA));

  return PC+2;
}


int
Pop(int PC) {
   
   // get rA
   unsigned char r = memory[PC+1];
   r >>= 4;
   int rA = r&0xf;
   
   // get %rsp
   int rsp = regs[4];
   int startBlock = rsp;
   
   // get data from memory starting at %rsp value
   unsigned char byte16[8];
   int cnt = 7;
   for ( ; cnt >= 0 ; cnt-- ){
      int byte = memory[startBlock]&0xff;
      byte16[cnt] = (unsigned char)byte&0xff;
      startBlock++;
   }
   
   // update register
   regs[rA] = getHexValue(byte16,8);
   
   // update rsp
   regs[4] = rsp + 0x8;
   
   // if in function call strcat to respective function call
   char buffer[30];
   sprintf( buffer, "\tpopq %s\n", *(regname+rA) );
   
   if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
   else  printf("\tpopq %s\n",*(regname+rA));
   
  return PC+2;
}


int
OPx(int PC) {
   
   // get fn(operation), rA, and rB
   unsigned char r = memory[PC];
   int operation = r&0xf;
   r = memory[PC+1];
   
   unsigned int rB = r&0xf;
   r >>= 4;
   unsigned int rA = r&0xf;
   
   unsigned int answer = 0;
   char buffer[30];
   
   // Do operation based on fn, update register, then update condition codes
   switch(operation) {
      case 0: // addq
         answer = regs[rA] + regs[rB];
         regs[rB] = answer;
         if (regs[rB] - regs[rA] == 0x0) ZF=0x01;
         else ZF=0x00;
         if (regs[rB] - regs[rA]<0x0) SF=0x01;
         else SF=0x00;
         
         sprintf( buffer, "addq %s, %s\n", regname[rA], regname[rB]);
         
         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\taddq %s, %s\n", regname[rA], regname[rB]);
   
         break;
      case 1: // subq
         answer = regs[rB] - regs[rA];
         regs[rB] = answer;
         if (answer==0x00) ZF=0x01;
         else ZF=0x00;
         if (regs[rB] - regs[rA]<0x0) SF=0x01;
         else SF=0x00;
         
         sprintf( buffer, "subq %s, %s\n", regname[rA], regname[rB]);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tsubq %s, %s\n", regname[rA], regname[rB]);

         break;
      case 2: // andq
         answer = regs[rB] & regs[rA];
         regs[rB] = answer;
         if (regs[rB] - regs[rA] == 0x0) ZF=0x01;
         else ZF=0x00;
         if (regs[rB] - regs[rA]<0x0) SF=0x01;
         else SF=0x00;
         
         sprintf( buffer, "andq %s, %s\n", regname[rA], regname[rB]);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\tandq %s, %s\n", regname[rA], regname[rB]);

         break;
      case 3: // xorq
         answer = regs[rB] ^ regs[rA];
         regs[rB] = answer;
         if (regs[rB] - regs[rA] == 0x0) ZF=0x01;
         else ZF=0x00;
         if (regs[rB] - regs[rA]<0x0) SF=0x01;
         else SF=0x00;
         
         sprintf( buffer, "xorq %s, %s\n", regname[rA], regname[rB]);

         if ( inCall == 1 ) { strcat( funcs[funcsCount] , buffer); }
         else  printf("\txorq %s, %s\n", regname[rA], regname[rB]);

         break;
   }
   
   return PC+2;
}

/*
 * Passed a char array(blocks of memory) returns hex value of the memory value
 */
int getHexValue(unsigned char *ptr, int bitCount){
  int i=1;
  int val=(*(ptr) << 8) | *(ptr++);

  while(i<bitCount){
   val=(val << 8) | *(ptr++);
   i++;
  }
  return val;
}

/*
 * Prints bytes of memory from start block to end block
 */
void printMemory(int startBlock, int endBlock){
   int ind = startBlock;
   while( ind >= startBlock && ind<=endBlock){
      printf("Mem[%x] = %x\n",ind,memory[ind]);
      ind++;
   }
   
}