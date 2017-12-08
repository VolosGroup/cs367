#define OFFSET_BITS 9  // VPO and PPO
#define PPN_BITS 11   // using 20 bit physical addresses
#define VPN_BITS 9   // using 18 bit virtual addresses

//   For the logging system
#define NEW_ADDRESS 0
#define ADDRESS_FROM_TLB 1
#define ADDRESS_FROM_PAGETABLE 2
#define DATA_FROM_CACHE 3
#define DATA_FROM_MEMORY 4
#define ILLEGALVIRTUAL 5
#define PHYSICALERROR 6
#define ILLEGALVPN 7

#ifndef MEMORY_SYSTEM_H
#define MEMORY_SYSTEM_H


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

#endif

extern TLB tlbentries[32]; // maybe array size should be 16?
extern Cache cache[32]; //
int getHex(int n);
void updateCache(int pa, int data);
void print();

void initialize();
int get_physical_address(int);
char get_byte(int);

extern int get_page_table_entry(int) ;
extern int get_word(int);
void  start_logging(), stop_logging(), log_entry(int,int);




