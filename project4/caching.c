#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "memory_system.h"

/*
 *  TLB will start have as invalid
 *  get ppn from page table and update the tlb by the set and tag of the virtual address
 *  cache will start as all invalid
 *  go get data from main memory and update cache by index and tag
 *  
 */



void print();
void updateCache(int pa, int data);
TLB tlbentries[32]; // maybe array size should be 16?

Cache cache[32];
int lastTLBidx;
int lastTLBtag;

void
initialize() {
/* if there is any initialization you would like to have, do it here */
/*  This is called for you in the main program */
    
   // for(int z=0; z<200;z++) printf("page[%x] = %d\n",z,get_page_table_entry(z));
   // exit(1);
    for(int z=0;z<32;z++){
        tlbentries[z].set = 0x0;
        tlbentries[z].tag = 0x0;
        tlbentries[z].ppn = 0x0;
        tlbentries[z].valid = 0;
    }
    
    for(int z=0;z<32;z++){
        cache[z].index = 0x0;
        cache[z].tag = 0x0;
        cache[z].data = 0x0;
        cache[z].valid = 0;
        cache[z].block = -1;
    }
    

    
    
 
    
    
    
    
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
    
    int vpo = virtuality & 0x1ff;
    virtuality >>= 9;
    int vpn = virtuality;
    
    int tlbi = virtuality & 0xf;
    lastTLBidx = tlbi;
    virtuality >>= 4;
    int tlbt = virtuality & 0x1f;
    lastTLBtag = tlbt;
    //printf("vpo %d\t tbli %d \t tlbt %d\n",vpo,tlbi, tlbt);

    int ppn = -1;

    printf("tlbi %x tlbt %x last ones %x %x\n",tlbi,tlbt,lastTLBidx,lastTLBtag);
    // checks the TLB table for the ppn
    for(int i=0; i<32; i++){
        if( tlbentries[i].tag == tlbt && tlbentries[i].set == tlbi )
         ppn = tlbentries[i].ppn;
         //printf("Found val : %x\n",tlbentries[i].ppn);
    }
    
    if( ppn == -1)
     ppn = get_page_table_entry(vpn);
    
    printf("PPN = %d\n",ppn);
     
     
    // adds the vpo to the ppn
    // pa is the physicaladdress
    PA = ppn;
    PA = (PA << 9) | vpo;

   // printf("Word %d\tPage %d\n",get_word(0x8485),get_page_table_entry(vpn));
    printf("PA %x\n",PA);
    
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

// get word goes to ram
*/

    char byte;
    printf("phys %x\n",phys_address);
   
    int pareplica = phys_address;
    int co = pareplica & 0x3;
    pareplica >>= 2 ;
    int ci = pareplica & 0x1f;
    pareplica >>= 5;
    int ct = pareplica & 0x1fff;
    
    int cacheData = -1;
    
    for(int i=0; i<32 ; i++){
     if(cache[i].index == ci && cache[i].tag == ct && cache[i].valid == 1){
        cacheData = cache[i].data;
        printf("Found data in cache: %x\n",cacheData);
     }
    }
    
   // data was found in cache
   if ( cacheData == -1 ){
    cacheData = get_word(phys_address);
    //update cache
    updateCache(phys_address,cacheData);
   }
   
   switch(co){
    case 0:
        break;
    case 1:
        break;
    case 2:
        break;
    case 3:
        break;
   }
   
   byte = (char) cacheData;
   
   return byte;
}



void updateCache(int pa, int data){
    int pareplica = pa;
    int co = pareplica & 0x3;
    pareplica >>= 2 ;
    int ci = pareplica & 0x1f;
    pareplica >>= 5;
    int ct = pareplica & 0x1fff;
    printf("cache[%x] : %x",ci,cache[ci].valid);

    cache[ci].valid = 1 ;
    cache[ci].index = ci;
    cache[ci].tag = ct;
    cache[ci].block = co;
    cache[ci].data = data;
    
}






void
print(){
    
    printf("\n\nTLB\n------------------------------\nSet\tTag\tPPN\tValid\n");
    for(int i=0; i<32 ; i++){
        if(tlbentries[i].valid!=0) printf("%x\t%x\t%x\t\%x\n",tlbentries[i].set, tlbentries[i].tag, tlbentries[i].ppn, tlbentries[i].valid);
    }
    printf("----------------------------\n\n");
    
   /* printf("Page Table\n-------------------------\nVPN\tPPN\tValid\n");
    for(int i=0; i<5 ; i++){
        printf("%x\t%x\t%x\n",pageentries[i].vpn, pageentries[i].ppn, pageentries[i].valid);
    }
    printf("--------------------------\n\n"); */
    
    printf("Cache\n-------------------------\nIndx\tTag\tValid\tData\n");
   for(int i=0; i<32 ; i++){
        if(cache[i].valid==1) printf("%x\t%x\t%x\t%x\n",cache[0].index,cache[0].tag,cache[0].valid,cache[0].data);
    }
    printf("--------------------------\n\n");
    
}



