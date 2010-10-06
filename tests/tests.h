#ifndef __test_include__
#define __test_include__

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include "learner.h"


int test_vector();
int test_sparse_vector();

#define print_separator()       printf("\n=================================================\n");
#define test(expr)              if(expr){printf("+\t%s\n", #expr); passed++;} else {printf("-\t%s\n\t(%s:%u)\n", #expr, __FILE__, __LINE__); failed++;}
#define test_error(err)         if(err){printf("-\tUnexpected error (%s)\n\t(%s:%u)\n", learner_error_codes[err], __FILE__, __LINE__); failed++;}
#define test_float(expr, val)   test(fabs(expr - val) <= FLT_EPSILON);
#define starting_tests()        int passed = 0, failed = 0; printf("Starting Tests From: %s\n", __FILE__);
#define finished_tests()        printf("\nPassed: %i, failed: %i\n\n", passed, failed); return failed;
#define run_test(test_function) print_separator(); failed += test_function();

#endif
