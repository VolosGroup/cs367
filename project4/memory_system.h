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
    int data;
}   Cache;

#endif

void print();
void initialize();
int get_physical_address(int);
char get_byte(int);

extern int get_page_table_entry(int) ;
extern int get_word(int);
void  start_logging(), stop_logging(), log_entry(int,int);




