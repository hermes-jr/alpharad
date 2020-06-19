#include <CUnit/Basic.h>
#include <setjmp.h>
#include "../settings.h"
#include "test_settings.h"

extern struct settings settings;

void test_settings_population_dimensions(void) {
    char **argv = (char *[]) {"prog", "--geometry=100x200"};
    populate_settings(2, argv);
    CU_ASSERT_EQUAL(settings.width, 100);
    CU_ASSERT_EQUAL(settings.height, 200);
}

void test_settings_incorrect_dimensions(void) {
    char **argv = (char *[]) {"prog", "--geometry=FUBARx-1"};
    populate_settings(2, argv);
//    CU_FAIL("Program should have quit by now");
// TODO: damn... maybe later
}

void test_settings_population_device(void) {
    char *dn = "/dev/somedevice";
    char **argv = (char *[]) {"prog", "--device", dn};
    populate_settings(3, argv);
    CU_ASSERT_STRING_EQUAL(settings.dev_name, dn);
}

void test_settings_population_outfile(void) {
    char *ofn = "testout";
    char **argv = (char *[]) {"prog", "-o", ofn};
    populate_settings(3, argv);
    CU_ASSERT_STRING_EQUAL(settings.file_out_name, ofn);
}

void test_settings_population_logfile(void) {
    char *hln = "testlog";
    char **argv = (char *[]) {"prog", "--hitlog-file", hln};
    populate_settings(3, argv);
    CU_ASSERT_STRING_EQUAL(settings.file_hitlog_name, hln);
}

void test_settings_population_verbose(void) {
    CU_ASSERT_EQUAL(settings.verbose, 0u);
    char **argv = (char *[]) {"prog", "-v", "3"};
    populate_settings(3, argv);
    CU_ASSERT_EQUAL(settings.verbose, 3u);
}
