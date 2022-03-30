enum{
    K6=0,K7,K8,K9,M1,M2,M3,M4,M5
};
#define MAX_ORDER M5

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
    };
    uint8_t data[1<<18];
}tree;//2^18 Byte






void buddy_init(uintptr_t heapstart,uintptr_t heapend);
void* buddy_alloc(size_t size);
void buddy_free(void* ptr);