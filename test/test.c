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
#include <string.h>

#ifdef BENCHMARK
  #ifndef NDEBUG
    #define NDEBUG
  #endif

  #undef VERBOSE

  #if BENCHMARK == 1
    #undef BENCHMARK
    #define BENCHMARK 0x2FFFFF
  #endif
#endif

#ifndef NDEBUG
  #include <stdarg.h>
#endif

#include "failfunc.h"
#include "lookup_hash.h"

static inline void debug_printf(const char *format, ...);
static inline void debug_ht(struct hashtable *ht);

static inline void debug_printf(const char *format, ...)
{
#ifndef NDEBUG
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
#endif
}

static inline void debug_ht(struct hashtable *ht)
{
#ifdef VERBOSE
  ht_size_t i;
  struct hashtableitem *j;

  debug_printf("{\n");
  debug_printf("  table = %p\n", ht->table);
  debug_printf("  {\n");

  for (i = 0; i < ht->table_size; i++)
  {
    debug_printf("    [0x%08x %10i] = %p\n", i, i, ht->table[i]);

    if (ht->table[i] != NULL)
    {
      j = ht->table[i];

      debug_printf("    {\n");
      while (j != NULL)
      {
        debug_printf("      -- key = '%.*s', keylen = %zu, \n"
                     "         key_hash = 0x%08x, data = %p, \n"
                     "         next = %p, prev = %p \n",
                     j->keylen, j->key, j->keylen,
                     j->key_hash, j->data, 
                     j->next, j->prev);
        j = j->next;
      }
      debug_printf("    }\n");
    }
  }

  debug_printf("  }\n");

  debug_printf("  table_size_p = %i, table_size = %i, \n"
               "  table_itemcount = %i, table_mask = 0x%08x\n",
               ht->table_size_p, ht->table_size, ht->table_itemcount, 
               ht->table_mask);

  debug_printf("  table_settings = \n");
  debug_printf("  {\n");

  debug_printf("    size_initial = %i, size_maximum = %i, \n"
               "    size_extend = %i, size_extend_trigger = %i, \n"
               "    hashfunction = %p \n",
               ht->table_settings.size_initial,
               ht->table_settings.size_maximum,
               ht->table_settings.size_extend,
               ht->table_settings.size_extend_trigger,
               ht->table_settings.hashfunction);

  debug_printf("  }\n");
  debug_printf("}\n");
#endif
}

char *testkeys[10] = 
 { "yki3coJRshu4ohKu", "aevaeiaevae1Fi", "Si12341evae7nohT2thai", 
   "zae6ajklfsdoY6eQuie8uph", "FieVe1giaX7ahkor", "ahtecBa", "hayawe84", 
   "iet0ooGh uTee1aiz aeShou0H Aepie5ma fie0Nool", "beemoh8Ooh6AiD",
   "Moo" };
#define testkey_count  (sizeof(testkeys) / sizeof(char *))

/* hopefully so that testkey[4] and getkey arn't the same memory location,
 * so it's a proper test. This string is testkey[4] + testkey[8] */
char *getkey = "FieVe1giaX7ahkorbeemoh8Ooh6AiD";

static inline void report_gettest(char *a, char *i)
{
  if (a == i)
  {
    debug_printf("Success\n");
  }
  else if (a == NULL)
  {
    debug_printf("Failure (not found)\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    #ifdef VERBOSE
      debug_printf("Failure (returned %p, not %p)\n", a, i);
    #else
      debug_printf("Failure (incorrect match)\n");
    #endif

    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv)
{
  #ifdef BENCHMARK
  int x, j;
  char *c;
  #else
  char *a, *b, *c;
  void *t;
  #endif

  struct hashtable ht;
  struct hashtablesettings s;
  int i, k;
  struct hashtableitem *l;
  char d[10];
  int testkey_lens[testkey_count];

  debug_printf("Have %i testkeys\n", testkey_count);

  for (k = 0; k < testkey_count; k++)
  {
    testkey_lens[k] = strlen(testkeys[k]);
  }

  #ifdef BENCHMARK
  for (x = 0; x < BENCHMARK; x++) {
  #endif

  s.size_initial = 0;
  s.size_maximum = 3;
  s.size_extend = 1;
  s.size_extend_trigger = 0;
  s.hashfunction = lookup_hash;

  debug_printf("Creating new hashtable size 2^3 = 8: ");
  hashtable_new_custom_f(&ht, &s);
  debug_printf("Done\n");
  debug_ht(&ht);

  s.size_initial = 255;

  debug_printf("Adding 7 items: ");
  #ifdef VERBOSE
    debug_printf("\n");
  #endif
  for (i = 0; i < 7; i++)
  {
    hashtable_set_f(&ht, testkeys[i], testkey_lens[i], &(d[i]));
    #ifdef VERBOSE
      debug_printf("  [%10u] = '%s'\n", i, testkeys[i]);
    #endif
    debug_ht(&ht);
  }
  debug_printf("Done\n");

  debug_printf("Adding the 8th item: ");
  hashtable_set_f(&ht, testkeys[7], testkey_lens[7], &(d[7]));
  debug_printf("Done\n");
  debug_ht(&ht);

  debug_printf("Adding the 9th item: ");
  hashtable_set_f(&ht, testkeys[8], testkey_lens[8], &(d[8]));
  debug_printf("Done\n");
  debug_ht(&ht);

  debug_printf("Attempting to get a pointer to the 8th item: ");
  hashtable_get_item_f(&ht, testkeys[7], testkey_lens[7], &l);

  if (l == NULL)
  {
    debug_printf("Failed\n");
    exit(EXIT_FAILURE);
  }
  else
  {
    debug_printf("Success\n");
  }

  debug_printf("Updating the 8th item via pointer: ");
  hashtable_update_item_f(&ht, l, &(d[2]));
  debug_printf("Done\n");
  debug_ht(&ht);

  #ifndef BENCHMARK
  t = l->data;
  #endif

  debug_printf("Updating the 5th item: ");
  hashtable_update_f(&ht, getkey, testkey_lens[4], &(d[3]));
  debug_printf("Done\n");
  debug_ht(&ht);

  debug_printf("Unsetting the 8th item via pointer: ");
  hashtable_unset_item_f(&ht, l);
  debug_printf("Done\n");
  debug_ht(&ht);

  debug_printf("Unsetting the 1st item: ");
  hashtable_unset_f(&ht, testkeys[0], testkey_lens[0]);
  debug_printf("Done\n");
  debug_ht(&ht);

  #ifndef BENCHMARK
  debug_printf("Tests: ");

  if (t != &(d[2]))
  {
    debug_printf("Error: Update to the 8th item failed.\n");
    exit(EXIT_FAILURE);
  }

  if (ht.table_itemcount != 7)
  {
    debug_printf("Error: table_itemcount incorrect\n");
  }

  if (hashtable_get(&ht, testkeys[0], testkey_lens[0], NULL) != 
                                               HASHTABLE_KEY_NOT_FOUND)
  {
    debug_printf("Error: 1st item still exists\n");
  }

  debug_printf("Ok\n");

  debug_printf("Attempting to get:\n");

  debug_printf(" The fifth item: ");
  hashtable_get_f(&ht, getkey, testkey_lens[4], (void **) &a);
  report_gettest(a, &(d[3]));

  debug_printf(" The ninth item: ");
  hashtable_get_f(&ht, getkey + testkey_lens[4], testkey_lens[8], 
                  (void **) &b);
  report_gettest(b, &(d[8]));

  debug_printf(" A nonexistant item (8th): ");
  hashtable_get_f(&ht, testkeys[7], testkey_lens[7], (void **) &c);
  report_gettest(c, NULL);

  debug_ht(&ht);
  #else
  for (j = 0; j < 7; j++)
  {
    hashtable_get_f(&ht, testkeys[j], testkey_lens[j], (void **) &c);
  }
  #endif

  debug_printf("Destroying the table: ");
  hashtable_delete(&ht);
  debug_printf("Done\n");
  debug_ht(&ht);

  #ifdef BENCHMARK
  }
  #endif

  exit(EXIT_SUCCESS);
}
