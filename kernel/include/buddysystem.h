enum{
    K6=0,K7,K8,K9,M1,M2,M3,M4,M5,MAX_ORDER
};


void buddy_init(uintptr_t heapstart,uintptr_t heapend);
void* buddy_alloc(size_t size);
void buddy_free(void* ptr);
void print_mem_tree();