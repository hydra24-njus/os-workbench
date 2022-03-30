enum{
    K2_6=0,K2_7,K2_8,K2_9,M2_1,M2_2,M2_3,M2_4,M2_5
};
#define MAX_ORDER M2_5
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