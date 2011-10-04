# -*- Mode: makefile; -*-

# Copyright (C) 2011 Alexey Veretennikov (alexey dot veretennikov at gmail.com)
# 
#	This file is part of liblogger.
#
# liblogger is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# liblogger is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with liblogger.  If not, see <http://www.gnu.org/licenses/>.


CC = gcc

CFLAGS = -ggdb -g -std=c99 -pedantic -Wall -Wextra -Wswitch-default -Wswitch-enum -Wdeclaration-after-statement -Wmissing-declarations
#-DLOGGER_REENTRANT
#-DLOGGER_OMIT_STDOUT
INCLUDES = -I .
LINKFLAGS = -L.
#-lrt

SRC_TEST = main.c
SRC_LIB = logger.c rtclock.c simplebackend.c txtbackend.c xmlbackend.c sexpbackend.c

HEADERS := $(wildcard *.h)
OBJECTS_LIB := $(patsubst %.c,%.o,$(SRC_LIB))
OBJECTS_TEST := $(patsubst %.c,%.o,$(SRC_TEST))
OBJECTS := $(patsubst %.c,%.o,$(SRC_TEST) $(SRC_LIB))

OUTPUT_TEST = loggertest
OUTPUT_LIB = liblogger.a

all: $(OBJECTS) $(OUTPUT_LIB) $(OUTPUT_TEST)

%.o : %.c %.h
	$(CC) -c $(CFLAGS) $(DEFINES) $(INCLUDES) $< -o $@

$(OUTPUT_TEST): $(HEADERS) $(OBJECTS) $(OUTPUT_LIB) 
	$(CC) $(OBJECTS_TEST) -o $(OUTPUT_TEST) $(LINKFLAGS) -llogger

$(OUTPUT_LIB): $(HEADERS) $(OBJECTS) $ $(OBJECTS_LIB)
	$(RM) -f $(OUTPUT_LIB)
	$(AR) cr $(OUTPUT_LIB) $(OBJECTS_LIB)
	ranlib $(OUTPUT_LIB)

lint:
	splint *.c

.PHONY : clean
clean :
	$(RM) $(OBJECTS_LIB)  $(OBJECTS_TEST)
	$(RM)	$(OUTPUT_LIB) $(OUTPUT_TEST) $(OUTPUT_DAEMON)

check-syntax: 
	gcc -o nul -S ${CHK_SOURCES} 

