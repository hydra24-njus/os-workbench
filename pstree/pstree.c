#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
const char *path ="/proc";

typedef struct{
  int pid;
  int ppid;
  char name[128];
  char state;
}Process;
typedef struct {
  bool show_pids;
  bool version;
  bool numeric_sort;
}Options;

Options cli;
Process process;
int procpid[1024];
static int isdigitstr(char *str){
  return (strspn(str,"0123456789")==strlen(str));
}

int readprocessfolder(){
  usigned int count=0;
  DIR *pDir=NULL;
  struct dirdent * pEnt =NULL;
  pDir=opendir("/proc");
  if(pDir==NULL){perror("error in readprocessfolder.\n");assert(0);}
  while(1){
    pEnt=readdir(pDir);
    if(pEnt!=NULL){
      if(dir->d_type!=DT_DIR)continue;
      if(!isdigitstr(dir->d_name)continue;
      //all process folder here.
      sscanf(dir->d_name,"%d",procpid[count++]);
    }
    else break;
  }
  closedir(pDir);
  return count;
}



int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    assert(argv[i]);
    //printf("argv[%d] = %s\n", i, argv[i]);
    if(strcmp(argv[i],"-V")==0)state.version=1;
    else if(strcmp(argv[i],"-p")==0)state.show_pids=1;
    else if(strcmp(argv[i],"-n")==0)state.numeric_sort=1;
    else {
      printf("pstree: invalid option -- '%s'\n",argv[i]);
      return 0;
    }
  }
  assert(!argv[argc]);
  if(state.version){
    perror("pstree 1.0\n\tCopyright (C) 2022 hydra24.\n");
    return 0;
  }
  int n=readprocessfolder();
  for(inti=0;i<n;i++)printf("%d\n",propid[i]);
  return 0;
}
