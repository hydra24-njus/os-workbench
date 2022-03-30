#include <common.h>
#include <buddysystem.h>
#include <lock.h>
#define M16 (1<<24)

typedef struct pagecontrol{
    int size;
    int type;
    int state;
    void* next;
}page_t;

typedef union{
    struct{
        void* free_list[MAX_ORDER];
        page_t units[8192];
        spinlock_t tree_lock;
    };
    uint8_t data[1<<18];
}tree;//2^18 Byte


static tree* tree_head;
void buddy_init(uintptr_t heapstart,uintptr_t heapend){
    printf("%d\n",sizeof(tree));
    tree_head=(tree*)heapstart;heapstart+=sizeof(tree);
    //uintptr_t page64k=heapstart;
    if(heapstart%M16!=0)heapstart=heapstart+M16-heapstart%M16;
    heapend=(heapend>>24)<<24;
    printf("%x~%x,size= %u MB\n",heapstart,heapend,(heapend-heapstart)>>20);
    //int i=0;tree_head->units[0]=

}