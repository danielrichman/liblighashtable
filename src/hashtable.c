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
#include <stdint.h>
#include <string.h>

#include "failfunc.h"
#include "lookup_hash.h"

#define HASHTABLE_SIZE_EXPONENT_LIMIT  32
#define HASHTABLE_POOL_MAX_SIZE        1024

#define HASHTABLE_GET_ITEM 0
#define HASHTABLE_GET_DATA 1

static inline int hashtable_resize(struct hashtable *ht, int size_exponent);
static inline void hashtable_insert(struct hashtable *ht, 
                                    struct hashtableitem *item);
static inline int hashtable_get_target(struct hashtable *ht,  
                                       const void *key, size_t keylen, 
                                       void **target, const int target_type);

int hashtable_new_custom(struct hashtable *ht, int size_exponent,
                         hash_function f)
{
  if (size_exponent < 1 || size_exponent >= HASHTABLE_SIZE_EXPONENT_LIMIT)
  {
    return HASHTABLE_INVALID_ARG;
  }

  ht->table              = NULL;
  ht->table_size         = 0;
  ht->table_itemcount    = 0;
  ht->table_mask         = 0;
  ht->table_hashfunction = f;

  /* If this fails now it won't have allocated any memory 
   * (this is not true for its later use in hashtable_set) */
  return hashtable_resize(ht, 1 << size_exponent);
}

int hashtable_new(struct hashtable *ht, int size_exponent)
{
  return hashtable_new_custom(ht, size_exponent, lookup_hash);
}

int hashtable_get_item(struct hashtable *ht, const void *key, size_t keylen, 
                       struct hashtableitem **item)
{
  return hashtable_get_target(ht, key, keylen, (void **) item, 
                              HASHTABLE_GET_ITEM);
}

int hashtable_get(struct hashtable *ht, const void *key, size_t keylen, 
                  void **data)
{
  return hashtable_get_target(ht, key, keylen, (void **) data, 
                              HASHTABLE_GET_DATA);
}

static inline int hashtable_get_target(struct hashtable *ht,  
                                       const void *key, size_t keylen, 
                                       void **target, const int target_type)
{
  uint32_t hash;
  struct hashtableitem *j;

  hash = (ht->table_hashfunction)(key, keylen);

  j = (ht->table)[hash & ht->table_mask];

  while (j != NULL)
  {
    if (j->key_hash == hash &&
        j->keylen   == keylen &&
        memcmp(j->key, key, keylen) == 0)
    {
      if (target != NULL)
      {
        if (target_type == HASHTABLE_GET_ITEM)
        {
          *target = j;
        }
        else  /* HASHTABLE_GET_DATA */
        {
          *target = j->data;
        }
      }

      return HASHTABLE_SUCCESS;
    }

    j = j->next;
  }

  /* otherwise... */
  if (target != NULL)
  {
    *target = NULL;
  }

  return HASHTABLE_KEY_NOT_FOUND;
}

int hashtable_set(struct hashtable *ht, const void *key, size_t keylen,
                  void *data)
{
  int i, k;
  struct hashtableitem *new_item;

  k = hashtable_get(ht, key, keylen, NULL);

  if (k == HASHTABLE_SUCCESS)
  {
    return HASHTABLE_DUPLICATE;
  }
  else if (k != HASHTABLE_KEY_NOT_FOUND)
  {
    /* some other error; bail */
    return k;
  }

  if (ht->table_itemcount == ht->table_size)
  {
    i = hashtable_resize(ht, ht->table_size << 1);

    if (i != HASHTABLE_SUCCESS)
    {
      return i;
    }
  }

  new_item = malloc(sizeof(struct hashtableitem));

  new_item->key      = key;
  new_item->keylen   = keylen;
  new_item->key_hash = (ht->table_hashfunction)(key, keylen);
  new_item->data     = data;

  hashtable_insert(ht, new_item);

  (ht->table_itemcount)++;

  return HASHTABLE_SUCCESS;
}

int hashtable_update_item(struct hashtable *ht, struct hashtableitem *item,
                          void *data)
{
  item->data = data;
  return HASHTABLE_SUCCESS;
}

int hashtable_update(struct hashtable *ht, const void *key, size_t keylen, 
                     void *data)
{
  int i;
  struct hashtableitem *item;

  i = hashtable_get_item(ht, key, keylen, &item);

  if (i != HASHTABLE_SUCCESS)
  {
    return i;
  }

  return hashtable_update_item(ht, item, data);
}

int hashtable_unset_item(struct hashtable *ht, struct hashtableitem *item)
{
  uint32_t slot;

  if (item->prev == NULL)
  {
    slot = (item->key_hash & ht->table_mask);
    (ht->table)[slot] = item->next;
  }
  else
  {
    item->prev->next = item->next;
  }

  ht->table_itemcount--;

  free(item);

  return HASHTABLE_SUCCESS;
}

int hashtable_unset(struct hashtable *ht, const void *key, size_t keylen)
{
  int i;
  struct hashtableitem *item;

  i = hashtable_get_item(ht, key, keylen, &item);

  if (i != HASHTABLE_SUCCESS)
  {
    return i;
  }

  return hashtable_unset_item(ht, item);
}

void hashtable_delete(struct hashtable *ht)
{
  uint32_t slot;
  struct hashtableitem *i, *j;

  for (slot = 0; slot < ht->table_size; slot++)
  {
    j = (ht->table)[slot];

    while (j != NULL)
    {
      i = j->next;
      free(j);
      j = i;
    }
  }

  if (ht->table != NULL)
  {
    free(ht->table);
    ht->table = NULL;
  }

  ht->table_size      = 0;
  ht->table_itemcount = 0;
  ht->table_mask      = 0;
}

static inline int hashtable_resize(struct hashtable *ht, int new_size)
{
  struct hashtable temp;
  struct hashtableitem *i, *j;
  int need_update, slot;
  uint32_t old_size;

  if (new_size == 0)
  {
    /* then (ht->table_size << 1) must have overflown */
    return HASHTABLE_TOO_LARGE;
  }

  temp.table_size = new_size;
  temp.table_mask = temp.table_size - 1;

  /* if realloc fails, it leaves the original memory area untouched */
  temp.table = realloc(ht->table,
                       sizeof(struct hashtableitem *) * temp.table_size);

  if (temp.table == NULL)
  {
    /* therefore, we can still bail and keep the original table intact */
    return HASHTABLE_OUT_OF_MEMORY;
  }

  /* zero the new, uninitialised, area of memory */
  memset(temp.table + ht->table_size, 0, 
         (temp.table_size - ht->table_size) * sizeof(struct hashtableitem *));

  /* if this isn't the creation of the table, ie. it had a size before */
  need_update = (ht->table_size != 0);

  /* now update ht */
  old_size = ht->table_size;
  ht->table      = temp.table;
  ht->table_size = temp.table_size;
  ht->table_mask = temp.table_mask;

  if (old_size != 0)
  {
    for (slot = 0; slot < ht->table_size; slot++)
    {
      j = (ht->table)[slot];

      while (j != NULL)
      {
        if ((j->key_hash & ht->table_mask) != slot)
        {
          /* Save this before it's nulled. */
          i = j->next;

          if (j->prev == NULL)
          {
            (ht->table)[slot] = j->next;
          }
          else
          {
            j->prev->next = j->next;
          }

          hashtable_insert(ht, j);

          j = i;
        }
        else
        {
          j = j->next;
        }
      }
    }
  }

  return HASHTABLE_SUCCESS;
}

static inline void hashtable_insert(struct hashtable *ht,
                                   struct hashtableitem *item)
{
  uint32_t slot;
  struct hashtableitem *j;

  /* This item will become the last in the chain */
  item->next = NULL;

  slot = (item->key_hash & ht->table_mask);

  /* If there are no items in the slot, then pop it in there. Otherwise,
   * add it to the end of the chain of items */

  j = (ht->table)[slot];

  if (j == NULL)
  {
    (ht->table)[slot] = item;
    item->prev = NULL;
  }
  else
  {
    while (j->next != NULL)
    {
      j = j->next;
    }

    j->next = item;
    item->prev = j;
  }
}

const char *hashtable_strerror(int hterror)
{
  switch (hterror)
  {
    case HASHTABLE_SUCCESS:        return "Success";
    case HASHTABLE_OUT_OF_MEMORY:  return "Out of memory";
    case HASHTABLE_KEY_NOT_FOUND:  return "Key not found";
    case HASHTABLE_INVALID_ARG:    return "Invalid argument";
    case HASHTABLE_DUPLICATE:      return "Duplicate key";
    case HASHTABLE_TOO_LARGE:      return "Size limit reached";
    default:                       return "Success";
  }
}

