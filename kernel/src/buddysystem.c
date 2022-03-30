#include <common.h>
#include <buddysystem.h>
#include <lock.h>
#define M16 (1<<24)

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
    page_t* tmp=tree_head->free_list[M5];
    while(tmp->next!=NULL){
        printf("%x->",tmp);
        tmp=tmp->next;
    }
}