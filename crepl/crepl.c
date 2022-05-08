#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>
#ifndef LOCAL_MACHINE
char* fpath="/tmp/func.c";
char* ppath="/tmp/test";
#else
char* fpath="./func.c";
char* ppath="./test";
#endif
#if defined(__i386__)
  #define TARGET "-m32"
#elif defined(__x86_64__)
  #define TARGET "-m64"
#endif
const char* header="#include<stdio.h>\n";
char func_str[4096*100];
char  pro_str[4096*100];
int fildes[2];

int main(int argc, char *argv[]) {
  static char line[4096];

  strcpy(func_str,header);
  if(pipe(fildes)!=0)assert(0);
  while (1) {
    //printf(">>");
    memset(line,0,sizeof(line));
    memset(pro_str,0,sizeof(pro_str));
    if(!fgets(line,sizeof(line),stdin))break;
    if(strlen(line)>2&&line[0]=='i'&&line[1]=='n'&&line[2]=='t'){
      strcat(func_str,line);
      strcpy( pro_str,func_str);
      strcat( pro_str,"int main(){printf(\"OK\\n\");}\0");
    }
    else{
      strcpy( pro_str,func_str);
      strcat( pro_str,"int main(){\nprintf(\"%d\\n\",");
      strcat( pro_str,line);
      strcat( pro_str,");}\0");
    }
    int pid=fork();
    if(pid==0){
      close(fildes[0]);
      FILE* fp=fopen(fpath,"w+");
      fprintf(fp,"%s\n",pro_str);
      fflush(fp);
      fclose(fp);
      dup2(fildes[1],STDOUT_FILENO);
      dup2(fildes[1],STDERR_FILENO);
      char *exec_argv[]={"gcc",TARGET,"-o",ppath,fpath,NULL};
      execvp("gcc",exec_argv);
      assert(0);
    }
    else{
      pid_t status;
      wait(&status);
      int ppid=fork();
      if(ppid==0){
        char *argv[]={NULL};
        execvp(ppath,argv);
        assert(0);
      }
      wait(&status);
    }
    fflush(stdout);
    /*printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    printf("Got %zu chars.\n", strlen(line)); // ??*/
  }
}
