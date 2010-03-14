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

#ifndef MISC_FAILFUNC_HEADER
#define MISC_FAILFUNC_HEADER

#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"

void *malloc_f(size_t size);

void fprintf_f(FILE *stream, const char *format, ...);
void fwrite_f(const void *ptr, size_t size, size_t nmemb, FILE *stream);
void fputc_f(char c, FILE *stream);

/* NB: hashtable_get_f does not bail and exit() if the key is not found. */
void hashtable_new_custom_f(struct hashtable *ht, int size_exponent, 
                            hash_function f);
void hashtable_new_f(struct hashtable *ht, int size_exponent);
void hashtable_get_item_f(struct hashtable *ht, char *key, size_t keylen,
                          struct hashtableitem **item);
void hashtable_get_f(struct hashtable *ht, char *key, size_t keylen, 
                    void **data);
void hashtable_set_f(struct hashtable *ht, char *key, size_t keylen, 
                    void *data);
void hashtable_update_item_f(struct hashtable *ht, struct hashtableitem *item,
                             void *data);
void hashtable_update_f(struct hashtable *ht, char *key, size_t keylen,
                        void *data);
void hashtable_unset_item_f(struct hashtable *ht, struct hashtableitem *item);
void hashtable_unset_f(struct hashtable *ht, char *key, size_t keylen);

#define hashtable_delete_f hashtable_delete

#endif  /* MISC_FAILFUNC_HEADER */

