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

#ifndef ALPHARAD_TEST_DATA_EXTRACTORS_H
#define ALPHARAD_TEST_DATA_EXTRACTORS_H

#include <stdlib.h>

void data_extractors_test_init(void);

void data_extractors_test_teardown(void);

void test_data_default(void);

#if HAVE_OPENSSL

void test_data_sha256_non_blank_frames(void);

void test_data_sha256_all_frames(void);

#endif //HAVE_OPENSSL

void test_data_bit_accumulator(void);

#endif //ALPHARAD_TEST_DATA_EXTRACTORS_H
