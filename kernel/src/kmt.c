#include <os.h>

static Context *kmt_context_save(Event ev,Context *context){
  //TODO():save context
  
  return NULL;
}
static Context *kmt_schedule(Event ev,Context *context){
  //TODO():线程调度。
  //debug("schedule\n");
  return context;
}
void kmt_init(){
  //int x=cpu_count();

  os->on_irq(INT32_MIN+1,EVENT_NULL,kmt_context_save);
  os->on_irq(INT32_MAX,EVENT_NULL,kmt_schedule);
  return;
}
int create(task_t *task,const char *name,void (*entry)(void *arg),void *arg){
  return 0;
}
void teardown(task_t *task){
  return;
}
void spin_init(spinlock_t *lk,const char *name){
  return;
}
void spin_lock(spinlock_t *lk){

}
void spin_unlock(spinlock_t *lk){

}
void sem_init(sem_t *sem,const char *name,int value){

}
void sem_wait(sem_t *sem){

}
void sem_signal(sem_t *sem){

}
MODULE_DEF(kmt) = {
 // TODO
 .init=kmt_init,
 .create=create,
 .teardown=teardown,
 .spin_init=spin_init,
 .spin_lock=spin_lock,
 .spin_unlock=spin_unlock,
 .sem_init=sem_init,
 .sem_wait=sem_wait,
 .sem_signal=sem_signal
};
