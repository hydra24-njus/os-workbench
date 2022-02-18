#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

const char *path ="/proc";


typedef struct {
  bool show_pids;
  bool version;
  bool numeric_sort;
}Options;
Options state;

int main(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
    if(strcmp(argv[i],"-V")==0)state.version=1;
    else if(strcmp(argv[i],"-p")==0)state.show_pids=1;
    else if(strcmp(argv[i],"-n")==0)state.numeric_sort=1;
    else {
      printf("error cle option.\n");
      assert(0);
    }
  }
  assert(!argv[argc]);
  if(state.version){
    perror("pstree 1.0\n\tCopyright (C) 2022 hydra24.\n");
    return 0;
    
  return 0;
}
