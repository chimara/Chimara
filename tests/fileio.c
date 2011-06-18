/* Test for file I/O bug */

#include <libchimara/glk.h>
#include <glib.h>
#include <string.h>

#define MAGIC_STRING "Zapp\xF6licious.\n"
#define BUFLEN 80

void
glk_main(void)
{
	char buffer[BUFLEN + 1];
	
	/* Open a temporary file */
	frefid_t ref = glk_fileref_create_temp(fileusage_Data | fileusage_BinaryMode, 0);
	strid_t file = glk_stream_open_file_uni(ref, filemode_Write, 0);
	
	/* Write the string to the file */
	glk_put_string_stream(file, MAGIC_STRING);
	
	/* Close and check result counts */
	stream_result_t counts;
	glk_stream_close(file, &counts);
	g_assert_cmpint(counts.readcount, ==, 0);
	g_assert_cmpint(counts.writecount, ==, 14);
	
	file = glk_stream_open_file_uni(ref, filemode_Read, 0);
	glui32 readcount = glk_get_line_stream(file, buffer, BUFLEN);
	g_printerr("String: %s\n", buffer);
	g_assert_cmpint(readcount, ==, strlen(buffer));
	
	glk_stream_close(file, &counts);
	glk_fileref_destroy(ref);
}
