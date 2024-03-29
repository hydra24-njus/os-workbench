#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <regex.h>
#include <dirent.h>
#include <time.h>

regex_t strace_time;
regex_t strace_name;

struct syscall{
  char name[256];
  double time;
}list[1024];
int list_cnt=0;
double total_time=0;
int cmp(const void *a,const void *b){
  return ((struct syscall*)b)->time>((struct syscall*)a)->time;
}

char __PATH[1024];
char* findpath(char* proc_name){
  char* path=strtok(__PATH,":");
  DIR* dir;
  struct dirent* entry;
  while(path){
    dir=opendir(path);
    if(!dir){
      path=strtok(NULL,":");
      continue;
    }
    while((entry=readdir(dir))!=NULL){
      if(strcmp(entry->d_name,proc_name)==0){
        closedir(dir);
        return path;
      }
    }
    closedir(dir);
    path=strtok(NULL,":");
  }
  return NULL;
}

void display(){
  qsort(list,list_cnt,sizeof(struct syscall),cmp);
  for(int i=0;i<5;i++){
    printf("%s\t",list[i].name);
    printf("(%d%%)\n",(int)(list[i].time*100/total_time));
  }
  printf("================================\n");
  for(int i=0;i<80;i++)printf("%c",'\0');
  fflush(stdout);
  fflush(stderr);
}
int main(int argc, char *argv[]) {
  char* exec_argv[argc+2];
  char** exec_env=__environ;


  exec_argv[0]="strace";
  exec_argv[1]="-T";
  memcpy(exec_argv+2,argv+1,(argc)*sizeof(char*));

  regcomp(&strace_name,"([a-zA-Z0-9]+_*)+\\(",REG_EXTENDED);
  regcomp(&strace_time,"<[0-9].[0-9]*>",REG_EXTENDED);

  strcpy(__PATH,getenv("PATH"));
  char* spath=findpath("strace");
  char strace_path[256];char* str="strace";
  sprintf(strace_path,"%s/%s",spath,str);

  char cmd_path[256];
  strcpy(__PATH,getenv("PATH"));
  if(strstr(argv[1],"/")==NULL){     
    char *_cmd_path=findpath(argv[1]);       
    sprintf(cmd_path,"%s/%s",_cmd_path,argv[1]);
    exec_argv[2]=cmd_path;
  }

  int fildes[2];
  if(pipe(fildes)!=0)assert(0);
  int pid=fork();
  if(pid==0){
    //TODO():连接管道
    close(fildes[0]);
    int fd=open("/dev/null",O_RDWR);
    dup2(fd,STDOUT_FILENO);
    dup2(fildes[1],STDERR_FILENO);
    execve(strace_path, exec_argv, exec_env);
    printf("should not reach here.\n");
    exit(1);
  }
  else{
    close(fildes[1]);
    dup2(fildes[0],STDIN_FILENO);
    char buf[4096];
    size_t nmatch=1;
    regmatch_t matchptr;
    time_t start,end;
    start=time(NULL);
    while(fgets(buf,4096,stdin)!=NULL){
      //正则表达式处理
      char name[256];char stime[256];
      memset(name,0,sizeof(name));
      memset(stime,0,sizeof(stime));
      if((regexec(&strace_name,buf,nmatch,&matchptr,0)!=REG_NOMATCH)&&
         (regexec(&strace_time,buf,nmatch,&matchptr,0)!=REG_NOMATCH)){
        regexec(&strace_name,buf,nmatch,&matchptr,0);
        memcpy(name,buf+matchptr.rm_so, matchptr.rm_eo-matchptr.rm_so);
        name[matchptr.rm_eo-matchptr.rm_so-1]='\0';
        regexec(&strace_time,buf,nmatch,&matchptr,0);
        memcpy(stime,buf+matchptr.rm_so+1, matchptr.rm_eo-matchptr.rm_so-1);
        stime[matchptr.rm_eo-matchptr.rm_so-2]='\0';
        int flag=-1;
        for(int i=0;i<list_cnt;i++){
          if(strcmp(list[i].name,name)==0){
            flag=i;break;
          }
        }
        if(flag==-1){
          flag=list_cnt;
          strcpy(list[list_cnt].name,name);
          list[list_cnt++].time=0;
        }
        list[flag].time+=atof(stime);
        total_time+=atof(stime);
        end=time(NULL);
        if(end-start>=1){
          display();
          start=end;
        }
      }
    }
    display();
  }
  return 0;
}
