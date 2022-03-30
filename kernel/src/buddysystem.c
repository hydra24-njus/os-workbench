#include <common.h>
#include <buddysystem.h>
#include <lock.h>
#define M16 (1<<24)
#define HEAPSTART 0x1000000
typedef struct{
    int size;
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

uintptr_t map2addr(uintptr_t map){
    uintptr_t num=(map-(uintptr_t)tree_head->units)/sizeof(page_t);
    num=HEAPSTART+num*(64<<10);
    return num;
}
uintptr_t addr2map(uintptr_t addr){
    uintptr_t num=(addr-HEAPSTART)/(64<<10);
    num=num*sizeof(page_t)+(uintptr_t)tree_head->units;
    return num;
}


void print_mem_tree(){
    for(int i=0;i<MAX_ORDER;i++){
    page_t* tmp=tree_head->free_list[i];
    printf("%d:",i);
    while(tmp!=NULL){
        uintptr_t num=map2addr((uintptr_t)tmp);
        printf("%x->",num);
        tmp=tmp->next;
    }
    printf("\n");
    }
}


void buddy_init(uintptr_t heapstart,uintptr_t heapend){
    printf("%d\n",sizeof(tree));
    tree_head=(tree*)heapstart;heapstart+=sizeof(tree);
    if(heapstart%M16!=0)heapstart=heapstart+M16-heapstart%M16;
    heapend=(heapend>>24)<<24;
    printf("%x~%x,size= %u MB\n",heapstart,heapend,(heapend-heapstart)>>20);
    memset(tree_head,0,sizeof(tree));
    int maxpage=((heapend-heapstart)>>20)/16;
    tree_head->free_list[M5]=&tree_head->units[0];
    for(int i=0;i<maxpage-1;i++){
        tree_head->units[i*256].size=M5;
        tree_head->units[i*256].state=0;
        tree_head->units[i*256].next=&tree_head->units[(i+1)*256];
    }
    tree_head->units[(maxpage-1)*256].next=NULL;
    print_mem_tree();
}

void* buddy_alloc(size_t size){
    int i=0;while((1<<i)<size)i++;
    if(tree_head->free_list[i]!=NULL){
        page_t* tmp=tree_head->free_list[i];
        tree_head->free_list[i]=tmp->next;
        tmp->next=NULL;
        uintptr_t num=((uintptr_t)tmp-(uintptr_t)tree_head->units)/sizeof(page_t);
        num=0x1000000+num*(64<<10);
        return (void*)num;
    }
    else{
        buddy_alloc(size<<1);
    }
    return NULL;
}