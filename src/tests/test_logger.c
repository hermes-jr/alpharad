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
#include "../logger.h"
#include "../settings.h"

extern struct settings settings;

void test_logger_levels(void) {
    uint8_t original_verbosity = settings.verbose;

    const char *test_message = "Test me";

    const size_t limit = 1024;
    char *mock_buf = calloc(limit, sizeof(mock_buf));
    FILE *mock_out = fmemopen(mock_buf, limit, "w+");

    /* Lower levels should not be logged */
    settings.verbose = LOG_FATAL;
    log_fp(LOG_INFO, mock_out, test_message);
    fflush(mock_out);

    CU_ASSERT_NSTRING_EQUAL(mock_buf, "", 7)

    /* Same or higher will be logged */
    settings.verbose = LOG_INFO;
    log_fp(LOG_INFO, mock_out, test_message);
    fflush(mock_out);
    CU_ASSERT_NSTRING_EQUAL(mock_buf, test_message, 7)

    settings.verbose = LOG_TRACE;
    log_fp(LOG_INFO, mock_out, test_message);
    fflush(mock_out);
    CU_ASSERT_NSTRING_EQUAL(mock_buf, test_message, 7)

    settings.verbose = original_verbosity;
    free(mock_buf);
    fclose(mock_out);
}