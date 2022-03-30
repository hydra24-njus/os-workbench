#include <common.h>
#include <buddysystem.h>
#include <lock.h>
#define M16 (1<<24)
static tree* tree_head;
void buddy_init(uintptr_t heapstart,uintptr_t heapend){
    printf("%d\n",sizeof(tree));
    tree_head=(tree*)heapstart;heapstart+=sizeof(tree);
    uintptr_t page64k=heapstart;
    if(heapstart%M16!=0)heapstart=heapstart+M16-heapstart%M16;
    heapend=(heapend>>24)<<24;
    printf("%x~%x,size= %u MB\n",heapstart,heapend,(heapend-heapstart)>>20);
    int i=0;tree_head->free_list[M5]=heapstart;
    page_t* tmp=tree_head->free_list[M5];
    heapstart+=M16;
    while(heapstart<heapend){
        tmp->next=heapstart;
        heapstart+=M16;
        i++;
    }
    tmp->next=NULL;
    tmp=tree_head->free_list[M5];
    while(tmp->next!=NULL){
        printf("%x->",tmp);
    }
}