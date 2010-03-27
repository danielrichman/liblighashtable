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

TEST_BINARY    = ./ht_test
TARGET_LIBRARY = liblighashtable
SRC_DIR        = src
TEST_DIR       = test

CC = gcc
AR = ar
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

src_cfiles      := $(wildcard $(SRC_DIR)/*.c)
test_cfiles     := $(wildcard $(TEST_DIR)/*.c)
src_headers     := $(wildcard $(SRC_DIR)/*.h) config.h
test_headers    := $(wildcard $(TEST_DIR)/*.h)
src_objects     := $(patsubst %.c,%.o,$(src_cfiles))
src_pic_objects := $(patsubst %.c,%.pic.o,$(src_cfiles))
test_objects    := $(patsubst %.c,%.o,$(test_cfiles))

all : $(TARGET_LIBRARY).a $(TARGET_LIBRARY).so test

test : $(TEST_BINARY)
	$(TEST_BINARY)

clean : clean-objects
	rm -f $(TARGET_LIBRARY).so $(TARGET_LIBRARY).a $(TEST_BINARY) 
	rm -f config.h configure
	rm -f gmon.out

clean-objects: 
	rm -f $(src_objects) $(src_pic_objects) $(test_objects)

config.h : ./configure
	./configure

configure : configure.c
	$(CC) -o $@ $<

$(SRC_DIR)/%.o : src/%.c $(src_headers)
	$(CC) $(CFLAGS) -I. -I$(SRC_DIR) -c -o $@ $<

$(SRC_DIR)/%.pic.o : src/%.c $(src_headers)
	$(CC) $(CFLAGS) -I. -I$(SRC_DIR) -fPIC -c -o $@ $<

$(TEST_DIR)/%.o : test/%.c $(test_headers) $(src_headers)
	$(CC) $(CFLAGS) -I. -I$(TEST_DIR) -I$(SRC_DIR) -c -o $@ $<

$(TEST_BINARY) : $(test_objects) $(src_objects)
	$(CC) $(CFLAGS) -o $@ $(test_objects) $(src_objects)

$(TARGET_LIBRARY).so : $(src_pic_objects)
	$(CC) $(CFLAGS) -shared -o $@ $(src_pic_objects)

$(TARGET_LIBRARY).a : $(src_objects)
	$(AR) rcs $@ $(src_objects)

.PHONY : all test clean clean-objects
.DEFAULT_GOAL := all
