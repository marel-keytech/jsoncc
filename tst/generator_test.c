#include <stdlib.h>
#include <string.h>
#include "tst.h"
#include "test.h"

static int test_integer()
{
    struct test in, out;
    memset(&in, 0, sizeof(in));
    in.is_set_the_integer = 1;
    in.the_integer = 42;
    
    char* json = test_pack(&in);
    ASSERT_TRUE(json);

    ASSERT_INT_GE(0, test_unpack(&out, json));
    ASSERT_TRUE(out.is_set_the_integer);
    ASSERT_INT_EQ(42, out.the_integer);

    test_cleanup(&out);
    free(json);
    return 0;
}

static int test_real()
{
    struct test in, out;
    memset(&in, 0, sizeof(in));
    in.is_set_the_real = 1;
    in.the_real = 3.14;
    
    char* json = test_pack(&in);
    ASSERT_TRUE(json);

    ASSERT_INT_GE(0, test_unpack(&out, json));
    ASSERT_TRUE(out.is_set_the_real);
    ASSERT_DOUBLE_GT(3.1399, out.the_real);
    ASSERT_DOUBLE_LT(3.1401, out.the_real);

    test_cleanup(&out);
    free(json);
    return 0;
}

static int test_bool()
{
    struct test in, out;
    memset(&in, 0, sizeof(in));
    in.is_set_the_bool = 1;
    in.the_bool = 1;
    
    char* json = test_pack(&in);
    ASSERT_TRUE(json);

    ASSERT_INT_GE(0, test_unpack(&out, json));
    ASSERT_TRUE(out.is_set_the_bool);
    ASSERT_TRUE(out.the_bool);

    test_cleanup(&out);
    free(json);
    return 0;
}

static int test_string()
{
    struct test in, out;
    memset(&in, 0, sizeof(in));
    in.is_set_the_string = 1;
    in.the_string = "foobar";
    
    char* json = test_pack(&in);
    ASSERT_TRUE(json);

    ASSERT_INT_GE(0, test_unpack(&out, json));
    ASSERT_TRUE(out.is_set_the_string);
    ASSERT_STR_EQ("foobar", out.the_string);

    test_cleanup(&out);
    free(json);
    return 0;
}

static int test_object()
{
    struct test in, out;
    memset(&in, 0, sizeof(in));
    in.is_set_the_object = 1;
    in.the_object.the_member = 42;
    
    char* json = test_pack(&in);
    ASSERT_TRUE(json);

    ASSERT_INT_GE(0, test_unpack(&out, json));
    ASSERT_TRUE(out.is_set_the_object);
    ASSERT_INT_EQ(42, out.the_object.the_member);

    test_cleanup(&out);
    free(json);
    return 0;
}

static int test_any()
{
    struct test in, out;
    memset(&in, 0, sizeof(in));
    in.is_set_the_any = 1;
    in.the_any.type = JSON_OBJ_STRING;
    in.the_any.string_ = "foobar";
    
    char* json = test_pack(&in);
    ASSERT_TRUE(json);

    ASSERT_INT_GE(0, test_unpack(&out, json));
    ASSERT_TRUE(out.is_set_the_any);
    ASSERT_INT_EQ(JSON_OBJ_STRING, out.the_any.type);
    ASSERT_STR_EQ("foobar", out.the_any.string_);

    test_cleanup(&out);
    free(json);
    return 0;
}

static int test_array()
{
    struct test in, out;
    memset(&in, 0, sizeof(in));
    in.is_set_the_array = 1;
    long long array[] = { 1, 2, 3 };
    in.the_array = array;
    in.length_of_the_array = 3;
    
    char* json = test_pack(&in);
    ASSERT_TRUE(json);

    ASSERT_INT_GE(0, test_unpack(&out, json));
    ASSERT_TRUE(out.is_set_the_array);
    ASSERT_INT_EQ(3, out.length_of_the_array);
    ASSERT_INT_EQ(1, out.the_array[0]);
    ASSERT_INT_EQ(2, out.the_array[1]);
    ASSERT_INT_EQ(3, out.the_array[2]);

    test_cleanup(&out);
    free(json);
    return 0;
}

int main()
{
    int r = 0;
    RUN_TEST(test_integer);
    RUN_TEST(test_real);
    RUN_TEST(test_bool);
    RUN_TEST(test_string);
    RUN_TEST(test_object);
    RUN_TEST(test_any);
    RUN_TEST(test_array);
    return r;
}

