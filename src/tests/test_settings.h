#ifndef ALPHARAD_TEST_SETTINGS_H
#define ALPHARAD_TEST_SETTINGS_H

void settings_test_init(void);

void settings_test_teardown(void);

void test_settings_population_dimensions(void);

void test_settings_incorrect_width(void);

void test_settings_incorrect_height(void);

void test_settings_population_device(void);

void test_settings_population_outfile(void);

void test_settings_population_verbose(void);

void test_settings_population_logfile(void);

void test_settings_population_crop(void);

void test_settings_population_help(void);

void test_settings_unrecognized_option(void);

#endif //ALPHARAD_TEST_SETTINGS_H
