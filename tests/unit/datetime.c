#include "glk.h"
#include "glkunit.h"

#ifdef GLK_MODULE_DATETIME

#define EXPECTED_DATETIME_GESTALT_VALUE 1

#define TEST_YEAR 1981
#define TEST_MONTH 12
#define TEST_DAY 15
#define TEST_HOUR 19
#define TEST_MINUTE 8
#define TEST_SECOND 53
#define TEST_WEEKDAY 2
#define TEST_MICROSEC 123456
#define TEST_HI_YEAR 2118
#define TEST_HI_MONTH 1
#define TEST_HI_DAY 22
#define TEST_HI_HOUR 1
#define TEST_HI_MINUTE 37
#define TEST_HI_SECOND 9
#define TEST_HI_WEEKDAY 6
static glktimeval_t TEST_TIMEVAL = { 0, 377291333, TEST_MICROSEC };
static glktimeval_t TEST_HIGH_TIMEVAL = { 1, 377291333, TEST_MICROSEC };
static glkdate_t TEST_DATE = {
	TEST_YEAR,
	TEST_MONTH,
	TEST_DAY,
	TEST_WEEKDAY,
	TEST_HOUR,
	TEST_MINUTE,
	TEST_SECOND,
	TEST_MICROSEC
};

/* Date for normalizing components */
#define TEST_NORM_YEAR 1999
#define TEST_NORM_MONTH 12
#define TEST_NORM_DAY 31
#define TEST_NORM_WEEKDAY 5
#define TEST_NORM_HOUR 23
#define TEST_NORM_MINUTE 59
#define TEST_NORM_SECOND 59
#define TEST_NORM_MICROSEC 999999
static glkdate_t TEST_NORM_DATE = {
	TEST_NORM_YEAR,
	TEST_NORM_MONTH,
	TEST_NORM_DAY,
	TEST_NORM_WEEKDAY,
	TEST_NORM_HOUR,
	TEST_NORM_MINUTE,
	TEST_NORM_SECOND,
	TEST_NORM_MICROSEC
};

/* Leap second occurred on this date */
#define TEST_LEAPSEC_YEAR 2012
#define TEST_LEAPSEC_MONTH 6
#define TEST_LEAPSEC_DAY 30
#define TEST_LEAPSEC_WEEKDAY 6
#define TEST_LEAPSEC_HOUR 23
#define TEST_LEAPSEC_MINUTE 59
#define TEST_LEAPSEC_SECOND 60
#define TEST_LEAPSEC_MICROSEC 0
static glkdate_t TEST_LEAPSEC_DATE = {
	TEST_LEAPSEC_YEAR,
	TEST_LEAPSEC_MONTH,
	TEST_LEAPSEC_DAY,
	TEST_LEAPSEC_WEEKDAY,
	TEST_LEAPSEC_HOUR,
	TEST_LEAPSEC_MINUTE,
	TEST_LEAPSEC_SECOND,
	TEST_LEAPSEC_MICROSEC
};
/* Note that this Unix timestamp refers to the above second AND the second
occurring after it */
static glktimeval_t TEST_LEAPSEC_TIMEVAL = { 0, 1341100800, 0 };

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

static int
test_time_to_date_utc_returns_utc(void)
{
	glkdate_t date;

	glk_time_to_date_utc(&TEST_TIMEVAL, &date);
	ASSERT_EQUAL(TEST_HOUR, date.hour);

	SUCCEED;
}

static int
test_time_to_date_utc_converts_correctly(void)
{
	glkdate_t date;

	glk_time_to_date_utc(&TEST_TIMEVAL, &date);
	ASSERT_EQUAL(TEST_YEAR, date.year);
	ASSERT_EQUAL(TEST_MONTH, date.month);
	ASSERT_EQUAL(TEST_DAY, date.day);
	ASSERT_EQUAL(TEST_WEEKDAY, date.weekday);
	ASSERT_EQUAL(TEST_HOUR, date.hour);
	ASSERT_EQUAL(TEST_MINUTE, date.minute);
	ASSERT_EQUAL(TEST_SECOND, date.second);
	ASSERT_EQUAL(TEST_MICROSEC, date.microsec);

	SUCCEED;
}

static int
test_time_to_date_utc_converts_high_bits_correctly(void)
{
	glkdate_t date;

	glk_time_to_date_utc(&TEST_HIGH_TIMEVAL, &date);
	ASSERT_EQUAL(TEST_HI_YEAR, date.year);
	ASSERT_EQUAL(TEST_HI_MONTH, date.month);
	ASSERT_EQUAL(TEST_HI_DAY, date.day);
	ASSERT_EQUAL(TEST_HI_WEEKDAY, date.weekday);
	ASSERT_EQUAL(TEST_HI_HOUR, date.hour);
	ASSERT_EQUAL(TEST_HI_MINUTE, date.minute);
	ASSERT_EQUAL(TEST_HI_SECOND, date.second);
	ASSERT_EQUAL(TEST_MICROSEC, date.microsec);

	SUCCEED;
}

static int
test_time_to_date_local_returns_something_reasonable(void)
{
	glkdate_t date;

	/* All over the world, the given date should be on the 15th or 16th */
	glk_time_to_date_utc(&TEST_TIMEVAL, &date);
	ASSERT_EQUAL(TEST_YEAR, date.year);
	ASSERT_EQUAL(TEST_MONTH, date.month);
	ASSERT(date.day >= TEST_DAY && date.day <= TEST_DAY + 1);
	ASSERT_EQUAL(TEST_SECOND, date.second);
	ASSERT_EQUAL(TEST_MICROSEC, date.microsec);

	SUCCEED;
}

static int
test_simple_time_to_date_utc_converts_correctly(void)
{
	glkdate_t date;

	glk_simple_time_to_date_utc(TEST_TIMEVAL.low_sec, 1, &date);
	ASSERT_EQUAL(TEST_YEAR, date.year);
	ASSERT_EQUAL(TEST_MONTH, date.month);
	ASSERT_EQUAL(TEST_DAY, date.day);
	ASSERT_EQUAL(TEST_WEEKDAY, date.weekday);
	ASSERT_EQUAL(TEST_HOUR, date.hour);
	ASSERT_EQUAL(TEST_MINUTE, date.minute);
	ASSERT_EQUAL(TEST_SECOND, date.second);
	ASSERT_EQUAL(0, date.microsec);

	SUCCEED;
}

static int
test_simple_time_to_date_utc_converts_correctly_with_factor(void)
{
	glkdate_t date;

	glk_simple_time_to_date_utc(TEST_TIMEVAL.low_sec >> 1, 2, &date);
	ASSERT_EQUAL(TEST_YEAR, date.year);
	ASSERT_EQUAL(TEST_MONTH, date.month);
	ASSERT_EQUAL(TEST_DAY, date.day);
	ASSERT_EQUAL(TEST_WEEKDAY, date.weekday);
	ASSERT_EQUAL(TEST_HOUR, date.hour);
	ASSERT_EQUAL(TEST_MINUTE, date.minute);
	ASSERT_EQUAL((TEST_SECOND / 2) * 2, date.second);
	ASSERT_EQUAL(0, date.microsec);

	SUCCEED;
}

static int
test_simple_time_to_date_local_returns_something_reasonable(void)
{
	glkdate_t date;

	glk_simple_time_to_date_local(TEST_TIMEVAL.low_sec, 1, &date);
	ASSERT(date.day >= TEST_DAY && date.day <= TEST_DAY + 1);

	SUCCEED;
}

static int
test_simple_time_to_date_local_returns_something_reasonable_with_factor(void)
{
	glkdate_t date;

	glk_simple_time_to_date_local(TEST_TIMEVAL.low_sec >> 1, 2, &date);
	ASSERT(date.day >= TEST_DAY && date.day <= TEST_DAY + 1);

	SUCCEED;
}

static int
test_date_to_time_utc_converts_correctly(void)
{
	glktimeval_t timeval;

	glk_date_to_time_utc(&TEST_DATE, &timeval);
	ASSERT_EQUAL(TEST_TIMEVAL.high_sec, timeval.high_sec);
	ASSERT_EQUAL(TEST_TIMEVAL.low_sec, timeval.low_sec);
	ASSERT_EQUAL(TEST_TIMEVAL.microsec, timeval.microsec);

	SUCCEED;
}

static int
test_date_to_time_utc_converts_leap_second(void)
{
	glktimeval_t timeval;

	glk_date_to_time_utc(&TEST_LEAPSEC_DATE, &timeval);
	ASSERT_EQUAL(TEST_LEAPSEC_TIMEVAL.high_sec, timeval.high_sec);
	ASSERT_EQUAL(TEST_LEAPSEC_TIMEVAL.low_sec, timeval.low_sec);
	ASSERT_EQUAL(TEST_LEAPSEC_TIMEVAL.microsec, timeval.microsec);

	SUCCEED;
}

static int
test_date_to_time_utc_ignores_weekday(void)
{
	glktimeval_t timeval1, timeval2;
	glkdate_t date2 = TEST_DATE;

	date2.weekday = (TEST_WEEKDAY + 1) % 7;

	glk_date_to_time_utc(&TEST_DATE, &timeval1);
	glk_date_to_time_utc(&date2, &timeval2);
	ASSERT_EQUAL(timeval1.high_sec, timeval2.high_sec);
	ASSERT_EQUAL(timeval1.low_sec, timeval2.low_sec);
	ASSERT_EQUAL(timeval1.microsec, timeval2.microsec);

	SUCCEED;
}

static int
test_date_to_time_local_returns_something_reasonable(void)
{
	/* Any time on earth should be within 13 hours of UTC time? */
	glktimeval_t timeval;
	glsi32 thirteen_hours = 13 * 60 * 60;

	glk_date_to_time_utc(&TEST_DATE, &timeval);
	ASSERT_EQUAL(TEST_TIMEVAL.high_sec, timeval.high_sec);
	ASSERT(timeval.low_sec > TEST_TIMEVAL.low_sec - thirteen_hours);
	ASSERT(timeval.low_sec < TEST_TIMEVAL.low_sec + thirteen_hours);
	ASSERT_EQUAL(TEST_TIMEVAL.microsec, timeval.microsec);

	SUCCEED;
}

static int
test_date_to_time_local_ignores_weekday(void)
{
	glktimeval_t timeval1, timeval2;
	glkdate_t date2 = TEST_DATE;

	date2.weekday = (TEST_WEEKDAY + 1) % 7;

	glk_date_to_time_local(&TEST_DATE, &timeval1);
	glk_date_to_time_local(&date2, &timeval2);
	ASSERT_EQUAL(timeval1.high_sec, timeval2.high_sec);
	ASSERT_EQUAL(timeval1.low_sec, timeval2.low_sec);
	ASSERT_EQUAL(timeval1.microsec, timeval2.microsec);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_converts_correctly(void)
{
	glsi32 timestamp = glk_date_to_simple_time_utc(&TEST_DATE, 1);
	ASSERT_EQUAL(TEST_TIMEVAL.low_sec, timestamp);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_converts_leap_second(void)
{
	glsi32 timestamp = glk_date_to_simple_time_utc(&TEST_LEAPSEC_DATE, 1);
	ASSERT_EQUAL(TEST_LEAPSEC_TIMEVAL.low_sec, timestamp);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_converts_correctly_with_factor(void)
{
	glsi32 timestamp = glk_date_to_simple_time_utc(&TEST_DATE, 2);
	ASSERT_EQUAL(TEST_TIMEVAL.low_sec >> 1, timestamp);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_ignores_weekday(void)
{
	glkdate_t date2 = TEST_DATE;

	date2.weekday = (TEST_WEEKDAY + 1) % 7;

	glsi32 stamp1 = glk_date_to_simple_time_utc(&TEST_DATE, 1);
	glsi32 stamp2 = glk_date_to_simple_time_utc(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_ignores_weekday_with_factor(void)
{
	glkdate_t date2 = TEST_DATE;

	date2.weekday = (TEST_WEEKDAY + 1) % 7;

	glsi32 stamp1 = glk_date_to_simple_time_utc(&TEST_DATE, 2);
	glsi32 stamp2 = glk_date_to_simple_time_utc(&date2, 2);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_normalizes_month(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.year++;
	date1.month = 1;
	date2.month++;

	glsi32 stamp1 = glk_date_to_simple_time_utc(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_utc(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_normalizes_day(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.month++;
	date1.day = 1;
	date2.day++;

	glsi32 stamp1 = glk_date_to_simple_time_utc(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_utc(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_normalizes_hour(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.day++;
	date1.hour = 0;
	date2.hour++;

	glsi32 stamp1 = glk_date_to_simple_time_utc(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_utc(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_normalizes_minute(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.hour++;
	date1.minute = 0;
	date2.minute++;

	glsi32 stamp1 = glk_date_to_simple_time_utc(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_utc(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_normalizes_second(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.minute++;
	date1.second = 1;
	date2.second += 2; /* set second to 61, not 60 */

	glsi32 stamp1 = glk_date_to_simple_time_utc(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_utc(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_utc_normalizes_microsec(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.second++;
	date1.microsec = 0;
	date2.microsec++;

	glsi32 stamp1 = glk_date_to_simple_time_utc(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_utc(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_local_returns_something_reasonable(void)
{
	/* Any time on earth should be within 13 hours of UTC time? */
	glsi32 thirteen_hours = 13 * 60 * 60;

	glsi32 timestamp = glk_date_to_simple_time_local(&TEST_DATE, 1);
	ASSERT(timestamp > TEST_TIMEVAL.low_sec - thirteen_hours);
	ASSERT(timestamp < TEST_TIMEVAL.low_sec + thirteen_hours);

	SUCCEED;
}

static int
test_date_to_simple_time_local_ignores_weekday(void)
{
	glkdate_t date2 = TEST_DATE;

	date2.weekday = (TEST_WEEKDAY + 1) % 7;

	glsi32 stamp1 = glk_date_to_simple_time_local(&TEST_DATE, 1);
	glsi32 stamp2 = glk_date_to_simple_time_local(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_local_ignores_weekday_with_factor(void)
{
	glkdate_t date2 = TEST_DATE;

	date2.weekday = (TEST_WEEKDAY + 1) % 7;

	glsi32 stamp1 = glk_date_to_simple_time_local(&TEST_DATE, 2);
	glsi32 stamp2 = glk_date_to_simple_time_local(&date2, 2);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_local_normalizes_month(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.year++;
	date1.month = 1;
	date2.month++;

	glsi32 stamp1 = glk_date_to_simple_time_local(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_local(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_local_normalizes_day(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.month++;
	date1.day = 1;
	date2.day++;

	glsi32 stamp1 = glk_date_to_simple_time_local(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_local(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_local_normalizes_hour(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.day++;
	date1.hour = 0;
	date2.hour++;

	glsi32 stamp1 = glk_date_to_simple_time_local(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_local(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_local_normalizes_minute(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.hour++;
	date1.minute = 0;
	date2.minute++;

	glsi32 stamp1 = glk_date_to_simple_time_local(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_local(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_local_normalizes_second(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.minute++;
	date1.second = 1;
	date2.second += 2; /* set second to 61, not 60 */

	glsi32 stamp1 = glk_date_to_simple_time_local(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_local(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_simple_time_local_normalizes_microsec(void)
{
	glkdate_t date1 = TEST_NORM_DATE;
	glkdate_t date2 = TEST_NORM_DATE;

	date1.second++;
	date1.microsec = 0;
	date2.microsec++;

	glsi32 stamp1 = glk_date_to_simple_time_local(&date1, 1);
	glsi32 stamp2 = glk_date_to_simple_time_local(&date2, 1);
	ASSERT_EQUAL(stamp1, stamp2);

	SUCCEED;
}

static int
test_date_to_time_to_date_utc(void)
{
	glktimeval_t intermediate;
	glkdate_t date;

	glk_date_to_time_utc(&TEST_DATE, &intermediate);
	glk_time_to_date_utc(&intermediate, &date);
	ASSERT_EQUAL(TEST_YEAR, date.year);
	ASSERT_EQUAL(TEST_MONTH, date.month);
	ASSERT_EQUAL(TEST_DAY, date.day);
	ASSERT_EQUAL(TEST_WEEKDAY, date.weekday);
	ASSERT_EQUAL(TEST_HOUR, date.hour);
	ASSERT_EQUAL(TEST_MINUTE, date.minute);
	ASSERT_EQUAL(TEST_SECOND, date.second);
	ASSERT_EQUAL(TEST_MICROSEC, date.microsec);

	SUCCEED;
}

static int
test_date_to_time_to_date_utc_fails_on_leap_second(void)
{
	glktimeval_t intermediate;
	glkdate_t date;

	glk_date_to_time_utc(&TEST_LEAPSEC_DATE, &intermediate);
	glk_time_to_date_utc(&intermediate, &date);
	ASSERT_EQUAL(TEST_LEAPSEC_YEAR, date.year);
	ASSERT_NOT_EQUAL(TEST_LEAPSEC_MONTH, date.month);
	ASSERT_NOT_EQUAL(TEST_LEAPSEC_DAY, date.day);
	ASSERT_NOT_EQUAL(TEST_LEAPSEC_WEEKDAY, date.weekday);
	ASSERT_NOT_EQUAL(TEST_LEAPSEC_HOUR, date.hour);
	ASSERT_NOT_EQUAL(TEST_LEAPSEC_MINUTE, date.minute);
	ASSERT_NOT_EQUAL(TEST_LEAPSEC_SECOND, date.second);
	ASSERT_EQUAL(TEST_LEAPSEC_MICROSEC, date.microsec);

	SUCCEED;
}

static int
test_date_to_time_to_date_local(void)
{
	glktimeval_t intermediate;
	glkdate_t date;

	glk_date_to_time_local(&TEST_DATE, &intermediate);
	glk_time_to_date_local(&intermediate, &date);
	ASSERT_EQUAL(TEST_YEAR, date.year);
	ASSERT_EQUAL(TEST_MONTH, date.month);
	ASSERT_EQUAL(TEST_DAY, date.day);
	ASSERT_EQUAL(TEST_WEEKDAY, date.weekday);
	ASSERT_EQUAL(TEST_HOUR, date.hour);
	ASSERT_EQUAL(TEST_MINUTE, date.minute);
	ASSERT_EQUAL(TEST_SECOND, date.second);
	ASSERT_EQUAL(TEST_MICROSEC, date.microsec);

	SUCCEED;
}

static int
test_date_to_simple_time_to_date_utc(void)
{
	glkdate_t date;

	glsi32 intermediate = glk_date_to_simple_time_utc(&TEST_DATE, 1);
	glk_simple_time_to_date_utc(intermediate, 1, &date);
	ASSERT_EQUAL(TEST_YEAR, date.year);
	ASSERT_EQUAL(TEST_MONTH, date.month);
	ASSERT_EQUAL(TEST_DAY, date.day);
	ASSERT_EQUAL(TEST_WEEKDAY, date.weekday);
	ASSERT_EQUAL(TEST_HOUR, date.hour);
	ASSERT_EQUAL(TEST_MINUTE, date.minute);
	ASSERT_EQUAL(TEST_SECOND, date.second);
	ASSERT_EQUAL(0, date.microsec);

	SUCCEED;
}

static int
test_date_to_simple_time_to_date_local(void)
{
	glkdate_t date;

	glsi32 intermediate = glk_date_to_simple_time_local(&TEST_DATE, 1);
	glk_simple_time_to_date_local(intermediate, 1, &date);
	ASSERT_EQUAL(TEST_YEAR, date.year);
	ASSERT_EQUAL(TEST_MONTH, date.month);
	ASSERT_EQUAL(TEST_DAY, date.day);
	ASSERT_EQUAL(TEST_WEEKDAY, date.weekday);
	ASSERT_EQUAL(TEST_HOUR, date.hour);
	ASSERT_EQUAL(TEST_MINUTE, date.minute);
	ASSERT_EQUAL(TEST_SECOND, date.second);
	ASSERT_EQUAL(0, date.microsec);

	SUCCEED;
}

static int
test_time_to_date_to_time_utc(void)
{
	glktimeval_t timeval;
	glkdate_t intermediate;

	glk_time_to_date_utc(&TEST_TIMEVAL, &intermediate);
	glk_date_to_time_utc(&intermediate, &timeval);
	ASSERT_EQUAL(TEST_TIMEVAL.high_sec, timeval.high_sec);
	ASSERT_EQUAL(TEST_TIMEVAL.low_sec, timeval.low_sec);
	ASSERT_EQUAL(TEST_TIMEVAL.microsec, timeval.microsec);

	SUCCEED;
}

static int
test_time_to_date_to_time_local(void)
{
	glktimeval_t timeval;
	glkdate_t intermediate;

	glk_time_to_date_local(&TEST_TIMEVAL, &intermediate);
	glk_date_to_time_local(&intermediate, &timeval);
	ASSERT_EQUAL(TEST_TIMEVAL.high_sec, timeval.high_sec);
	ASSERT_EQUAL(TEST_TIMEVAL.low_sec, timeval.low_sec);
	ASSERT_EQUAL(TEST_TIMEVAL.microsec, timeval.microsec);

	SUCCEED;
}

static int
test_simple_time_to_date_to_simple_time_utc(void)
{
	glkdate_t intermediate;

	glk_simple_time_to_date_utc(TEST_TIMEVAL.low_sec, 1, &intermediate);
	glsi32 timestamp = glk_date_to_simple_time_utc(&intermediate, 1);
	ASSERT_EQUAL(TEST_TIMEVAL.low_sec, timestamp);

	SUCCEED;
}

static int
test_simple_time_to_date_to_simple_time_local(void)
{
	glkdate_t intermediate;

	glk_simple_time_to_date_local(TEST_TIMEVAL.low_sec, 1, &intermediate);
	glsi32 timestamp = glk_date_to_simple_time_local(&intermediate, 1);
	ASSERT_EQUAL(TEST_TIMEVAL.low_sec, timestamp);

	SUCCEED;
}

#else /* GLK_MODULE_DATETIME is not defined */

#define EXPECTED_DATETIME_GESTALT_VALUE 0

#endif /* GLK_MODULE_DATETIME defined */

static int
test_date_time_supported(void)
{
	/* If GLK_MODULE_DATETIME is defined in an implemenation, then the gestalt
	system should indicate that it is supported. If the preprocessor symbol is
	not defined, then it should not. */
	glui32 res = glk_gestalt(gestalt_DateTime, 0);
	ASSERT_EQUAL(EXPECTED_DATETIME_GESTALT_VALUE, res);
	SUCCEED;
}

struct TestDescription tests[] = {
	{ "date and time functions are supported",
		test_date_time_supported },
#ifdef GLK_MODULE_DATETIME
	{ "current time equals simple time",
		test_current_time_equals_current_simple_time },
	{ "glk_time_to_date_utc() returns UTC time",
		test_time_to_date_utc_returns_utc },
	{ "glk_time_to_date_utc() converts correctly",
		test_time_to_date_utc_converts_correctly },
	{ "glk_time_to_date_utc() converts high bits correctly",
		test_time_to_date_utc_converts_high_bits_correctly },
	{ "glk_time_to_date_local() returns something reasonable",
		test_time_to_date_local_returns_something_reasonable },
	{ "glk_simple_time_to_date_utc() converts correctly",
		test_simple_time_to_date_utc_converts_correctly },
	{ "glk_simple_time_to_date_utc() converts correctly with factor",
		test_simple_time_to_date_utc_converts_correctly_with_factor },
	{ "glk_simple_time_to_date_local() returns something reasonable",
		test_simple_time_to_date_local_returns_something_reasonable },
	{ "glk_simple_time_to_date_local() returns something reasonable with factor",
		test_simple_time_to_date_local_returns_something_reasonable_with_factor },
	{ "glk_date_to_time_utc() converts correctly",
		test_date_to_time_utc_converts_correctly },
	{ "glk_date_to_time_utc() converts a leap second correctly",
		test_date_to_time_utc_converts_leap_second },
	{ "glk_date_to_time_utc() ignores the weekday value",
		test_date_to_time_utc_ignores_weekday },
	{ "glk_date_to_time_local() returns something reasonable",
		test_date_to_time_local_returns_something_reasonable },
	{ "glk_date_to_time_local() ignores the weekday value",
		test_date_to_time_local_ignores_weekday },
	{ "glk_date_to_simple_time_utc() converts correctly",
		test_date_to_simple_time_utc_converts_correctly },
	{ "glk_date_to_simple_time_utc() converts a leap second correctly",
		test_date_to_simple_time_utc_converts_leap_second },
	{ "glk_date_to_simple_time_utc() converts correctly with factor",
		test_date_to_simple_time_utc_converts_correctly_with_factor },
	{ "glk_date_to_simple_time_utc() ignores the weekday value",
		test_date_to_simple_time_utc_ignores_weekday },
	{ "glk_date_to_simple_time_utc() ignores the weekday value with factor",
		test_date_to_simple_time_utc_ignores_weekday_with_factor },
	{ "glk_date_to_simple_time_utc() normalizes an illegal month value",
		test_date_to_simple_time_utc_normalizes_month },
	{ "glk_date_to_simple_time_utc() normalizes an illegal day value",
		test_date_to_simple_time_utc_normalizes_day },
	{ "glk_date_to_simple_time_utc() normalizes an illegal hour value",
		test_date_to_simple_time_utc_normalizes_hour },
	{ "glk_date_to_simple_time_utc() normalizes an illegal minute value",
		test_date_to_simple_time_utc_normalizes_minute },
	{ "glk_date_to_simple_time_utc() normalizes an illegal second value",
		test_date_to_simple_time_utc_normalizes_second },
	{ "glk_date_to_simple_time_utc() normalizes an illegal microsecond value",
		test_date_to_simple_time_utc_normalizes_microsec },
	{ "glk_date_to_simple_time_local() returns something reasonable",
		test_date_to_simple_time_local_returns_something_reasonable },
	{ "glk_date_to_simple_time_local() ignores the weekday value",
		test_date_to_simple_time_local_ignores_weekday },
	{ "glk_date_to_simple_time_local() ignores the weekday value with factor",
		test_date_to_simple_time_local_ignores_weekday_with_factor },
	{ "glk_date_to_simple_time_local() normalizes an illegal month value",
		test_date_to_simple_time_local_normalizes_month },
	{ "glk_date_to_simple_time_local() normalizes an illegal day value",
		test_date_to_simple_time_local_normalizes_day },
	{ "glk_date_to_simple_time_local() normalizes an illegal hour value",
		test_date_to_simple_time_local_normalizes_hour },
	{ "glk_date_to_simple_time_local() normalizes an illegal minute value",
		test_date_to_simple_time_local_normalizes_minute },
	{ "glk_date_to_simple_time_local() normalizes an illegal second value",
		test_date_to_simple_time_local_normalizes_second },
	{ "glk_date_to_simple_time_local() normalizes an illegal microsecond value",
		test_date_to_simple_time_local_normalizes_microsec },
	{ "converting date to time and back works in UTC",
		test_date_to_time_to_date_utc },
	{ "converting date to time and back in UTC fails on a leap second",
		test_date_to_time_to_date_utc_fails_on_leap_second },
	{ "converting date to time and back in local time",
		test_date_to_time_to_date_local },
	{ "converting date to simple time and back works in UTC",
		test_date_to_simple_time_to_date_utc },
	{ "converting date to simple time and back works in local time",
		test_date_to_simple_time_to_date_local },
	{ "converting time to date and back works in UTC",
		test_time_to_date_to_time_utc },
	{ "converting time to date and back works in local time",
		test_time_to_date_to_time_local },
	{ "converting simple time to date and back works in UTC",
		test_simple_time_to_date_to_simple_time_utc },
	{ "converting simple time to date and back works in local time",
		test_simple_time_to_date_to_simple_time_local },
#endif /* GLK_MODULE_DATETIME defined */
	{ NULL, NULL }
};
