#include <stdio.h>
#include <stdlib.h>
#include "glk.h"
#include "glkunit.h"

extern struct TestDescription tests[];

void
glk_main(void)
{
    struct TestDescription *test = tests;
    int tested = 0, succeeded = 0, failed = 0;
    while(test->name != NULL) {
        tested++;
        /* Use stdio.h to print to stdout, Glk can't do that */
        printf("  Testing %s... ", test->name);
        if( test->testfunc() ) {
            succeeded++;
            printf("PASS\n");
        } else {
            failed++;
            printf("FAIL\n");
        }
        test++;
    }
    printf("%d tests run, %d failures\n", tested, failed);
    exit(failed > 0);
}
