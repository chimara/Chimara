/* Test for file I/O bug */

#include <libchimara/glk.h>
#include <glib.h>
#include <string.h>

#define MAGIC_STRING "Zapp\xF6licious.\n"
#define BUFLEN 80

static void
delete_if_exists(frefid_t ref)
{
    if(glk_fileref_does_file_exist(ref) == 1) {
        g_print("(Deleting existing file) ");
		glk_fileref_delete_file(ref);
    }
}

void
glk_main(void)
{
	char buffer[BUFLEN + 1];
	
	g_print("Test getline... ");

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
	g_print("(String: %s) ", buffer);
	g_assert_cmpint(readcount, ==, strlen(buffer));
	
	g_print("PASS\n");

	glk_stream_close(file, &counts);
	glk_fileref_destroy(ref);

	/* testfile7 - append, seek, write, close, read. */
	g_print("Test append-seek-write-close-read... ");

	ref = glk_fileref_create_by_name(fileusage_Data | fileusage_BinaryMode, "testfile7", 0);
	delete_if_exists(ref);
	strid_t str = glk_stream_open_file(ref, filemode_WriteAppend, 0);
	glk_put_string_stream(str, "Purple monkey chef.\n");
	glk_stream_set_position(str, 14, seekmode_Start);
	g_assert_cmpuint(glk_stream_get_position(str), ==, 14);
	glk_put_string_stream(str, "dishwasher.\n");
	glk_stream_close(str, &counts);
	g_assert_cmpuint(counts.readcount, ==, 0);
	g_assert_cmpuint(counts.writecount, ==, 32);

	str = glk_stream_open_file(ref, filemode_Read, 0);
	readcount = glk_get_buffer_stream(str, buffer, BUFLEN);
	buffer[readcount] = '\0';
	g_assert_cmpstr(buffer, ==, "Purple monkey dishwasher.\n");

	glk_stream_close(str, &counts);
	g_assert_cmpuint(counts.readcount, ==, 26);
	g_assert_cmpuint(counts.writecount, ==, 0);

	g_print("PASS\n");

	/* testfile10 - Write, close, read, write, close, read. */
	g_print("Test write-close-read-write-close-read... ");

	ref = glk_fileref_create_by_name(fileusage_Data | fileusage_BinaryMode, "testfile10", 0);
	delete_if_exists(ref);
	str = glk_stream_open_file(ref, filemode_ReadWrite, 0);
	glk_put_string_stream(str, "Purple synchroscopes.\n");
	glk_stream_close(str, &counts);
	g_assert_cmpuint(counts.readcount, ==, 0);
	g_assert_cmpuint(counts.writecount, ==, 22);

	str = glk_stream_open_file(ref, filemode_ReadWrite, 0);
	readcount = glk_get_buffer_stream(str, buffer, 7);
	buffer[readcount] = '\0';
	g_assert_cmpstr(buffer, ==, "Purple ");
	g_assert_cmpuint(glk_stream_get_position(str), ==, 7);
	glk_put_string_stream(str, "monkey dishwasher.\n");
	glk_stream_set_position(str, 0, seekmode_Start);
	g_assert_cmpuint(glk_stream_get_position(str), ==, 0);
	readcount = glk_get_buffer_stream(str, buffer, BUFLEN);
	buffer[readcount] = '\0';
	g_assert_cmpstr(buffer, ==, "Purple monkey dishwasher.\n");
	glk_stream_close(str, &counts);
	g_assert_cmpuint(counts.readcount, ==, 33);
	g_assert_cmpuint(counts.writecount, ==, 19);

	g_print("PASS\n");
}
