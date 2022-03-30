#include <common.h>
#include <buddysystem.h>
#include <lock.h>
#define M16 (1<<24)
static tree* tree_head;
void buddy_init(uintptr_t heapstart,uintptr_t heapend){
    printf("%d\n",sizeof(tree));
    tree_head=(tree*)heapstart;heapstart+=sizeof(tree);
    if(heapstart%M16!=0)heapstart=heapstart+M16-heapstart%M16;
    heapend=(heapend<<24)>>24;
    printf("%x~%x,size= %d MB\n",heapstart,heapend,(heapend-heapstart)>>20);
}