#    Copyright (C) 2008  Daniel Richman
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.

#    For a full copy of the GNU General Public License, 
#    see <http://www.gnu.org/licenses/>.

CC = gcc
override CFLAGS += -Wall -pedantic --std=gnu99 -D_GNU_SOURCE
GPERF = gperf

ifdef OPT
  override CFLAGS += -O2 -DNDEBUG
endif

ifdef BENCHMARK
  override CFLAGS += -pg -DBENCHMARK
endif

ifdef DEBUG
  override CFLAGS += -g -DVERBOSE
endif

ifdef INCLUDE_DIRS
  override CFLAGS += $(INCLUDE_DIRS)
endif

cfiles    := $(wildcard $(CFILE_SEARCH)) $(MISC_CFILES)
objects  := $(patsubst %.c,%.o,$(cfiles))
headers   := $(wildcard $(HEADER_SEARCH))

$(TARGETBINARY) : $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects)

%.o : %.c $(headers)
	$(CC) $(CFLAGS) -c -o $@ $<

clean : clean-objects
	rm -f $(TARGETBINARY)

clean-objects: 
	rm -f $(objects)

.DEFAULT_GOAL := $(TARGETBINARY)
