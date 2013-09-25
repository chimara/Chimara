#include "glk.h"
#include "glkunit.h"

static int
test_current_time_equals_current_simple_time(void)
{
	glktimeval_t timeval;

	/* This test will fail if the following two operations do not occur within
	a second of each other. That is not a robust test, but you could argue that
	there is a serious bug if either operation takes more than one second. */
	glk_current_time(&timeval);
	glsi32 simple_time = glk_current_simple_time(1);
	ASSERT_EQUAL(simple_time, timeval.low_sec);

	glk_current_time(&timeval);
	glsi32 simple_time_high_bits = glk_current_simple_time(0xFFFFFFFF);
	ASSERT_EQUAL(simple_time_high_bits, timeval.high_sec);

	SUCCEED;
}

struct TestDescription tests[] = {
	{ "current time equals simple time", test_current_time_equals_current_simple_time },
	{ NULL, NULL }
};
