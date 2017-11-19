#include <stdio.h>
#include <stdlib.h>

#define MAX_MEMORY 9000

char convert(char v1, char v2) ;
void get_input(FILE*);
extern void execute(int);

char memory[MAX_MEMORY] ;
int PC = 0;



int main(int argc, char **argv) {
   FILE *fp;
   if (argc > 1) {
     fp = fopen(argv[1],"r"); 
     if (!fp) {printf("Can't open file %s\n",argv[1]); exit(1); }
   } else {
      printf("Usage: %s <filename>\n",argv[0]);
      exit(1);
   }
   get_input(fp);
   execute(PC);
}


void
get_input(FILE *fp) {
   char c1,c2;
   char pc[32];
   int pos = 0;
   fscanf(fp,"%s",pc);
   PC = (long int)strtol(pc,NULL,16);

   fscanf(fp,"%c",&c1);
   while (fscanf(fp,"%c%c",&c1,&c2) > 1) {   
     if (pos == MAX_MEMORY) { printf("Input too large\nExecution will continue but results may be unreliable\n"); return; }
     memory[pos++] = convert(c1,c2);
   }

   for (;pos<MAX_MEMORY;pos++) memory[pos] = 0x00;
}

char convert(char v1, char v2) {
    int rval = 0;
    if (v1 >= '0' && v1 <= '9')
	    rval = (v1 - '0') << 4;
    else if (v1 >= 'a' && v1 <= 'f')
	    rval = ((v1 - 'a') + 10)<< 4;
    else if (v1 >= 'A' && v1 <= 'F')
	    rval = ((v1 - 'A') + 10)<< 4;
    if (v2 >= '0' && v2 <= '9')
	    rval += (v2 - '0') ;
    else if (v2 >= 'a' && v2 <= 'f')
	    rval += ((v2 - 'a') + 10);
    else if (v2 >= 'A' && v2 <= 'F')
	    rval += ((v2 - 'A') + 10);
//    printf("%c%c 0x%x\n",v1,v2,rval);
    return (char) rval;
}





