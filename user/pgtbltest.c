#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "kernel/riscv.h"
#include "kernel/types.h"
#include "user/user.h"

// Forward declarations
void pgaccess_test(int test_num);
void simple_test_function();
void pgaccess_clear_test();
void pgaccess_read_write_test();
void pgaccess_boundary_test();

char *testname = "???";

void
err(char *why)
{
  printf("pgtbltest: %s failed: %s, pid=%d\n", testname, why, getpid());
  exit(1);
}

int
main(int argc, char *argv[])
{
  if (argc == 2) {
    int test_num = atoi(argv[1]);
    pgaccess_test(test_num);
  } 
  else {
    pgaccess_test(1);
  }

  printf("pgtbltest: all tests succeeded\n");
  exit(0);
}

void
pgaccess_test(int test_num)
{
  if (test_num == 1) {
    simple_test_function();
  } else if (test_num == 2) {
    pgaccess_clear_test();
  } else if (test_num == 3) {
    pgaccess_read_write_test();
  } else if (test_num == 4) {
    pgaccess_boundary_test();
  } else {
    printf("Invalid test number. Please use 1, 2, 3, or 4.\n");
    exit(1);
  }
}

// simple test to verify that pgaccess correctly identifies accessed pages
void
simple_test_function()
{
  char *buf;
  int abits;
  printf("simple_test_function starting\n");
  testname = "simple_test_function";
  
  buf = malloc(32 * PGSIZE);
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
    
  buf[PGSIZE * 1] += 1;
  buf[PGSIZE * 2] += 1;
  buf[PGSIZE * 30] += 1;
  
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
    
  if (abits != ((1 << 1) | (1 << 2) | (1 << 30)))
    err("incorrect access bits set");
    
  free(buf);
  printf("simple_test_function: OK\n");
}

// test to see if pgaccess clears the accessed bits after reading them, and that it can detect accesses again after clearing
void
pgaccess_clear_test()
{
  char *buf;
  int abits;
  printf("pgaccess_clear_test starting\n");
  testname = "pgaccess_clear_test";
  
  buf = malloc(32 * PGSIZE);
  pgaccess(buf, 32, &abits); // Clear initial state
  
  buf[PGSIZE * 5] += 1; // Access page 5
  
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
  if (abits != (1 << 5))
    err("did not detect access on page 5");
    
  // Check immediately again - the bit should now be 0
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
  if (abits != 0)
    err("PTE_A bit was not cleared by the previous pgaccess call!");
    
  free(buf);
  printf("pgaccess_clear_test: OK\n");
}

// test to verify that pgaccess can detect pure reads and pure writes,
// and that it does not get confused by volatile accesses that the compiler might optimize away
void
pgaccess_read_write_test()
{
  char *buf;
  int abits;
  
  printf("pgaccess_read_write_test starting\n");
  testname = "pgaccess_read_write_test";
  
  buf = malloc(32 * PGSIZE);
  pgaccess(buf, 32, &abits); // Clear initial state
  
  buf[PGSIZE * 3] = 42;    // Pure write
  volatile char x = buf[PGSIZE * 10]; // Pure read
  x += 1; // Use x to prevent compiler from optimizing away the read
  if (pgaccess(buf, 32, &abits) < 0)
    err("pgaccess failed");
    
  if (abits != ((1 << 3) | (1 << 10)))
    err("failed to detect pure read or pure write");
    
  free(buf);
  printf("pgaccess_read_write_test: OK\n");
}

// test to verify that pgaccess correctly handles boundary conditions, such as the last page in a large allocation
void
pgaccess_boundary_test()
{
  char *buf;
  uint64 abits; 
  
  printf("pgaccess_boundary_test starting\n");
  testname = "pgaccess_boundary_test";
  
  buf = malloc(64 * PGSIZE);
  pgaccess(buf, 64, (int*)&abits); // Clear initial state
  
  buf[0] = 1;           // Touch first page
  buf[PGSIZE * 63] = 1; // Touch 64th page
  
  if (pgaccess(buf, 64, (int*)&abits) < 0)
    err("pgaccess failed");
    
  // 1ULL ensures the compiler treats the shift as a 64-bit integer operation
  if (abits != (1ULL | (1ULL << 63)))
    err("failed boundary check on 64th page");
    
  free(buf);
  printf("pgaccess_boundary_test: OK\n");
}