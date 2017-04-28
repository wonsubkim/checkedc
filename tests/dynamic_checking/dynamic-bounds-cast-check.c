// The following lines are for the clang automated test suite
//
// RUN: %clang -fcheckedc-extension %s -o %t -Werror
// RUN: %t pass1 | FileCheck %s --check-prefixes=CHECK,CHECK-PASS,CHECK-PASS-1
// RUN: %t pass2 | FileCheck %s --check-prefixes=CHECK,CHECK-PASS,CHECK-PASS-2
// RUN: %t pass3 | FileCheck %s --check-prefixes=CHECK,CHECK-PASS,CHECK-PASS-3
// RUN: %t pass4 | FileCheck %s --check-prefixes=CHECK,CHECK-PASS,CHECK-PASS-4
// RUN: %t fail1 | FileCheck %s --check-prefixes=CHECK,CHECK-FAIL,CHECK-FAIL-1
// RUN: %t fail2 | FileCheck %s --check-prefixes=CHECK,CHECK-FAIL,CHECK-FAIL-2
// RUN: %t fail3 | FileCheck %s --check-prefixes=CHECK,CHECK-FAIL,CHECK-FAIL-3
// RUN: %t fail4 | FileCheck %s --check-prefixes=CHECK,CHECK-FAIL,CHECK-FAIL-4

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/stdchecked.h"

void passing_test_1(void);
void passing_test_2(void);
void passing_test_3(void);
void passing_test_4(void);

void failing_test_1(void);
void failing_test_2(void);
void failing_test_3(void);
void failing_test_4(void);

// Handle an out-of-bounds reference by immediately exiting. This causes
// some output to be missing.
void handle_error(int err) {
  _Exit(0);
}

// This signature for main is exactly what we want here,
// it also means any uses of argv[i] are checked too!
int main(int argc, array_ptr<char*> argv : count(argc)) {

  // Set up the handler for a failing bounds check.  Currently the Checked C
  // clang implementation raises a SIGILL when a bounds check fails.  This
  // may change in the future.
  signal(SIGILL, handle_error);

  // This makes sure output is not buffered for when
  // we hit errors.
  int err = setvbuf(stdout, NULL, _IONBF, 0);
  if (err) {
    // CHECK-NOT: Error Setting Up Buffering
    puts("Error Setting Up Buffering");
    return EXIT_FAILURE;
  }
  
  if (argc < 2) {
    // CHECK-NOT: Requires Argument
    puts("Requires Argument");
    return EXIT_FAILURE;
  }

  int a checked[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  // CHECK: Starting Test
  puts("Starting Test");

  if (strcmp(argv[1], "pass1") == 0) {
    // CHECK-PASS-1: Printable
    // CHECK-PASS-1: Expected Success
    passing_test_1();
  }
  else if (strcmp(argv[1], "pass2") == 0) {
    // CHECK-PASS-2: Printable
    // CHECK-PASS-2: Expected Success
    passing_test_2();
  }
  else if (strcmp(argv[1], "pass3") == 0) {
    // CHECK-PASS-3: Printable
    // CHECK-PASS-3: Expected Success
    passing_test_3();
  }
  else if (strcmp(argv[1], "pass4") == 0) {
    // CHECK-PASS-4: Printable
    // CHECK-PASS-4: Expected Success
    passing_test_4();
  }
  else if (strcmp(argv[1], "fail1") == 0) {
    // CHECK-FAIL-1-NOT: Unprintable
    // CHECK-FAIL-1-NOT: Unexpected Success
    failing_test_1();
  }
  else if (strcmp(argv[1], "fail2") == 0) {
    // CHECK-FAIL-2-NOT: Unprintable
    // CHECK-FAIL-2-NOT: Unexpected Success
    failing_test_2();
  }
  else if (strcmp(argv[1], "fail3") == 0) {
    // CHECK-FAIL-3-NOT: Unprintable
    // CHECK-FAIL-3-NOT: Unexpected Success
    failing_test_3();
  }
  else if (strcmp(argv[1], "fail4") == 0) {
    // CHECK-FAIL-4 : Printable1
    // CHECK-FAIL-4-NOT: Unprintable2
    // CHECK-FAIL-4-NOT: Unexpected Success
    failing_test_4();
  }
  else {
    // CHECK-NOT: Unexpected Test Name
    puts("Unexpected Test Name");
    return EXIT_FAILURE;
  }

  // CHECK-PASS: All Dynamic Checks Passed
  // CHECK-FAIL-NOT: All Dynamic Checks Passed
  puts("All Dynamic Checks Passed");

  return EXIT_SUCCESS;
}

// dynamic_check(r != NULL) && dynamic_check(r <= r && r+3 <= r+10) - > OK
void passing_test_1(void) {
  ptr<int> q = 0;
  int r checked[10] = {0,1,2,3,4,5,6,7,8,9};
  q = _Dynamic_bounds_cast<array_ptr<int>>(r, 3);

  printf("Printable\n");

  puts("Expected Success");
}

// dynamic_check(r != NULL) && dynamic_check(r <= r && r+1 <= r+10) - > OK
void passing_test_2(void) {
  ptr<int> q = 0;
  int r checked[10] = {0,1,2,3,4,5,6,7,8,9};
  q = _Dynamic_bounds_cast<ptr<int>>(r);

  printf("Printable\n");

  puts("Expected Success");
}

// dynamic_check((r+3) != NULL) && dynamic_check(r <= r && r+1 <= r+10) - > OK
void passing_test_3(void) {
  ptr<int> q = 0;
  int r checked[10] = {0,1,2,3,4,5,6,7,8,9};
  q = _Dynamic_bounds_cast<ptr<int>>(r+3);
  printf("Printable\n");

  puts("Expected Success");
}

// dynamic_check((r+3) != NULL) && dynamic_check(r <= r && r+3 <= r+10) - > OK
void passing_test_4(void) {
  ptr<int> q = 0;
  int r checked[10] = {0,1,2,3,4,5,6,7,8,9};
  q = _Dynamic_bounds_cast<array_ptr<int>>(r+3, 3);

  printf("Printable\n");

  puts("Expected Success");
}

// dynamic_check(r != NULL) && dynamic_check(r <= r && r+15 <= r+10) -> FAIL
void failing_test_1(void) {
  ptr<int> q = 0;
  int r checked[10] = {0,1,2,3,4,5,6,7,8,9};
  q = _Dynamic_bounds_cast<array_ptr<int>>(r, 15);
  
  printf("Unprintable\n");
  
  puts("Unexpected Success");
}

// dynamic_check((r+8) != NULL) && dynamic_check(r <= r+8 && (r+8+3) <= r+10) -> FAIL
void failing_test_2(void) {
  ptr<int> q = 0;
  int r checked[10] = {0,1,2,3,4,5,6,7,8,9};
  q = _Dynamic_bounds_cast<array_ptr<int>>(r+8, 3);

  printf("Unprintable\n");
  
  puts("Unexpected Success");
}

// dynamic_check(r != NULL) && dynamic_check(r <= s && s+3 <= r+10) -> FAIL
void failing_test_3(void) {
  ptr<int> q = 0;
  int r checked[10] = {0,1,2,3,4,5,6,7,8,9};
  array_ptr<int> s : count(5) = 0;
  q = _Dynamic_bounds_cast<array_ptr<int>>(r, s, s+3);
  
  printf("Unprintable\n");
  
  puts("Unexpected Success");
}

// bounds_cast insert dynamic_check(r <= r && (r+5) <= r+10) -> OK;
// dereference insert dynamic_check(s <= s+5 && (s+5) < s+3) -> FAIL
void failing_test_4(void) {
  int r checked[10] = {0,1,2,3,4,5,6,7,8,9};
  array_ptr<int> s : count(3) = _Dynamic_bounds_cast<array_ptr<int>>(r, 5);
  
  printf("Printable1\n");
  printf("Unprintable2: %d\n", *(s+5));
  
  puts("Unexpected Success");
}

