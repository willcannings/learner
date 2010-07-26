#ifndef __test_include__
#define __test_include__

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "learner.h"


#define test(expr)                if(expr){printf("+ '%s' %s:%u\n", #expr, __FILE__, __LINE__); passed++;} else {printf("- '%s' %s:%u\n", #expr, __FILE__, __LINE__); failed++;}
#define starting_tests()          int passed = 0, failed = 0; printf("Starting Tests From: %s\n", __FILE__);
#define finished_tests()          printf("Finished Tests From: %s\nPassed: %i, failed: %i, total: %i\n\n", __FILE__, passed, failed, passed + failed);
#define run_test(test_function)   printf("\n=================================================\n"); test_function();

#endif
