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

#ifndef HASHTABLE_HEADER
#define HASHTABLE_HEADER

#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef uint32_t (*hash_function)(const void *key, size_t length);

struct hashtableitem
{
  const void *key;
  size_t keylen;
  uint32_t key_hash;
  void *data;
  struct hashtableitem *next;
  struct hashtableitem *prev;
};

struct hashtable
{
  struct hashtableitem **table;
  uint32_t table_size;
  uint32_t table_itemcount;
  uint32_t table_mask;
  hash_function table_hashfunction;
};

int hashtable_new_custom(struct hashtable *ht, int size_exponent, 
                         hash_function f);
int hashtable_new(struct hashtable *ht, int size_exponent);
int hashtable_get_item(struct hashtable *ht, const void *key, size_t keylen, 
                       struct hashtableitem **item);
int hashtable_get(struct hashtable *ht, const void *key, size_t keylen, 
                  void **data);
int hashtable_set(struct hashtable *ht, const void *key, size_t keylen, 
                  void *data);
int hashtable_update(struct hashtable *ht, const void *key, size_t keylen, 
                     void *data);
int hashtable_unset_item(struct hashtable *ht, struct hashtableitem *item);
int hashtable_unset(struct hashtable *ht, const void *key, size_t keylen);
void hashtable_delete(struct hashtable *ht);

#define HASHTABLE_SUCCESS         0
#define HASHTABLE_OUT_OF_MEMORY   1
#define HASHTABLE_KEY_NOT_FOUND   2
#define HASHTABLE_INVALID_ARG     3
#define HASHTABLE_DUPLICATE       4
#define HASHTABLE_TOO_LARGE       5

/* These functions are so simple that they should be macros. 
 *
 *  int hashtable_get_item_data(struct hashtable *ht,
 *                              struct hashtableitem *item,
 *                              void **data);
 *
 *  int hashtable_update_item(struct hashtable *ht, struct hashtableitem *item, 
 *                            void **data);
 *
 * The ", HASHTABLE_SUCCESS" part makes them behave as if they were a 
 * function that returned SUCCESS. */
#define hashtable_get_item_data(ht, item, d)   \
  (*d = item->data, HASHTABLE_SUCCESS)
#define hashtable_update_item(ht, item, d)     \
  (item->data = d, HASHTABLE_SUCCESS)

const char *hashtable_strerror(int hterror);

#endif  /* HASHTABLE_HEADER */

