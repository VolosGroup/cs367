#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "memory_system.h"

typedef struct {
    int index;
    
    int tag;
    int ppn;
    int valid;
}   TLB;

typedef struct {
    int index;
    
    int tag;
    int valid;
    int data;
    long int time;
    
    int tag2;
    int valid2;
    int data2;
    long int time2;
    
}   Cache;


int getHex(int n); // Converts decimal value to hex

void updateCache(int pa, int data); // Updates cache 

TLB tlbentries[16]; // Array of TLB blocks
Cache cache[32]; // Array of cache blocks





void
initialize() {
    
    // Initialize all TLB blocks
    for( int z=0; z<16; z++ ){
        
        tlbentries[z].index = 0x0;
        
        tlbentries[z].tag = 0x0;
        tlbentries[z].ppn = 0x0;
        tlbentries[z].valid = 0;
        
    }
    
    // Initialize all cache blocks
    for( int z=0; z<32; z++ ){
        
        cache[z].index = 0x0;

        cache[z].tag = 0x0;
        cache[z].data = 0x0;
        cache[z].valid = 0;
        
        cache[z].tag2 = 0x0;
        cache[z].data2 = 0x0;
        cache[z].valid2 = 0;
        
    }
    

}





int
get_physical_address(int virt_address) {
    
    // virtual address is too big
    if ( virt_address > 262143 ){
        log_entry(ILLEGALVIRTUAL, virt_address);
        return -1;
    }


    int PA;
    
    int virtuality = getHex(virt_address); // get hex value of input

    int vpo = virtuality & 0x1ff; // VPO: 9 bits
    
    virtuality >>= 9; 
    
    int vpn = virtuality; // VPN: 9 bits
    
    int tlbi = virtuality & 0xf; // TLB index: 4 bits
    
    virtuality >>= 4;
    
    int tlbt = virtuality & 0x1f; // TLB tag: 5 bits

    int ppn = -1; // initialize PPN
    
    
    // VPN is too big
    if ( vpn > 511 ){
        log_entry(ILLEGALVPN, vpn); 
        return -1;
    }
    
    
    // checks the TLB table for the ppn
    if( tlbentries[tlbi].tag == tlbt && tlbentries[tlbi].valid == 1 ){
        ppn = tlbentries[tlbi].ppn;
        PA = ppn;
        PA = (PA << 9) | vpo;
        log_entry(ADDRESS_FROM_TLB,PA);
    }

    
    // PPN was not found in TLB
    if( ppn == -1) {
        
     ppn = get_page_table_entry(vpn); // get PPN from page table
     
     // update TLB block
     tlbentries[tlbi].ppn = ppn;
     tlbentries[tlbi].tag = tlbt;
     tlbentries[tlbi].index = tlbi;
     tlbentries[tlbi].valid = 1;
     
     // Set up physical address with PPN and VPO
     PA = ppn;
     PA = (PA << 9) | vpo;
     
     log_entry(ADDRESS_FROM_PAGETABLE,PA);
    }
    
    // physical address is too big
    if ( PA > 1048575 ){
        log_entry(PHYSICALERROR,PA);
        return -1;
    }
    
    return PA;
}





char
get_byte(int phys_address) {
    
    char byte;
    
    int foundInCache = 0; // tracks if data was found in cache
   
    int pareplica = phys_address; // replica of physical address for operations
    
    int co = pareplica & 0x3; // physical address offset
    
    pareplica >>= 2 ;
    
    int ci = pareplica & 0x1f; // physical address index
    
    pareplica >>= 5;
    
    int ct = pareplica & 0x1fff; // physical address tag
    
    int cacheData = -1;
    
    
    // check cache block 1
    if( cache[ci].tag == ct && cache[ci].valid == 1){
        cacheData = cache[ci].data; // data from cache
        foundInCache = 1;
    }
    
    // check cache block 2
    if( cache[ci].tag2 == ct && cache[ci].valid2 == 1){
        cacheData = cache[ci].data2; // data from cache
        foundInCache = 1;
    }

    
   // data was not found in cache
   if ( cacheData == -1 ){
    
    cacheData = get_word(phys_address); // get data from memory
    
    updateCache(phys_address,cacheData); // update cache
    
   }
   
   
   // get byte according to offset bits
   switch(co){
    
    case 0:
        byte = cacheData & 0xff;
        break;
    case 1:
        cacheData >>= 8;
        byte = cacheData & 0xff;
        break;
    case 2:
        cacheData >>= 16;
        byte = cacheData & 0xff;
        break;
    case 3:
        cacheData >>= 24;
        byte = cacheData & 0xff;
        break;
    
   }
   
   
   if ( foundInCache == 1 )   log_entry(DATA_FROM_CACHE,byte&0xff);
   else   log_entry(DATA_FROM_MEMORY,byte&0xff);
   
   
   return byte;
}





void
updateCache(int pa, int data){
    
    int pareplica = pa; // copy physical address
    
    pareplica >>= 2 ;
    
    int ci = pareplica & 0x1f; // Cache Index: 5 bits
    
    pareplica >>= 5;
    
    int ct = pareplica & 0x1fff; // Cache tag: 13 bits
    

// If block 1 or 2 are not set: check and update the one
// with valid equal to 0
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
    
// If both blocks are set, check which is oldest
// overwrite the oldest
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
}





// Converts an integer from decimal to hexadecimal
// n : decimal number
int
getHex(int n){
    
    char newhex[8]; // stores the hexadecimal digits
    
    int ind = 0; // counter for newhex
    
    int tmp; // holds remainder value of modulo 16
    
    
    // Loop to find the remainder of the decimal number and get correct hex value
    // puts hex value in newhex array
    while(n){
        
        tmp = n%16; // get remainder
        
        n /= 16; // divide decimal number by 16
        
        switch(tmp){
            case 0:
                newhex[ind] = (char)48;
                break;
            case 1:
                newhex[ind] = 48 + tmp;
                break;
            case 2:
                newhex[ind] = '2';
                break;
            case 3:
                newhex[ind] = '3';
                break;
            case 4:
                newhex[ind] = '4';
                break;
            case 5:
                newhex[ind] = '5';
                break;
            case 6:
                newhex[ind] = '6';
                break;
            case 7:
                newhex[ind] = '7';
                break;
            case 8:
                newhex[ind] = '8';
                break;
            case 9:
                newhex[ind] = '9';
                break;
            case 10:
                newhex[ind] = 'A';
                break;
            case 11:
                newhex[ind] = 'B';
                break;
            case 12:
                newhex[ind] = 'C';
                break;
            case 13:
                newhex[ind] = 'D';
                break;
            case 14:
                newhex[ind] = 'E';
                break;
            case 15:
                newhex[ind] = 'F';
                break;
        }
        ind++;
    }
    newhex[ind] = '\0'; // terminate string
    
    
    // Reverse the converted hex number so it is in correct order
    //
    int len = strlen(newhex); 
    
    int i = 0; 
    
    int j = strlen(newhex) - 1; 
    
    // Start at first and last characters
    // switch and increment/decrement until
    // all characters have been switched
    while( i < j ){
        
        char tmp = newhex[j]; 
        newhex[j] = newhex[i]; 
        newhex[i] = tmp;
        i++; j--;
        
    }
    
    int converted = (int)strtol(newhex,NULL,16); // put new hexadecimal value in int 
    
    return converted;
}





