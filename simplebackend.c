/* -*- Mode: C; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
  Copyright (C) 2011 Alexey Veretennikov (alexey dot veretennikov at gmail.com)
 
  This file is part of liblogger.

  liblogger is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  liblogger is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with liblogger.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

#include "simplebackend.h"

#define HEADER_FORMAT_SIMPLE_WARNING "warning: %s\n"
#define HEADER_FORMAT_SIMPLE_ERROR "error: %s\n"
#define HEADER_FORMAT_SIMPLE_NORMAL "%s\n"
#define HEADER_FORMAT_SIMPLE_INFO "info: %s\n"


void logger_simple_backend_init_file(FILE* file)
{
  fflush(file);
}

void logger_simple_backend_write_entry(FILE* file,
                                       const log_entry* const entry)
{
  const char* header_format;
  switch(entry->log_entry_type)
  {
  case LOG_ENTRY_WARNING:
      header_format = HEADER_FORMAT_SIMPLE_WARNING; break;
  case LOG_ENTRY_ERROR:
      header_format = HEADER_FORMAT_SIMPLE_ERROR; break;
  case LOG_ENTRY_INFO:
      header_format = HEADER_FORMAT_SIMPLE_INFO; break;
  case LOG_ENTRY_NORMAL:
  default:
      header_format = HEADER_FORMAT_SIMPLE_NORMAL; break;
  }
  fprintf(file, header_format, entry->log_message);
}

void logger_simple_backend_fini_file(FILE* file)
{
  fflush(file);
}
