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

#include "hashtable.h"
#include "lookup_hash.h"

#define HASHTABLE_GET_ITEM 0
#define HASHTABLE_GET_DATA 1

const struct hashtablesettings hashtable_defaults = 
{
  /* size_initial         */ 3,
  /* size_maximum         */ 4,
  /* size_extend          */ 1,
  /* size_extend_trigger  */ 0,
  /* hashfunction         */ lookup_hash
};

static inline int hashtable_verify_settings(const struct hashtablesettings *s);
static inline int hashtable_resize(struct hashtable *ht, 
                                   ht_size_p_t new_size_p);
static inline void hashtable_insert(struct hashtable *ht, 
                                    struct hashtableitem *item);
static inline int hashtable_get_target(struct hashtable *ht,  
                                       const void *key, size_t keylen, 
                                       void **target, const int target_type);

int hashtable_new_custom(struct hashtable *ht, 
                         const struct hashtablesettings *s)
{
  int r;

  r = hashtable_verify_settings(s);
  if (r != HASHTABLE_SUCCESS)
  {
    return r;
  }

  ht->table              = NULL;
  ht->table_size_p       = 0;
  ht->table_size         = 0;
  ht->table_itemcount    = 0;
  ht->table_mask         = 0;
  ht->table_settings     = *s;

  /* If this fails now it won't have allocated any memory 
   * (this is not true for its later use in hashtable_set) */
  return hashtable_resize(ht, ht->table_settings.size_initial);
}

int hashtable_new(struct hashtable *ht)
{
  return hashtable_new_custom(ht, &hashtable_defaults);
}

int hashtable_verify_settings(const struct hashtablesettings *s)
{
  if (s->size_initial <= ht_size_lim_p && 
      s->size_maximum <= ht_size_lim_p && 
      s->size_extend <= ht_size_lim_p && 
      s->size_extend_trigger <= ht_size_lim_p && 
      s->size_extend != 0)
  {
    return HASHTABLE_SUCCESS;
  }
  else
  {
    return HASHTABLE_INVALID_ARG;
  }
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
  ht_hash_t hash;
  struct hashtableitem *j;

  hash = (ht->table_settings.hashfunction)(key, keylen);

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
          hashtable_get_item_data(ht, j, target);
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
  ht_size_p_t extend_trigger, extend;
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

  extend         = ht->table_size_p + ht->table_settings.size_extend;
  extend_trigger = ht->table_size_p + ht->table_settings.size_extend_trigger;

  if (extend <= ht->table_settings.size_maximum && 
      extend <= ht_size_lim_p && extend_trigger <= ht_size_lim_p && 
      ht->table_itemcount == (1 << extend_trigger))
  {
    i = hashtable_resize(ht, extend);
  }
  else
  {
    i = HASHTABLE_SUCCESS;
  }

  new_item = malloc(sizeof(struct hashtableitem));

  if (new_item == NULL)
  {
    return HASHTABLE_OUT_OF_MEMORY;
  }

  new_item->key      = key;
  new_item->keylen   = keylen;
  new_item->key_hash = (ht->table_settings.hashfunction)(key, keylen);
  new_item->data     = data;

  hashtable_insert(ht, new_item);

  (ht->table_itemcount)++;

  if (i == HASHTABLE_OUT_OF_MEMORY)
  {
    return HASHTABLE_SUCCESS_YET_OUT_OF_MEMORY;
  }
  else
  {
    return HASHTABLE_SUCCESS;
  }
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
  ht_size_t slot;

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
  ht_size_t slot;
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

  ht->table_size_p    = 0;
  ht->table_size      = 0;
  ht->table_itemcount = 0;
  ht->table_mask      = 0;
}

static inline int hashtable_resize(struct hashtable *ht, 
                                   ht_size_p_t new_size_p)
{
  struct hashtable temp;
  struct hashtableitem *i, *j;
  ht_size_t slot, old_size;

  temp.table_size_p = new_size_p;
  temp.table_size = 1 << temp.table_size_p;
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

  /* now update ht */
  old_size         = ht->table_size;
  ht->table        = temp.table;
  ht->table_size_p = temp.table_size_p;
  ht->table_size   = temp.table_size;
  ht->table_mask   = temp.table_mask;

  if (old_size != 0)
  {
    for (slot = 0; slot < old_size; slot++)
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

            if (j->next != NULL)
            {
              j->next->prev = NULL;
            }
          }
          else
          {
            j->prev->next = j->next;

            if (j->next != NULL)
            {
              j->next->prev = j->prev;
            }
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
  ht_size_t slot;
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
    case HASHTABLE_SUCCESS:                    return "Success";
    case HASHTABLE_SUCCESS_YET_OUT_OF_MEMORY:  return "Success yet out of memory";
    case HASHTABLE_OUT_OF_MEMORY:              return "Out of memory";
    case HASHTABLE_KEY_NOT_FOUND:              return "Key not found";
    case HASHTABLE_INVALID_ARG:                return "Invalid argument";
    case HASHTABLE_DUPLICATE:                  return "Duplicate key";
    default:                                   return "Success";
  }
}

