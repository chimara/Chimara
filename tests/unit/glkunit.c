#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "glk.h"
#include "gi_blorb.h"
#include "glkstart.h"
#include "glkunit.h"

extern struct TestDescription tests[];

glkunix_argumentlist_t glkunix_arguments[] = {
    { NULL, glkunix_arg_End, NULL },
};

int
glkunix_startup_code(glkunix_startup_t *data)
{
    const char *source_dir = getenv("SOURCE_DIR");
    if (!source_dir)
        source_dir = "tests";
    size_t bufsize = strlen(source_dir) + 30; /* 30 = subpath length + 1 */
    char* path = malloc(bufsize);
    snprintf(path, bufsize, "%s/unit/resource/unittest.blorb", source_dir);

    strid_t stream = glkunix_stream_open_pathname_gen(path, 0 /* readonly */, 0 /* binary */, 0 /* rock */);
    if (!stream) {
        printf("Bail out! Could not load resource file from %s\n", path);
        free(path);
        return FALSE;
    }
    free(path);

    giblorb_set_resource_map(stream);

    return TRUE;
}

void
glk_main(void)
{
    struct TestDescription *test = tests;
    int total, tested, failed = 0;

    printf("TAP version 13\n");

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
    glk_exit();
}
