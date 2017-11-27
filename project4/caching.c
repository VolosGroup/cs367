#include <stdio.h>
#include <stdlib.h>
#include "memory_system.h"

typedef struct {
    int vpn;
    int ppn;
    int valid;
}   PageTable;

typedef struct {
    int set;
    int tag;
    int ppn;
    int valid;
}   TLB;

typedef struct {
    int index;
    int tag;
    int valid;
    int block0;
    int block1;
    int block2;
    int block3;
}   Cache;

void print();
TLB tlbentries[10];
PageTable pageentries[10];
Cache cache[10];
int lastTLB;
int lastPage;

void
initialize() {
/* if there is any initialization you would like to have, do it here */
/*  This is called for you in the main program */
    

    
    tlbentries[0].set = 0x0;
    tlbentries[0].tag = 0x3;
    tlbentries[0].ppn = 0x0;
    tlbentries[0].valid = 0;
    
    tlbentries[1].set = 0;
    tlbentries[1].tag = 0x9;
    tlbentries[1].ppn = 0x0d;
    tlbentries[1].valid = 1;
    
    tlbentries[2].set = 0;
    tlbentries[2].tag = 0x7;
    tlbentries[2].ppn = 0x2;
    tlbentries[2].valid = 1;
    
    tlbentries[3].set = 1;
    tlbentries[3].tag = 0x3;
    tlbentries[3].ppn = 0x2d;
    tlbentries[3].valid = 1;
    
    tlbentries[4].set = 1;
    tlbentries[4].tag = 0x2;
    tlbentries[4].ppn = 0;
    tlbentries[4].valid = 0;

    
    //
    
    pageentries[0].vpn = 0x0;
    pageentries[0].ppn = 0x28;
    pageentries[0].valid = 1;
    
    pageentries[1].vpn = 0x1;
    pageentries[1].ppn = 0;
    pageentries[1].valid = 0;
    
    pageentries[2].vpn = 0x3;
    pageentries[2].ppn = 0x33;
    pageentries[2].valid = 1;
    
    pageentries[3].vpn = 0x4;
    pageentries[3].ppn = 2;
    pageentries[3].valid = 1;
    
    pageentries[4].vpn = 8;
    pageentries[4].ppn = 0x13;
    pageentries[4].valid = 1;
    
    
    //
    
    cache[0].index = 0xe;
    cache[0].tag = 0x13;
    cache[0].valid = 1;
    cache[0].block0 = 0x83;
    cache[0].block1 = 0x77;
    cache[0].block2 = 0x1b;
    cache[0].block3 = 0xd3;
    
    print();
    
    
}

/* You will implement the two functions below:
 *     * you may add as many additional functions as you like
 *     * you may add header files for your data structures
 *     * you MUST call the relevant log_entry() functions (described below)
 *          or you will not receive credit for all of your hard work!
 */

int
get_physical_address(int virt_address) {
/*
   Convert the incoming virtual address to a physical address. 
     * if virt_address too large, 
          log_entry(ILLEGALVIRTUAL,virt_address); 
          return -1
     * if PPN is in the TLB, 
	  compute PA 
          log_entry(ADDRESS_FROM_TLB,PA);
          return PA
     * else use the page table function to get the PPN:
          PPN = get_page_table_entry(VPN) // provided function
          compute PA 
          log_entry(ADDRESS_FROM_PAGETABLE,PA);
          return PA
*/

    int PA;
    
    char x[32];
    sprintf(x,"%x",virt_address);
    int virtuality = (int)strtol(x, NULL, 16);
    
    int vpo = virtuality & 0xff;
    virtuality >>= 6;
    int vpn = virtuality;
    int tlbi = virtuality & 0x3;
    virtuality >>= 2;
    int tlbt = virtuality & 0x3f;
    //printf("vpo %d\t tbli %d \t tlbt %d\n",vpo,tlbi, tlbt);

    int ppn = -1;

    // checks the TLB table for the ppn
    for(int i=0; i<5; i++){
        if( (tlbentries[i].set == tlbi) && (tlbentries[i].tag == tlbt) ){
            lastTLB = i;
            printf("Found in TLB\n");
            ppn = tlbentries[i].ppn;
            break;
        } else{
            
        }
    }

    // checks page table for ppn if it hasn't been found yet
    // looks for ppn in the page table
    // by the vpn number
    if( ppn == -1 ){
        for(int i=0; i<5; i++){
            if( (pageentries[i].vpn == vpn) && (pageentries[i].valid == 1) ){
                lastPage = i;
                ppn = pageentries[i].ppn;
                PA = ppn;
                PA = (PA << 6) | vpo;
                printf("Found in page table\n");
                //update TLB
                // look for spot in tlb that has same set but valid = 0
                // set tag,ppn, valid = 1
                for(int y=0; y<10 ; y++){
                    if(tlbentries[y].set == tlbi && tlbentries[y].valid == 0){
                        tlbentries[y].ppn = ppn;
                        tlbentries[y].valid = 1;
                        tlbentries[y].tag = tlbt;
                    }
                }
                break;
            }
        }
    }
    else{
        // adds the vpo to the ppn
        // pa is the physicaladdress
        PA = ppn;
        PA = (PA << 6) | vpo;
        
        for(int i=0; i<5; i++){
            if( (pageentries[i].vpn == vpn) && (pageentries[i].valid == 1) ){
                lastPage = i;
                break;
            }
        }
    }
    
    return PA;
}
char
get_byte(int phys_address) {
/*
   Use the incoming physical address to find the relevant byte. 
     * if data is in the cache, use the offset (last 2 bits of address)
          to compute the byte to be returned data
          log_entry(DATA_FROM_CACHE,byte);
          return byte 
     * else use the function get_long_word(phys_address) to get the 
          4 bytes of data where the relevant byte will be at the
          given offset (last 2 bits of address)
          log_entry(DATA_FROM_MEMORY,byte);
          return byte

NOTE: if the incoming physical address is too large, there is an
error in the way you are computing it...
*/
   char byte;
   
   printf("phys %x\n",phys_address);
   int pareplica = phys_address;
   int co = pareplica & 0x3;
   pareplica >>= 2 ;
   int ci = pareplica & 0xf;
   pareplica >>= 4;
   int ct = pareplica & 0x3f;
   
   printf("co %x\tci %x\tct %x\n",co,ci,ct);
   
   for(int i=0; i<10 ; i++){
    //printf("vallue ci %x ct %x\n",cache[i].index, cache[i].tag);
    // found value in cache, put in 
    if(cache[i].index == ci && cache[i].tag == ct && cache[i].valid == 1){
        tlbentries[lastTLB].ppn = cache[i].tag;
        tlbentries[lastTLB].valid = 1;
        pageentries[lastPage].ppn = cache[i].tag;
        pageentries[lastPage].valid = 1;
    }
      
        
        
   }
   

   return byte;
}

void print(){
    
    printf("TLB\n------------------------------\nSet\tTag\tPPN\tValid\n");
    for(int i=0; i<5 ; i++){
        printf("%x\t%x\t%x\t\%x\n",tlbentries[i].set, tlbentries[i].tag, tlbentries[i].ppn, tlbentries[i].valid);
    }
    printf("----------------------------\n\n");
    
    printf("Page Table\n-------------------------\nVPN\tPPN\tValid\n");
    for(int i=0; i<5 ; i++){
        printf("%x\t%x\t%x\n",pageentries[i].vpn, pageentries[i].ppn, pageentries[i].valid);
    }
    printf("--------------------------\n\n");
    
}



