#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "glk.h"
#include "gi_blorb.h"
#include "glkunit.h"

static int
test_glk_stream_open_resource_reads_bina_chunk_not_including_header(void)
{
    strid_t stream = glk_stream_open_resource(1, 0);
    ASSERT_NONNULL(stream, "Opening resource stream should succeed");

    char magic[4];
    glui32 read = glk_get_buffer_stream(stream, magic, 4);
    ASSERT_EQUAL(read, 4);

    ASSERT_NOT_EQUAL(strncmp(magic, "BINA", 4), 0);

    glk_stream_close(stream, /* counts = */ NULL);

    SUCCEED;
}

static int
test_glk_stream_open_resource_uni_reads_bina_chunk_not_including_header(void)
{
    strid_t stream = glk_stream_open_resource_uni(1, 0);
    ASSERT_NONNULL(stream, "Opening resource stream should succeed");

    glui32 magic;
    glui32 read = glk_get_buffer_stream_uni(stream, &magic, 1);
    ASSERT_EQUAL(read, 1);

    glui32 header_BINA = 0x42494e41;
    ASSERT_NOT_EQUAL(magic, header_BINA);

    glk_stream_close(stream, /* counts = */ NULL);

    SUCCEED;
}

static int
test_glk_stream_open_resource_reads_form_chunk_including_header(void)
{
    strid_t stream = glk_stream_open_resource(3, 0);
    ASSERT_NONNULL(stream, "Opening resource stream should succeed");

    char magic[4];
    glui32 read = glk_get_buffer_stream(stream, magic, 4);
    ASSERT_EQUAL(read, 4);

    ASSERT_EQUAL(strncmp(magic, "FORM", 4), 0);

    glk_stream_close(stream, /* counts = */ NULL);

    SUCCEED;
}

static int
test_glk_stream_open_resource_uni_reads_form_chunk_including_header(void)
{
    strid_t stream = glk_stream_open_resource_uni(3, 0);
    ASSERT_NONNULL(stream, "Opening resource stream should succeed");

    glui32 magic;
    glui32 read = glk_get_buffer_stream_uni(stream, &magic, 1);
    ASSERT_EQUAL(read, 1);

    glui32 header_FORM = 0x464f524d;
    ASSERT_EQUAL(magic, header_FORM);

    glk_stream_close(stream, /* counts = */ NULL);

    SUCCEED;
}

struct TestDescription tests[] = {
    { "glk_stream_open_resource() reads a BINA chunk not including header",
        test_glk_stream_open_resource_reads_bina_chunk_not_including_header },
    { "glk_stream_open_resource_uni() reads a BINA chunk not including header",
        test_glk_stream_open_resource_uni_reads_bina_chunk_not_including_header },
    { "glk_stream_open_resource() reads a FORM chunk including header",
        test_glk_stream_open_resource_reads_form_chunk_including_header },
    { "glk_stream_open_resource_uni() reads a FORM chunk including header",
        test_glk_stream_open_resource_uni_reads_form_chunk_including_header },
    { NULL, NULL }
};
