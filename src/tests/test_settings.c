#include <CUnit/Basic.h>
#include "../settings.h"
#include "test_settings.h"
#include <getopt.h>

extern struct settings settings;

/* Reset getopt state to supply new vector each time */
void settings_test_init(void) {
    optind = 1;
}

void test_settings_population_dimensions(void) {
    char **argv = (char *[]) {"prog", "--geometry=100:200"};
    populate_settings(2, argv, stdout);
    CU_ASSERT_EQUAL(settings.width, 100)
    CU_ASSERT_EQUAL(settings.height, 200)
}

void test_settings_incorrect_width(void) {
    const size_t limit = 1024;
    char mock_buf[limit];
    FILE *mock_out = fmemopen(mock_buf, limit, "w+");

    char **argv_w = (char *[]) {"prog", "--geometry=FOOBAR:-1"};
    int settings_ret = populate_settings(2, argv_w, mock_out);
    fflush(mock_out);

    /* Expect fatal error */
    CU_ASSERT_EQUAL(settings_ret, -1)
    CU_ASSERT_STRING_EQUAL(mock_buf, "Couldn't parse width\n")

    fclose(mock_out);
}

void test_settings_incorrect_height(void) {
    const size_t limit = 1024;
    char mock_buf[limit];
    FILE *mock_out = fmemopen(mock_buf, limit, "w+");

    char **argv_h = (char *[]) {"prog", "--geometry=640:FOOBAR"};
    int settings_ret = populate_settings(2, argv_h, mock_out);
    fflush(mock_out);

    /* Expect fatal error */
    CU_ASSERT_EQUAL(settings_ret, -1)
    CU_ASSERT_STRING_EQUAL(mock_buf, "Couldn't parse height\n")

    fclose(mock_out);
}

void test_settings_population_device(void) {
    char *dn = "/dev/some_device";
    char **argv = (char *[]) {"prog", "--device", dn};
    populate_settings(3, argv, stdout);
    CU_ASSERT_STRING_EQUAL(settings.dev_name, dn)
}

void test_settings_population_outfile(void) {
    char *ofn = "test_out";
    char **argv = (char *[]) {"prog", "-o", ofn};
    int settings_ret = populate_settings(3, argv, stdout);

    /* Expect no error */
    CU_ASSERT_EQUAL(settings_ret, 0)
    CU_ASSERT_STRING_EQUAL(settings.file_out_name, ofn)
}

void test_settings_population_logfile(void) {
    char *hln = "test_log";
    char **argv = (char *[]) {"prog", "--hits-file", hln};
    populate_settings(3, argv, stdout);
    CU_ASSERT_STRING_EQUAL(settings.file_hits_name, hln)
}

void test_settings_population_verbose(void) {
    CU_ASSERT_EQUAL(settings.verbose, 0u)
    char **argv = (char *[]) {"prog", "-v", "3"};
    populate_settings(3, argv, stdout);
    CU_ASSERT_EQUAL(settings.verbose, 3u)
}
