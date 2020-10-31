#ifndef ALPHARAD_TEST_SETTINGS_H
#define ALPHARAD_TEST_SETTINGS_H

int settings_suite_init(void);

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

void test_settings_population_mode(void);

void test_settings_population_mode_unrecognized(void);

void test_settings_population_print_modes(void);

#endif //ALPHARAD_TEST_SETTINGS_H
