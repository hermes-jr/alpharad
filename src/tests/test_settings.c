/*
Copyright (C) 2020 Mikhail Antonov <hermes@cyllene.net>

This file is part of alpharad project.

alpharad is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

alpharad is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with alpharad.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <CUnit/Basic.h>
#include "../settings.h"
#include "../data_extractors.h"
#include "test_settings.h"
#include <getopt.h>

extern struct settings settings;

int settings_suite_init(void) {
    register_processors();
    return 0;
}

/* Reset getopt state to supply new vector each time */
void settings_test_init(void) {
    optind = 1;
}

/* Restore whatever was ruined by test */
void settings_test_teardown(void) {
    settings.dev_name = S_DEFAULT_DEV_NAME;
    settings.file_out_name = S_DEFAULT_FILE_OUT_NAME;
    settings.file_out = S_DEFAULT_FILE_OUT;
    settings.file_hits_name = S_DEFAULT_FILE_HITS_NAME;
    settings.file_hits = S_DEFAULT_FILE_HITS;
    settings.frame_processor = (frame_processor_t) S_DEFAULT_FRAME_PROCESSOR;
    settings.width = S_DEFAULT_WIDTH;
    settings.height = S_DEFAULT_HEIGHT;
    settings.crop = S_DEFAULT_CROP;
    settings.threshold = S_DEFAULT_THRESHOLD;
    settings.verbose = S_DEFAULT_VERBOSE;
}

void test_settings_population_dimensions(void) {
    char **argv = (char *[]) {"prog", "--geometry=100:200"};
    populate_settings(stdout, argv, 2);
    CU_ASSERT_EQUAL(settings.width, 100)
    CU_ASSERT_EQUAL(settings.height, 200)
}

void test_settings_incorrect_width(void) {
    char mock_buf[BUFSIZ];
    FILE *mock_out = fmemopen(mock_buf, BUFSIZ, "w+");

    char **argv_w = (char *[]) {"prog", "--geometry=FOOBAR:-1"};
    int settings_ret = populate_settings(mock_out, argv_w, 2);
    fflush(mock_out);

    /* Expect fatal error */
    CU_ASSERT_EQUAL(settings_ret, -1)
    CU_ASSERT_NSTRING_EQUAL(mock_buf, "Couldn't parse width", 20)

    fclose(mock_out);
}

void test_settings_incorrect_height(void) {
    char mock_buf[BUFSIZ];
    FILE *mock_out = fmemopen(mock_buf, BUFSIZ, "w+");

    char **argv_h = (char *[]) {"prog", "--geometry=640:FOOBAR"};
    int settings_ret = populate_settings(mock_out, argv_h, 2);
    fflush(mock_out);

    /* Expect fatal error */
    CU_ASSERT_EQUAL(settings_ret, -1)
    CU_ASSERT_NSTRING_EQUAL(mock_buf, "Couldn't parse height", 21)

    fclose(mock_out);
}

void test_settings_population_device(void) {
    char *dn = "/dev/some_device";
    char **argv = (char *[]) {"prog", "--device", dn};
    populate_settings(stdout, argv, 3);
    CU_ASSERT_STRING_EQUAL(settings.dev_name, dn)
}

void test_settings_population_outfile(void) {
    char *ofn = "test_out";
    char **argv = (char *[]) {"prog", "-o", ofn};
    int settings_ret = populate_settings(stdout, argv, 3);

    /* Expect no error */
    CU_ASSERT_EQUAL(settings_ret, 0)
    CU_ASSERT_STRING_EQUAL(settings.file_out_name, ofn)
}

void test_settings_population_logfile(void) {
    char *hln = "test_log";
    char **argv = (char *[]) {"prog", "--hits-file", hln};
    populate_settings(stdout, argv, 3);
    CU_ASSERT_STRING_EQUAL(settings.file_hits_name, hln)
}

void test_settings_population_verbose(void) {
    CU_ASSERT_EQUAL(settings.verbose, 0u)
    char **argv = (char *[]) {"prog", "-v", "3"};
    populate_settings(stdout, argv, 3);
    CU_ASSERT_EQUAL(settings.verbose, 3u)
}

void test_settings_population_crop(void) {
    CU_ASSERT_EQUAL(settings.crop, 0u)
    char **argv = (char *[]) {"prog", "-b", "7"};
    populate_settings(stdout, argv, 3);
    CU_ASSERT_EQUAL(settings.crop, 7u)
}

void test_settings_population_threshold(void) {
    CU_ASSERT_EQUAL(settings.threshold, S_DEFAULT_THRESHOLD)
    char **argv = (char *[]) {"prog", "--threshold=42"};
    populate_settings(stdout, argv, 2);
    CU_ASSERT_EQUAL(settings.threshold, 42u)
}

void test_settings_population_help(void) {
    char mock_buf[BUFSIZ];
    FILE *mock_out = fmemopen(mock_buf, BUFSIZ, "w+");

    char **argv_h = (char *[]) {"prog", "-h"};
    int settings_ret = populate_settings(mock_out, argv_h, 2);
    fflush(mock_out);

    /* This is ok situation, return value should indicate exit with success */
    CU_ASSERT_EQUAL(settings_ret, 1)
    CU_ASSERT_NSTRING_EQUAL(mock_buf, "Usage: prog", 11)

    fclose(mock_out);
}

void test_settings_unrecognized_option(void) {
    char mock_buf[BUFSIZ];
    FILE *mock_out = fmemopen(mock_buf, BUFSIZ, "w+");

    /* Passing unknown option should lead to help message being printed and failure exit code */
    char **argv_h = (char *[]) {"prog", "--this-should-be-an-unsupported-parameter-name"};
    int settings_ret = populate_settings(mock_out, argv_h, 2);
    fflush(mock_out);

    /* Expect fatal error and help printed */
    CU_ASSERT_EQUAL(settings_ret, -1)
    CU_ASSERT_NSTRING_EQUAL(mock_buf, "Usage: prog", 11)

    fclose(mock_out);
}


void test_settings_population_mode(void) {
    CU_ASSERT_PTR_NOT_EQUAL(settings.frame_processor.execute, process_image_comparator)

    char **argv_h = (char *[]) {"prog", "-m", "COMPARATOR"};
    int settings_ret = populate_settings(stdout, argv_h, 3);

    CU_ASSERT_EQUAL(settings_ret, 0)
    CU_ASSERT_PTR_EQUAL(settings.frame_processor.execute, process_image_comparator)
}

void test_settings_population_mode_unrecognized(void) {
    char mock_buf[BUFSIZ];
    FILE *mock_out = fmemopen(mock_buf, BUFSIZ, "w+");

    char **argv_h = (char *[]) {"prog", "-m", "garbage"};
    int settings_ret = populate_settings(mock_out, argv_h, 3);
    fflush(mock_out);

    CU_ASSERT_EQUAL(settings_ret, -1)
    CU_ASSERT_NSTRING_EQUAL(mock_buf, "Unsupported processor requested", 31)

    fclose(mock_out);
}

void test_settings_population_print_modes(void) {
    char mock_buf[BUFSIZ];
    FILE *mock_out = fmemopen(mock_buf, BUFSIZ, "w+");

    char **argv_h = (char *[]) {"prog", "-M"};
    int settings_ret = populate_settings(mock_out, argv_h, 2);
    fflush(mock_out);

    CU_ASSERT_EQUAL(settings_ret, 1)
    CU_ASSERT_NSTRING_EQUAL(mock_buf, "DEFAULT", 7)

    fclose(mock_out);
}
