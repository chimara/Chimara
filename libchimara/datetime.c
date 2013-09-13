#include <glib.h>
#include "glk.h"
#include "magic.h"

/* Parts adapted from Andrew Plotkin's Cheapglk implementation */

/* Copy a GDateTime to a glkdate. */
static void
date_from_gdatetime(glkdate_t *date, GDateTime *dt)
{
    date->year = g_date_time_get_year(dt);
    date->month = g_date_time_get_month(dt);
    date->day = g_date_time_get_day_of_month(dt);
    /* GDateTime has 1-7, with 1 = Monday; Glk has 0-6, with 0 = Sunday */
    date->weekday = g_date_time_get_day_of_week(dt) % G_DATE_SUNDAY;
    date->hour = g_date_time_get_hour(dt);
    date->minute = g_date_time_get_minute(dt);
    date->second = g_date_time_get_second(dt);
}

/* Copy a glkdate to a GDateTime.
 This is used in the "glk_date_to_..." functions, which are supposed
 to normalize the glkdate. We're going to rely on GDateTime to do that.
 Returns NULL if the date is not supported by GDateTime.
 Call g_date_time_unref() on the return value when done.
 */
static GDateTime *
date_to_gdatetime(glkdate_t *date, GTimeZone *tz)
{
	/* Combining seconds and microseconds into one floating-point number should
	 * take care of normalizing any negative microseconds or microseconds > one
	 * million */
	double seconds = date->second + (double)date->microsec / G_USEC_PER_SEC;
	GDateTime *retval = g_date_time_new(tz,
		date->year,
		date->month,
		date->day,
		date->hour,
		date->minute,
		seconds);
	if( G_UNLIKELY(retval == NULL) ) {
		if(date->year < 1)
			WARNING("Years earlier than 1 C.E. are not currently supported.");
		else if(date->year > 9999)
			WARNING("Years later than 9999 C.E. are not currently supported.");
		else
			WARNING("Date is not supported or not valid.");
	}
	return retval;
}

/* Convert a Unix timestamp (seconds since Jan 1 1970) to a glktimeval,
 * adding a number of microseconds as well. */
static void
unix_time_to_time(gint64 sec, int microsec, glktimeval_t *time)
{
	time->high_sec = (sec >> 32) & 0xFFFFFFFF;
	time->low_sec = sec & 0xFFFFFFFF;
    time->microsec = microsec;
}

/* Convert a gint64 microseconds value, as returned by g_get_real_time(),
 * to a glktimeval. */
static void
real_time_to_time(gint64 real_time, glktimeval_t *time)
{
	gint64 unix_time = real_time / G_USEC_PER_SEC;
	int microsec = real_time % G_USEC_PER_SEC;
	unix_time_to_time(unix_time, microsec, time);
}

/* Divide a Unix timestamp by a (positive) value. */
static glsi32
simplify_time(gint64 timestamp, glui32 factor)
{
    /* We want to round towards negative infinity, which takes a little
	 bit of fussing. */
    if (timestamp >= 0) {
		return timestamp / (gint64)factor;
    }
    else {
		return -1 - (((gint64)-1 - timestamp) / (gint64)factor);
    }
}

/* Convert a glkdate to a glktimeval, in the given time zone. */
static void
date_to_time(glkdate_t *date, glktimeval_t *tv, GTimeZone *tz)
{
	GDateTime *dt = date_to_gdatetime(date, tz);
	if(dt == NULL) {
		tv->high_sec = -1;
		tv->low_sec = -1;
		return;
	}
	gint64 timestamp = g_date_time_to_unix(dt);
	int microsec = g_date_time_get_microsecond(dt);
	g_date_time_unref(dt);
	unix_time_to_time(timestamp, microsec, tv);
}

/* Convert a glkdate to a Unix timestamp divided by a value, in the given time
zone. */
static glsi32
date_to_simple_time(glkdate_t *date, glui32 factor, GTimeZone *tz)
{
	GDateTime *dt = date_to_gdatetime(date, tz);
	if(dt == NULL)
		return -1;
	gint64 timestamp = g_date_time_to_unix(dt);
	g_date_time_unref(dt);

	return simplify_time(timestamp, factor);
}

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
	real_time_to_time(g_get_real_time(), time);
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
	g_return_val_if_fail(factor != 0, 0);

	gint64 sec = g_get_real_time() / G_USEC_PER_SEC;
	return simplify_time(sec, factor);
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

	time_t timestamp = time->low_sec;
    if (sizeof(timestamp) > 4) {
        timestamp += ((gint64)time->high_sec << 32);
    }

	GDateTime *dt = g_date_time_new_from_unix_utc(timestamp);
	date_from_gdatetime(date, dt);
	g_date_time_unref(dt);
    date->microsec = time->microsec;
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

	time_t timestamp = time->low_sec;
    if (sizeof(timestamp) > 4) {
        timestamp += ((int64_t)time->high_sec << 32);
    }

	GDateTime *dt = g_date_time_new_from_unix_local(timestamp);
	date_from_gdatetime(date, dt);
	g_date_time_unref(dt);
    date->microsec = time->microsec;
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

	time_t timestamp = (time_t)time * factor;

	GDateTime *dt = g_date_time_new_from_unix_utc(timestamp);
	date_from_gdatetime(date, dt);
	g_date_time_unref(dt);
    date->microsec = 0;
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

	time_t timestamp = (time_t)time * factor;

	GDateTime *dt = g_date_time_new_from_unix_local(timestamp);
	date_from_gdatetime(date, dt);
	g_date_time_unref(dt);
    date->microsec = 0;
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
 * <note><title>Chimara</title><para>
 *   Chimara does not currently support years earlier than 1 C.E. or later than
 *   9999 C.E.
 * </para></note>
 */
void
glk_date_to_time_utc(glkdate_t *date, glktimeval_t *time)
{
	g_return_if_fail(date != NULL);
	g_return_if_fail(time != NULL);

	GTimeZone *utc = g_time_zone_new_utc();
	date_to_time(date, time, utc);
	g_time_zone_unref(utc);
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

	GTimeZone *local = g_time_zone_new_local();
	date_to_time(date, time, local);
	g_time_zone_unref(local);
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
 * <note><title>Chimara</title><para>
 *   Chimara does not currently support years earlier than 1 C.E. or later than
 *   9999 C.E.
 * </para></note>
 *
 * Returns: a timestamp divided by @factor, and truncated to 32 bits, or -1 on
 * error.
 */
glsi32
glk_date_to_simple_time_utc(glkdate_t *date, glui32 factor)
{
	g_return_val_if_fail(date != NULL, 0);
	g_return_val_if_fail(factor != 0, 0);

	GTimeZone *utc = g_time_zone_new_utc();
	glsi32 retval = date_to_simple_time(date, factor, utc);
	g_time_zone_unref(utc);
	return retval;
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
	g_return_val_if_fail(date != NULL, 0);
	g_return_val_if_fail(factor != 0, 0);

	GTimeZone *local = g_time_zone_new_local();
	glsi32 retval = date_to_simple_time(date, factor, local);
	g_time_zone_unref(local);
	return retval;
}
