#include <glib.h>
#include "glk.h"

/**
 * glk_current_time:
 * @time: pointer to a #glktimeval_t structure.
 *
 * The current Unix time is stored in the structure @time. (The argument may not
 * be %NULL.) This is the number of seconds since the beginning of 1970 (UTC).
 *
 * The first two values in the structure should be considered a single
 * <emphasis>signed</emphasis> 64-bit number. This allows the #glktimeval_t to
 * store a reasonable range of values in the future and past. The @high_sec
 * value will remain zero until sometime in 2106. If your computer is running in
 * 1969, perhaps due to an unexpected solar flare, then @high_sec will be
 * negative.
 * 
 * The third value in the structure represents a fraction of a second, in
 * microseconds (from 0 to 999999). The resolution of the glk_current_time()
 * call is platform-dependent; the @microsec value may not be updated
 * continuously.
 */
void 
glk_current_time(glktimeval_t *time)
{
	g_return_if_fail(time != NULL);
}

/**
 * glk_current_simple_time:
 * @factor: Factor by which to divide the time value.
 *
 * If dealing with 64-bit values is awkward, you can also get the current time
 * as a lower-resolution 32-bit value. This is simply the Unix time divided by
 * the @factor argument (which must not be zero). For example, if factor is 60,
 * the result will be the number of minutes since 1970 (rounded towards negative
 * infinity). If factor is 1, you will get the Unix time directly, but the value
 * will be truncated starting some time in 2038.
 *
 * Returns: Unix time divided by @factor, truncated to 32 bits.
 */
glsi32
glk_current_simple_time(glui32 factor)
{
	g_return_val_if_fail(factor != 0, -1);
	
	return -1;
}

/**
 * glk_time_to_date_utc:
 * @time: A #glktimeval_t structure as returned by glk_current_time().
 * @date: An empty #glkdate_t structure to fill in.
 *
 * Convert the given timestamp (as returned by glk_current_time()) to a
 * broken-out structure. This function returns a date and time in universal time
 * (GMT).
 *
 * <note><para>
 *   The seconds value may be 60 because of a leap second.
 * </para></note>
 */
void
glk_time_to_date_utc(glktimeval_t *time, glkdate_t *date)
{
	g_return_if_fail(time != NULL);
	g_return_if_fail(date != NULL);
}

/**
 * glk_time_to_date_local:
 * @time: A #glktimeval_t structure as returned by glk_current_time().
 * @date: An empty #glkdate_t structure to fill in.
 *
 * Does the same thing as glk_time_to_date_utc(), but this function returns
 * local time.
 */
void
glk_time_to_date_local(glktimeval_t *time, glkdate_t *date)
{
	g_return_if_fail(time != NULL);
	g_return_if_fail(date != NULL);
}

/**
 * glk_simple_time_to_date_utc:
 * @time: Timestamp as returned by glk_current_simple_time().
 * @factor: Factor by which to multiply @time in order to get seconds.
 * @date: An empty #glkdate_t structure to fill in.
 *
 * Convert the given timestamp (as returned by glk_current_simple_time()) to a
 * broken-out structure in universal time. The @time argument is multiplied by
 * @factor to produce a Unix timestamp.
 *
 * Since the resolution of glk_simple_time_to_date_utc() and
 * glk_simple_time_to_date_local() is no better than seconds, they will return
 * zero for the microseconds value. 
 */ 
void
glk_simple_time_to_date_utc(glsi32 time, glui32 factor, glkdate_t *date)
{
	g_return_if_fail(factor != 0);
	g_return_if_fail(date != NULL);
}

/**
 * glk_simple_time_to_date_local:
 * @time: Timestamp as returned by glk_current_simple_time().
 * @factor: Factor by which to multiply @time in order to get seconds.
 * @date: An empty #glkdate_t structure to fill in.
 *
 * Does the same thing as glk_simple_time_to_date_utc(), but fills in the @date
 * structure in local time.
 */
void
glk_simple_time_to_date_local(glsi32 time, glui32 factor, glkdate_t *date)
{
	g_return_if_fail(factor != 0);
	g_return_if_fail(date != NULL);
}

/**
 * glk_date_to_time_utc:
 * @date: A date in the form of a #glkdate_t structure.
 * @time: An empty #glktimeval_t structure to fill in.
 *
 * Convert the broken-out structure (interpreted as universal time) to a
 * timestamp. The weekday value in @date is ignored. The other values need not
 * be in their normal ranges; they will be normalized.
 *
 * If the time cannot be represented by the platform's time library, this may
 * return -1 for the seconds value. (I.e., the @high_sec and @low_sec fields
 * both $FFFFFFFF. The microseconds field is undefined in this case.)
 */
void
glk_date_to_time_utc(glkdate_t *date, glktimeval_t *time)
{
	g_return_if_fail(date != NULL);
	g_return_if_fail(time != NULL);
}

/**
 * glk_date_to_time_local:
 * @date: A date in the form of a #glkdate_t structure.
 * @time: An empty #glktimeval_t structure to fill in.
 *
 * Does the same thing as glk_date_to_time_utc(), but interprets the broken-out
 * structure as local time.
 *
 * The glk_date_to_time_local() function may not be smart about Daylight Saving
 * Time conversions.
 * <note><para>
 *   If implemented with the mktime() libc function, it should use the negative
 *   @tm_isdst flag to <quote>attempt to divine whether summer time is in
 *   effect</quote>.
 * </para></note>
 */
void
glk_date_to_time_local(glkdate_t *date, glktimeval_t *time)
{
	g_return_if_fail(date != NULL);
	g_return_if_fail(time != NULL);
}

/**
 * glk_date_to_simple_time_utc:
 * @date: A date in the form of a #glkdate_t structure.
 * @factor: Factor by which to divide the time value.
 *
 * Convert the broken-out structure (interpreted as universal time) to a
 * timestamp divided by @factor. The weekday value in @date is ignored. The
 * other values need not be in their normal ranges; they will be normalized.
 *
 * If the time cannot be represented by the platform's time library, this may
 * return -1.
 *
 * Returns: a timestamp divided by @factor, and truncated to 32 bits, or -1 on
 * error.
 */
glsi32
glk_date_to_simple_time_utc(glkdate_t *date, glui32 factor)
{
	g_return_val_if_fail(date != NULL, -1);
	g_return_val_if_fail(factor != 0, -1);
	
	return -1;
}

/**
 * glk_date_to_simple_time_local:
 * @date: A date in the form of a #glkdate_t structure.
 * @factor: Factor by which to divide the time value.
 *
 * Does the same thing as glk_date_to_simple_time_utc(), but interprets the
 * broken-out structure as local time.
 *
 * Returns: a timestamp divided by @factor, and truncated to 32 bits, or -1 on
 * error.
 */
glsi32
glk_date_to_simple_time_local(glkdate_t *date, glui32 factor)
{
	g_return_val_if_fail(date != NULL, -1);
	g_return_val_if_fail(factor != 0, -1);
	
	return -1;
}
