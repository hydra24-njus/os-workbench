#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctype.h>
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
  int count=0;
  DIR *Dir=opendir("/proc");
  struct dirdent *dir;
  if(Dir==NULL){perror("error in readprocessfolder.\n");assert(0);}
  while((dir=readdir(Dir))!=NULL){
    if(dir->d_type!=DT_DIR)continue;
    if(!isdigitstr(dir->d_name))continue;
    //all process folder here.
    sscanf(dir->d_name,"%d",procpid[count++]);
  }
  closedir(Dir);
  return count;
}



int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    assert(argv[i]);
    //printf("argv[%d] = %s\n", i, argv[i]);
    if(strcmp(argv[i],"-V")==0)cli.version=1;
    else if(strcmp(argv[i],"-p")==0)cli.show_pids=1;
    else if(strcmp(argv[i],"-n")==0)cli.numeric_sort=1;
    else {
      printf("pstree: invalid option -- '%s'\n",argv[i]);
      return 0;
    }
  }
  assert(!argv[argc]);
  if(cli.version){
    perror("pstree 1.0\n\tCopyright (C) 2022 hydra24.\n");
    return 0;
  }
  int n=readprocessfolder();
  for(int i=0;i<n;i++)printf("%d\n",procpid[i]);
  return 0;
}
