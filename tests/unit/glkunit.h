#ifndef GLKUNIT_H
#define GLKUNIT_H

#include <stdio.h>

#define _BEGIN do {
#define _END } while(0);

/* msg must be a string literal */
#define _ASSERT(expr, msg, ...) _BEGIN \
    if( !(expr) ) { \
        fprintf(stderr, "Assertion failed: " msg "\n", __VA_ARGS__); \
        return 0; \
    } _END

#define SUCCEED _BEGIN return 1; _END
#define ASSERT(expr) _ASSERT(expr, "%s", #expr)
/* This macro is meant for int-like things that can print with %d */
#define ASSERT_EQUAL(expected, actual) _ASSERT((expected) == (actual), \
    "%s == %s (expected %d, was %d)", \
    #actual, #expected, expected, actual);
#define ASSERT_NOT_EQUAL(unexpected, actual) _ASSERT((unexpected) != (actual), \
    "%s != %s (expected not to be %d but was)", \
    #actual, #unexpected, unexpected);

struct TestDescription {
    char *name;
    int (*testfunc)(void);
};

#endif /* GLKUNIT_H */
