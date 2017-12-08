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
Cache cache[32]; //


void
initialize() {
/* if there is any initialization you would like to have, do it here */
/*  This is called for you in the main program */
    
    
    for(int z=0;z<32;z++){
        tlbentries[z].index = 0x0;
        tlbentries[z].tag = 0x0;
        tlbentries[z].ppn = 0x0;
        tlbentries[z].valid = 0;
    }
    
    for(int z=0;z<32;z++){
        cache[z].index = 0x0;
        cache[z].tag = 0x0;
        cache[z].data = 0x0;
        cache[z].valid = 0;
        
        cache[z].tag2 = 0x0;
        cache[z].data2 = 0x0;
        cache[z].valid2 = 0;
    }
    

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
          NOT DONE
     * if PPN is in the TLB, 
          compute PA 
          log_entry(ADDRESS_FROM_TLB,PA);
          return PA
          DONE
     * else use the page table function to get the PPN:
          PPN = get_page_table_entry(VPN) // provided function
          compute PA 
          log_entry(ADDRESS_FROM_PAGETABLE,PA);
          return PA
          DONE
*/

    int PA;
    
    char x[32];
    sprintf(x,"%x",virt_address);
    int virtuality = (int)strtol(x, NULL, 16);
    
    
    int vpo = virtuality & 0x1ff;
    virtuality >>= 9;
    int vpn = virtuality;
    
    int tlbi = virtuality & 0xf;
    virtuality >>= 4;
    int tlbt = virtuality & 0x1f;

    int ppn = -1;
    
    // checks the TLB table for the ppn
    if( tlbentries[tlbi].tag == tlbt && tlbentries[tlbi].valid == 1 ){
        ppn = tlbentries[tlbi].ppn;
        
        PA = ppn;
        PA = (PA << 9) | vpo;
   //     log_entry(ADDRESS_FROM_TLB,PA);
    }

    
    // get PPN from page table and update TLB
    if( ppn == -1) {
     ppn = get_page_table_entry(vpn);
     
     tlbentries[tlbi].ppn = ppn;
     tlbentries[tlbi].tag = tlbt;
     tlbentries[tlbi].index = tlbi;
     tlbentries[tlbi].valid = 1;
   //  printf("Updated TLB\n");
     PA = ppn;
     PA = (PA << 9) | vpo;
  //   log_entry(ADDRESS_FROM_PAGETABLE,PA);
    }
    //printf("tlb idx= %x\n",tlbi);
    //printf("PPN = %x\n",ppn);
     
     
    // adds the vpo to the ppn
    // pa is the physicaladdress
    

   // printf("Word %d\tPage %d\n",get_word(0x8485),get_page_table_entry(vpn));

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
    int foundInCache = 0;
   
    int pareplica = phys_address;
    int co = pareplica & 0x3;
    pareplica >>= 2 ;
    int ci = pareplica & 0x1f;
    pareplica >>= 5;
    int ct = pareplica & 0x1fff;
    
    int cacheData = -1;
    

     if( cache[ci].tag == ct && cache[ci].valid == 1){
        cacheData = cache[ci].data;
    //    printf("Found data in cache: %x\n",cacheData);
        foundInCache = 1;
     }
     if( cache[ci].tag2 == ct && cache[ci].valid2 == 1){
        cacheData = cache[ci].data2;
    //    printf("Found data in cache: %x\n",cacheData);
        foundInCache = 1;
     }

    
   // data was found in cache
   if ( cacheData == -1 ){
    cacheData = get_word(phys_address);
    //update cache
    updateCache(phys_address,cacheData);
   }
   
   printf("cache data = %x\n",cacheData);
   
   switch(co){
    case 0:
        byte = cacheData & 0xff;
        break;
    case 1:
        cacheData >>= 8;
      //  printf("shifted = %02x\n",cacheData);
        byte = cacheData & 0xff;
        break;
    case 2:
        cacheData >>= 16;
      //  printf("shifted = %02x\n",cacheData);
        byte = cacheData & 0xff;
        break;
    case 3:
        cacheData >>= 24;
        //printf("shifted = %x\n",cacheData);
        byte = cacheData & 0xff;
        break;
   }
   
   if (foundInCache == 1)   log_entry(DATA_FROM_CACHE,byte&0xff);
   else     log_entry(DATA_FROM_MEMORY,byte&0xff);
   
   
   //print();
  // ("Byte = %x\n",byte &0xff);
   return byte;
}



void updateCache(int pa, int data){
    int pareplica = pa;
    pareplica >>= 2 ;
    int ci = pareplica & 0x1f;
    pareplica >>= 5;
    int ct = pareplica & 0x1fff;
    
    /*
     * check cache at index
     * if block1 or block2 valid == 0
     *  update block 1 or 2
     * if block 1 and block 2 valid == 1
     *  check both times
     *   update oldest
     */
    
    if (cache[ci].valid == 0 || cache[ci].valid2 == 0){
        if(cache[ci].valid == 0){
            cache[ci].valid = 1;
            cache[ci].index = ci;
            cache[ci].tag = ct;
            cache[ci].data = data;
            cache[ci].time = (unsigned long)time(NULL);
        }else{
            cache[ci].valid2 = 1;
            cache[ci].index = ci;
            cache[ci].tag2 = ct;
            cache[ci].data2 = data;
            cache[ci].time2 = (unsigned long)time(NULL);
        }
    }
    else if ( cache[ci].valid == 1 && cache[ci].valid2 == 1){
        if( cache[ci].time < cache[ci].time2 ){
            cache[ci].valid = 1;
            cache[ci].index = ci;
            cache[ci].tag = ct;
            cache[ci].data = data;
            cache[ci].time = (unsigned long)time(NULL);
        }else{
            cache[ci].valid2 = 1;
            cache[ci].index = ci;
            cache[ci].tag2 = ct;
            cache[ci].data2 = data;
            cache[ci].time2 = (unsigned long)time(NULL);
        }
    }
    
    printf("Updated cache\n");
}


void
print(){
    
    printf("\n\nTLB\n------------------------------\nIndx\tTag\tPPN\tValid\n");
    for(int i=0; i<32 ; i++){
        if(tlbentries[i].valid!=0) printf("%x\t%x\t%x\t\%x\n",tlbentries[i].index, tlbentries[i].tag, tlbentries[i].ppn, tlbentries[i].valid);
    }
    printf("----------------------------\n\n");
    
   /* printf("Page Table\n-------------------------\nVPN\tPPN\tValid\n");
    for(int i=0; i<5 ; i++){
        printf("%x\t%x\t%x\n",pageentries[i].vpn, pageentries[i].ppn, pageentries[i].valid);
    }
    printf("--------------------------\n\n"); */
    
    printf("Cache\n------------------------------------------------------------------\nIndx\tTag\tValid\tData\t\tTime\t\n");
   for(int i=0; i<32 ; i++){
        if (cache[i].valid==1 || cache[i].valid2==1){
            printf("%x\t%x\t%x\t%x\t%lu\n",cache[i].index,cache[i].tag,cache[i].valid,cache[i].data,cache[i].time);
            printf("%x\t%x\t%x\t%x\t%lu\n",cache[i].index,cache[i].tag2,cache[i].valid2,cache[i].data2,cache[i].time2);
        }
    }
    printf("---------------------------------------------------------------------\n\n");
    
}



