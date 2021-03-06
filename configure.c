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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

static inline void put_string(FILE *f, const char *s)
{
  if (fputs(s, f) == EOF)
  {
    perror("Error writing to file");
    exit(EXIT_FAILURE);
  }
}

#define check_size(type, size) check_size_(sizeof(type), (size), #type)
static inline void check_size_(size_t real_size, size_t intended_size, 
                               const char *name)
{
  if (real_size != intended_size)
  {
    fprintf(stderr, "sizeof(%s) is %zi; expected %zi\n", 
            name, real_size, intended_size);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv)
{
  FILE *config;
  uint16_t two_byte_int;
  uint8_t  *byte_array;

  check_size(uint8_t,  1);
  check_size(uint16_t, 2);
  check_size(uint32_t, 4);
  check_size(uint64_t, 8);

  config = fopen("config.h", "w");
  if (config == NULL)
  {
    fprintf(stderr, "Failed to open config.h\n");
    exit(EXIT_FAILURE);
  }

  put_string(config, "/* config.h: generated by configure.c */\n\n");

  /* either 0x00 0x01 or 0x01 0x00 depending on endianness */
  two_byte_int = 1;
  byte_array = (void *) &two_byte_int;

  if (byte_array[0] == 0x01)
  {
    put_string(config, "#define ENDIAN_LITTLE\n");
  }
  else
  {
    put_string(config, "#define ENDIAN_BIG\n");
  }

  fclose(config);

  exit(EXIT_SUCCESS);
}
