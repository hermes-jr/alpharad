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

#ifndef ALPHARAD_TEST_IMAGE_PROCESSING_H
#define ALPHARAD_TEST_IMAGE_PROCESSING_H

#include <stdlib.h>
#include "../frame_cca.h"

void image_processing_test_init(void);

void image_processing_test_teardown(void);

void test_image_ccl(void);

void test_image_ccl_rr(void);

void test_image_return_single(void);

void test_image_return_multiple(void);

void test_image_wrapper(void);

void test_image_empty(void);

void test_image_logging(void);

void test_image_border_crop(void);

void test_image_rounding_errors(void);

void test_image_ll(void);

uint cartesian_to_yuv(coordinate c);

void dump_list(node_t *head);

#endif //ALPHARAD_TEST_IMAGE_PROCESSING_H
