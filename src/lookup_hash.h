/*
    Copyright (C) 2008  Daniel Richman

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published 
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    For a full copy of the GNU Lesser General Public License, 
    see <http://www.gnu.org/licenses/>.
*/

#ifndef LOOKUP_HASH_HEADER
#define LOOKUP_HASH_HEADER

#include <stdio.h>
#include <stdint.h>

uint32_t lookup_hash(const void *key, size_t length);

#endif  /* LOOKUP_HASH_HEADER */

