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

  setvmprint(1);

  if(exec(argv[1], &argv[1]) < 0){
    printf("exec failed!\n");
    exit(1);
  }
  
  exit(0);
}