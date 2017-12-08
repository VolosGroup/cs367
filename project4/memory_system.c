// Do not edit this file.

#include <stdio.h>
#include <stdlib.h>
#include "memory_system.h"

static FILE *log_file;

int
main() {
  int virt_address;
  int phy_address;
  start_logging();
  
  initialize();

  printf("> ");
  scanf("%x",&virt_address);
  while (virt_address >= 0) {
// While loop that takes the input virtual address, maps it to
// a physical address, and then does a lookup to get the corresponding
// byte.
     log_entry(NEW_ADDRESS,virt_address);

     phy_address = get_physical_address(virt_address);   // implement this function
// Assumes function returns -1 if address is illegal 
     if (phy_address != -1) {
        char ch = get_byte(phy_address);  // implement this function
     }
     printf("> ");
     scanf("%x",&virt_address);
  }
  stop_logging();
}



void start_logging() {
   log_file = fopen("project4_logfile","w");
}

void stop_logging() {
   fclose(log_file);
}

/* you can add other types of logging but be sure to disable this before
 * submitting
 */

void log_entry(int type, int data) {
   switch (type) {
     case NEW_ADDRESS:
  	fprintf(log_file,"Virtual Address: 0x%x \n",data); break;
     case DATA_FROM_MEMORY:
  	fprintf(log_file,"\tData: 0x%x from memory\n",data); break;
     case DATA_FROM_CACHE:
  	fprintf(log_file,"\tData: 0x%x from cache\n",data); break;
     case ADDRESS_FROM_PAGETABLE:
  	fprintf(log_file,"\tPhysical Address: 0x%x from page table\n",data); break;
     case ADDRESS_FROM_TLB:
  	fprintf(log_file,"\tPhysical Address: 0x%x from TLB\n",data); break;
     case ILLEGALVPN:
  	fprintf(log_file,"\tIllegal VPN generated: 0x%x \n",data); break;
     case PHYSICALERROR:
  	fprintf(log_file,"\tIllegal Physical Address generated: 0x%x \n",data); break;
     case ILLEGALVIRTUAL:
  	fprintf(log_file,"\tIllegal Virtual Address as input: 0x%x \n",data); break;
   }
}
