#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf("Usage: vmtest <command>\n");
    exit(1);
  }

  int pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    exit(1);
  }

  if(pid == 0){
    setvmprint(1);
    if(exec(argv[1], &argv[1]) < 0){
      printf("exec failed!\n");
      exit(1);
    }
  } else {
    wait(0);
  }
  
  exit(0);
}