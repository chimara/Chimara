#include <stdio.h>
#include <stdlib.h>
#include "glk.h"
#include "glkunit.h"

extern struct TestDescription tests[];

void
glk_main(void)
{
    struct TestDescription *test = tests;
    int total, tested, failed = 0;

    /* Print test plan. Use stdio.h to print to stdout, Glk can't do that */
    for(total = 0; tests[total].name != NULL; total++)
        ;  /* count tests for test plan*/
    printf("1..%d\n", total);

    for(tested = 0; tested < total; tested++, test++) {
        if( !test->testfunc() ) {
            printf("not ");
            failed++;
        }
        printf("ok %d %s\n", tested + 1, test->name);
    }
    /* Not supposed to use exit() in Glk, but we don't want to pause until the
    window is closed */
    exit(0);
}
