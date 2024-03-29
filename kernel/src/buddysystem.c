#include <common.h>
#include <buddysystem.h>
#include <lock.h>
#define M16 (1<<24)

uintptr_t heap_start=0;
typedef struct{
    int size;
    int state;
    void* next;
}page_t;

typedef union{
    struct{
        void* free_list[MAX_ORDER];
        page_t units[8192];
        pmm_spinlock_t tree_lock;
    };
    uint8_t data[1<<18];
}tree;//2^18 Byte
static tree* tree_head;

uintptr_t map2addr(uintptr_t map){
    uintptr_t num=(map-(uintptr_t)tree_head->units)/sizeof(page_t);
    num=heap_start+num*(64<<10);
    return num;
}
uintptr_t addr2map(uintptr_t addr){
    uintptr_t num=addr-heap_start;
    num=num/(64<<10);
    num=num*sizeof(page_t)+(uintptr_t)tree_head->units;
    return num;
}


void print_mem_tree(){
    for(int i=0;i<MAX_ORDER;i++){
    page_t* tmp=tree_head->free_list[i];
    printf("%d:",i);
    while(tmp!=NULL){
        uintptr_t num=map2addr((uintptr_t)tmp);
        printf("%x(%d)->",num,tmp->size);
        tmp=tmp->next;
    }
    printf("\n");
    }
}


void buddy_init(uintptr_t heapstart,uintptr_t heapend){
    tree_head=(tree*)heapstart;heapstart+=sizeof(tree);
    if(heapstart%M16!=0)heapstart=heapstart+M16-heapstart%M16;
    heapend=(heapend>>24)<<24;
    heap_start=heapstart;
    memset(tree_head,0,sizeof(tree));
    int maxpage=((heapend-heapstart)>>20)/16;
    tree_head->free_list[M5]=&tree_head->units[0];
    for(int i=0;i<maxpage-1;i++){
        tree_head->units[i*256].size=M5;
        tree_head->units[i*256].state=0;
        tree_head->units[i*256].next=&tree_head->units[(i+1)*256];
    }
    tree_head->units[(maxpage-1)*256].size=M5;
    tree_head->units[(maxpage-1)*256].state=0;
    tree_head->units[(maxpage-1)*256].next=NULL;
}

void* buddy_alloc(size_t size){
    if(size>M16)return NULL;
    int i=0;while((1<<i)<size)i++;i-=16;
    if(tree_head->free_list[i]!=NULL){
        page_t* tmp=tree_head->free_list[i];
        tree_head->free_list[i]=tmp->next;
        tmp->next=NULL;
        tmp->state=1;
        uintptr_t num=map2addr((uintptr_t)tmp);
        return (void*)num;
    }
    else{
        uintptr_t addr=(uintptr_t)buddy_alloc(size<<1);
        if(addr==0)return NULL;
        page_t* tmp=(page_t*)addr2map(addr);
        page_t* tmp2=(page_t*)addr2map(addr+size);
        tmp->state=1;tmp2->state=0;
        tree_head->free_list[i]=(void*)tmp2;
        tmp->size=i;tmp2->size=i;
        return (void*)addr;
    }
    return NULL;
}
void buddy_free(void* addr){
    page_t* map=(page_t*)addr2map((uintptr_t)addr);
    map->state=0;
    for(int i=map->size;i<MAX_ORDER-1;i++){
        //找到next_page
        int num=(int)(map-tree_head->units);
        page_t* next_page=NULL;int flag=0;
        if(num%(1<<(map->size+1))==0){
            flag=1;
            next_page=map+(1<<map->size);
        }
        else next_page=map-(1<<map->size);
        if(next_page->state==1)break;
        //释放next_page
        page_t* tmp=tree_head->free_list[map->size];
        if(tmp==next_page){tree_head->free_list[map->size]=tmp->next;}
        else{
            while(tmp->next!=next_page)tmp=tmp->next;
            tmp->next=next_page->next;
        }
        //合并
        next_page->next=NULL;
        if(flag==1){//map first
            map->size=map->size+1;
            next_page->size=0;
        }
        else{//next_page first
            map->size=0;
            next_page->size=next_page->size+1;
            map=next_page;
        }
    }
    //重新加入链表
    map->next=tree_head->free_list[map->size];
    tree_head->free_list[map->size]=map;
}