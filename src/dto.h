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

#ifndef ALPHARAD_DTO_H
#define ALPHARAD_DTO_H

#include <stdint.h>

/* Don't forget to free *arr after use! */
typedef struct {
    uint len;
    uint8_t *arr;
} bytes_spawned;

typedef struct {
    uint x;
    uint y;
} coordinate;

/* Don't forget to free *arr after use! */
typedef struct {
    uint len;
    coordinate *arr;
} points_detected;

#endif //ALPHARAD_DTO_H