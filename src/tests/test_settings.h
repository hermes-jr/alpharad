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

void test_settings_population_threshold(void);

void test_settings_population_help(void);

void test_settings_unrecognized_option(void);

void test_settings_population_mode(void);

void test_settings_population_mode_unrecognized(void);

void test_settings_population_print_modes(void);

#endif //ALPHARAD_TEST_SETTINGS_H
