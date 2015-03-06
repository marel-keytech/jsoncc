#include <stdlib.h>
#include "tst.h"
#include "json_string.h"

char* encode(const char* str)
{
    return json_string_encode(str, strlen(str));
}

char* decode(const char* str)
{
    return json_string_decode(str, strlen(str));
}

static int test_decode_empty_string()
{
    char* out = decode("");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("", out);

    free(out);
    return 0;
}

static int test_decode_regular_string()
{
    char* out = decode("foo, bar.");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("foo, bar.", out);

    free(out);
    return 0;
}

static int test_encode_regular_string()
{
    char* out = encode("foo, bar.");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("foo, bar.", out);

    free(out);
    return 0;
}

static int test_encode_empty_string()
{
    char* out = encode("");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("", out);

    free(out);
    return 0;
}

static int test_decode_newline_string()
{
    char* out = decode("\\nfoo\\n\\nbar\\n");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("\nfoo\n\nbar\n", out);

    free(out);
    return 0;
}

static int test_decode_single_backslash()
{
    char* out = decode("\\");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("", out);

    free(out);
    return 0;
}

static int test_decode_hexcode()
{
    char* out = decode("\\u0041BCD");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("ABCD", out);

    free(out);
    return 0;
}

static int test_encode_newline_string()
{
    char* out = encode("\nfoo\n\nbar\n");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("\\nfoo\\n\\nbar\\n", out);

    free(out);
    return 0;
}

static int test_encode_backslash_string()
{
    char* out = encode("\\\\foo\\bar\\");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("\\\\\\\\foo\\\\bar\\\\", out);

    free(out);
    return 0;
}

static int test_encode_non_printable()
{
    char* out = encode("asdf\x006xxx");
    ASSERT_TRUE(out);

    ASSERT_STR_EQ("asdf\\u0006xxx", out);

    free(out);
    return 0;
}

int main(int argc, char* argv[])
{
    int r = 0;

    RUN_TEST(test_decode_regular_string);
    RUN_TEST(test_decode_empty_string);
    RUN_TEST(test_decode_newline_string);
    RUN_TEST(test_decode_single_backslash);
    RUN_TEST(test_decode_hexcode);

    RUN_TEST(test_encode_regular_string);
    RUN_TEST(test_decode_empty_string);
    RUN_TEST(test_encode_newline_string);
    RUN_TEST(test_encode_backslash_string);
    RUN_TEST(test_encode_non_printable);

    return r;
}

