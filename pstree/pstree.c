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
Process process[1024];
int procpid[1024];



static int isdigitstr(char *str){return (strspn(str,"0123456789")==strlen(str));}

int getprocessfolder(){
  int cnt=0;
  DIR *d=opendir("/proc");
  struct dirent *dir;
  if(d==NULL){perror("error in readprocessfolder.\n");assert(0);}
  while((dir=readdir(d))!=NULL){
    if(dir->d_type!=DT_DIR)continue;
    if(!isdigitstr(dir->d_name))continue;
    sscanf(dir->d_name,"%d",&procpid[cnt++]);
  }
  closedir(d);
  return cnt;
}

int getprocess(int n){
  int cnt=0;
  char buf[64];
  char path[64];
  for(int i=0;i<n;i++){
    sprintf(path,"/proc/%d/stat",procpid[i]);
    FILE *fp=fopen(path,"r");
    if(fp==NULL){printf("%d:process not exit.\n",procpid[i]);assert(0);}
    fgets(buf,sizeof(buf),fp);
    fclose(fp);
    sscanf(buf,"%d",&process[cnt].pid);
    int k=0;
    while(buf[k++]!=' ');
    int k1=63;
    while(buf[k1--]!=')');
    k1++;
    strncpy(process[cnt].name,buf+k+1,k1-k-2);
    //printf("%s\n",buf+k1+1);
    sscanf(buf+k1+1,"%c %d",&process[cnt].state,&process[cnt].ppid);
    //printf("%d (%s) %c %d\n",process[cnt].pid,process[cnt].name,process[cnt].state,process[cnt].ppid);
    //printf("%s\n\n",buf);
    cnt++;
  }
  return cnt;
}

void print_tree(int root,int deep){
printf("%s\t%d\n",process[root].name,process[root].pid);
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
  int n=getprocessfolder();
  n=getprocess(n);
  for(int i=0;i<n;i++)if(process[i].ppid==0)print_tree(i,0);
  return 0;
}
