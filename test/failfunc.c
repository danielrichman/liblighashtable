/*
    Copyright (C) 2008  Daniel Richman

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License, 
    see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "hashtable.h"

void *malloc_f(size_t size)
{
  char *r;
  r = malloc(size);

  if (r == NULL)
  {
    fprintf(stderr, "Out of memory: denied %li bytes of RAM\n", size);
    exit(EXIT_FAILURE);
  }

  return r;
}

void fprintf_f(FILE *stream, const char *format, ...)
{
  va_list args;
  int i;

  va_start(args, format);
  i = vfprintf(stream, format, args);
  va_end(args);

  if (i <= 0)
  {
    fprintf(stderr, "Error writing to output file\n");
    exit(EXIT_FAILURE);
  }
}

void fwrite_f(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  if (fwrite(ptr, size, nmemb, stream) != nmemb)
  {
    fprintf(stderr, "Error writing to output file \n");
    exit(EXIT_FAILURE);
  }
}

void fputc_f(char c, FILE *stream)
{
  if (fputc(c, stream) != c)
  {
    fprintf(stderr, "Error writing to output file\n");
    exit(EXIT_FAILURE);
  }
}

void hashtable_new_custom_f(struct hashtable *ht, int size_exponent, 
                            hash_function f)
{
  int r;
  r = hashtable_new_custom(ht, size_exponent, f);

  if (r != HASHTABLE_SUCCESS)
  {
    fprintf(stderr, "Error creating hashtable: %s\n", hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

void hashtable_new_f(struct hashtable *ht, int size_exponent)
{
  int r;
  r = hashtable_new(ht, size_exponent);

  if (r != HASHTABLE_SUCCESS)
  {
    fprintf(stderr, "Error creating hashtable: %s\n", hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

void hashtable_get_item_f(struct hashtable *ht, char *key, size_t keylen,
                          struct hashtableitem **item)
{
  int r;
  r = hashtable_get_item(ht, key, keylen, item);

  if (r != HASHTABLE_SUCCESS && r != HASHTABLE_KEY_NOT_FOUND)
  {
    fprintf(stderr, "Error while getting an item from a hashtable: %s\n",
                    hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

void hashtable_get_f(struct hashtable *ht, char *key, size_t keylen, 
                    void **data)
{
  int r;
  r = hashtable_get(ht, key, keylen, data);

  if (r != HASHTABLE_SUCCESS && r != HASHTABLE_KEY_NOT_FOUND)
  {
    fprintf(stderr, "Error while getting a value from a hashtable: %s\n", 
                    hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

void hashtable_set_f(struct hashtable *ht, char *key, size_t keylen, 
                     void *data)
{
  int r;
  r = hashtable_set(ht, key, keylen, data);

  if (r != HASHTABLE_SUCCESS)
  {
    fprintf(stderr, "Error while setting a key in a hashtable: %s\n",
                    hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

void hashtable_update_item_f(struct hashtable *ht, struct hashtableitem *item,
                             void *data)
{
  int r;
  r = hashtable_update_item(ht, item, data);

  if (r != HASHTABLE_SUCCESS)
  {
    fprintf(stderr, "Error while updating an item in a hashtable: %s\n",
                    hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

void hashtable_update_f(struct hashtable *ht, char *key, size_t keylen,
                        void *data)
{
  int r;
  r = hashtable_update(ht, key, keylen, data);

  if (r != HASHTABLE_SUCCESS)
  {
    fprintf(stderr, "Error while updating a key in a hashtable: %s\n",
                    hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

void hashtable_unset_item_f(struct hashtable *ht, struct hashtableitem *item)
{
  int r;
  r = hashtable_unset_item(ht, item);

  if (r != HASHTABLE_SUCCESS)
  {
    fprintf(stderr, "Error while unsetting an item in a hashtable: %s\n",
                    hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

void hashtable_unset_f(struct hashtable *ht, char *key, size_t keylen)
{
  int r;
  r = hashtable_unset(ht, key, keylen);

  if (r != HASHTABLE_SUCCESS) 
  {
    fprintf(stderr, "Error while unsetting a key in a hashtable: %s\n",
                    hashtable_strerror(r));
    exit(EXIT_FAILURE);
  }
}

