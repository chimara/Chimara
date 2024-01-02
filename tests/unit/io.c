#include <stdio.h>

#include "glk.h"
#include "glkunit.h"

static int
test_glk_put_char_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_stream_set_current(stream);
    glk_put_char('A');

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[4] = {0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 4);
    ASSERT_EQUAL(count, 1);
    ASSERT_EQUAL(buf[0], 'A');
    ASSERT_EQUAL(buf[1], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_char_stream_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_put_char_stream(stream, 'A');

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[4] = {0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 4);
    ASSERT_EQUAL(count, 1);
    ASSERT_EQUAL(buf[0], 'A');
    ASSERT_EQUAL(buf[1], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_string_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_stream_set_current(stream);
    glk_put_string("AbC");

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[4] = {0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 4);
    ASSERT_EQUAL(count, 3);
    ASSERT_EQUAL(buf[0], 'A');
    ASSERT_EQUAL(buf[1], 'b');
    ASSERT_EQUAL(buf[2], 'C');
    ASSERT_EQUAL(buf[3], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_string_stream_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_put_string_stream(stream, "AbC");

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[4] = {0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 4);
    ASSERT_EQUAL(count, 3);
    ASSERT_EQUAL(buf[0], 'A');
    ASSERT_EQUAL(buf[1], 'b');
    ASSERT_EQUAL(buf[2], 'C');
    ASSERT_EQUAL(buf[3], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_buffer_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_stream_set_current(stream);
    char inbuf[4] = {'A', 'b', 'C', 0xfe};
    glk_put_buffer(inbuf, 3);

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[4] = {0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 4);
    ASSERT_EQUAL(count, 3);
    ASSERT_EQUAL(buf[0], 'A');
    ASSERT_EQUAL(buf[1], 'b');
    ASSERT_EQUAL(buf[2], 'C');
    ASSERT_EQUAL(buf[3], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_buffer_stream_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    char inbuf[4] = {'A', 'b', 'C', 0xfe};
    glk_put_buffer_stream(stream, inbuf, 3);

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[4] = {0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 4);
    ASSERT_EQUAL(count, 3);
    ASSERT_EQUAL(buf[0], 'A');
    ASSERT_EQUAL(buf[1], 'b');
    ASSERT_EQUAL(buf[2], 'C');
    ASSERT_EQUAL(buf[3], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_char_uni_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_stream_set_current(stream);
    glk_put_char_uni(U'Ä');

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[4] = {0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 4);
    ASSERT_EQUAL(count, 2);  /* length of UTF-8 encoded Ä is 2 bytes */
    ASSERT_EQUAL(buf[0], 0xc3);
    ASSERT_EQUAL(buf[1], 0x84);
    ASSERT_EQUAL(buf[2], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_char_stream_uni_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_put_char_stream_uni(stream, U'Ä');

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[4] = {0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 4);
    ASSERT_EQUAL(count, 2);  /* length of UTF-8 encoded Ä is 2 bytes */
    ASSERT_EQUAL(buf[0], 0xc3);
    ASSERT_EQUAL(buf[1], 0x84);
    ASSERT_EQUAL(buf[2], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_string_uni_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_stream_set_current(stream);
    glk_put_string_uni(U"Ä🐝ℂ");

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[12] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 12);
    ASSERT_EQUAL(count, 9);  /* 2 + 4 + 3 */
    ASSERT_EQUAL(buf[0], 0xc3);  /* Ä U+00C4*/
    ASSERT_EQUAL(buf[1], 0x84);
    ASSERT_EQUAL(buf[2], 0xf0);  /* honeybee emoji U+1F41D */
    ASSERT_EQUAL(buf[3], 0x9f);
    ASSERT_EQUAL(buf[4], 0x90);
    ASSERT_EQUAL(buf[5], 0x9d);
    ASSERT_EQUAL(buf[6], 0xe2);  /* double-struck C U+2102 */
    ASSERT_EQUAL(buf[7], 0x84);
    ASSERT_EQUAL(buf[8], 0x82);
    ASSERT_EQUAL(buf[9], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_string_stream_uni_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glk_put_string_stream_uni(stream, U"Ä🐝ℂ");

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[12] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 12);
    ASSERT_EQUAL(count, 9);  /* 2 + 4 + 3 */
    ASSERT_EQUAL(buf[0], 0xc3);  /* Ä U+00C4*/
    ASSERT_EQUAL(buf[1], 0x84);
    ASSERT_EQUAL(buf[2], 0xf0);  /* honeybee emoji U+1F41D */
    ASSERT_EQUAL(buf[3], 0x9f);
    ASSERT_EQUAL(buf[4], 0x90);
    ASSERT_EQUAL(buf[5], 0x9d);
    ASSERT_EQUAL(buf[6], 0xe2);  /* double-struck C U+2102 */
    ASSERT_EQUAL(buf[7], 0x84);
    ASSERT_EQUAL(buf[8], 0x82);
    ASSERT_EQUAL(buf[9], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_buffer_uni_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glui32 inbuf[4] = {U'Ä', U'🐝', U'ℂ', 0xfefefefe};

    glk_stream_set_current(stream);
    glk_put_buffer_uni(inbuf, 3);

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[12] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 12);
    ASSERT_EQUAL(count, 9);  /* 2 + 4 + 3 */
    ASSERT_EQUAL(buf[0], 0xc3);  /* Ä U+00C4*/
    ASSERT_EQUAL(buf[1], 0x84);
    ASSERT_EQUAL(buf[2], 0xf0);  /* honeybee emoji U+1F41D */
    ASSERT_EQUAL(buf[3], 0x9f);
    ASSERT_EQUAL(buf[4], 0x90);
    ASSERT_EQUAL(buf[5], 0x9d);
    ASSERT_EQUAL(buf[6], 0xe2);  /* double-struck C U+2102 */
    ASSERT_EQUAL(buf[7], 0x84);
    ASSERT_EQUAL(buf[8], 0x82);
    ASSERT_EQUAL(buf[9], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

static int
test_glk_put_buffer_stream_uni_writes_utf8_to_unicode_text_file(void)
{
    frefid_t ref = glk_fileref_create_temp(fileusage_TextMode | fileusage_Data, 0);
    ASSERT_NONNULL(ref, "temp fileref should succeed");

    strid_t stream = glk_stream_open_file_uni(ref, filemode_Write, 0);
    ASSERT_NONNULL(stream, "write stream should succeed");

    glui32 inbuf[4] = {U'Ä', U'🐝', U'ℂ', 0xfefefefe};

    glk_put_buffer_stream_uni(stream, inbuf, 3);

    glk_stream_close(stream, /* counts = */ NULL);

    frefid_t ref2 = glk_fileref_create_from_fileref(fileusage_BinaryMode | fileusage_Data, ref, 0);
    ASSERT_NONNULL(ref2, "second fileref should succeed");

    stream = glk_stream_open_file(ref2, filemode_Read, 0);
    ASSERT_NONNULL(stream, "read stream should succeed");

    unsigned char buf[12] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    glui32 count = glk_get_buffer_stream(stream, (char *)buf, 12);
    ASSERT_EQUAL(count, 9);  /* 2 + 4 + 3 */
    ASSERT_EQUAL(buf[0], 0xc3);  /* Ä U+00C4*/
    ASSERT_EQUAL(buf[1], 0x84);
    ASSERT_EQUAL(buf[2], 0xf0);  /* honeybee emoji U+1F41D */
    ASSERT_EQUAL(buf[3], 0x9f);
    ASSERT_EQUAL(buf[4], 0x90);
    ASSERT_EQUAL(buf[5], 0x9d);
    ASSERT_EQUAL(buf[6], 0xe2);  /* double-struck C U+2102 */
    ASSERT_EQUAL(buf[7], 0x84);
    ASSERT_EQUAL(buf[8], 0x82);
    ASSERT_EQUAL(buf[9], 0xff);

    glk_stream_close(stream, /* counts = */ NULL);

    glk_fileref_delete_file(ref);
    glk_fileref_destroy(ref);

    SUCCEED;
}

struct TestDescription tests[] = {
    { "glk_put_char() writes UTF-8 to a Unicode text file",
        test_glk_put_char_writes_utf8_to_unicode_text_file },
    { "glk_put_char_stream() writes UTF-8 to a Unicode text file",
        test_glk_put_char_stream_writes_utf8_to_unicode_text_file },
    { "glk_put_string() writes UTF-8 to a Unicode text file",
        test_glk_put_string_writes_utf8_to_unicode_text_file },
    { "glk_put_string_stream() writes UTF-8 to a Unicode text file",
        test_glk_put_string_stream_writes_utf8_to_unicode_text_file },
    { "glk_put_buffer() writes UTF-8 to a Unicode text file",
        test_glk_put_buffer_writes_utf8_to_unicode_text_file },
    { "glk_put_buffer_stream() writes UTF-8 to a Unicode text file",
        test_glk_put_buffer_stream_writes_utf8_to_unicode_text_file },
    { "glk_put_char_uni() writes UTF-8 to a Unicode text file",
        test_glk_put_char_uni_writes_utf8_to_unicode_text_file },
    { "glk_put_char_stream_uni() writes UTF-8 to a Unicode text file",
        test_glk_put_char_stream_uni_writes_utf8_to_unicode_text_file },
    { "glk_put_string_uni() writes UTF-8 to a Unicode text file",
        test_glk_put_string_uni_writes_utf8_to_unicode_text_file },
    { "glk_put_string_stream_uni() writes UTF-8 to a Unicode text file",
        test_glk_put_string_stream_uni_writes_utf8_to_unicode_text_file },
    { "glk_put_buffer_uni() writes UTF-8 to a Unicode text file",
        test_glk_put_buffer_uni_writes_utf8_to_unicode_text_file },
    { "glk_put_buffer_stream_uni() writes UTF-8 to a Unicode text file",
        test_glk_put_buffer_stream_uni_writes_utf8_to_unicode_text_file },
    { NULL, NULL }
};
