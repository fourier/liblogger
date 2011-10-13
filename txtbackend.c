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
#include <string.h>
#include <assert.h>
#include <time.h>

#include "txtbackend.h"


#define LOGGER_TIME_BUFFER_SIZE 64
#define LOGGER_MAX_HEADER_SIZE 512

#ifdef LOGGER_REENTRANT

#define HEADER_FORMAT_WARNING "[%s][%s][%p] /warning: %s\n"
#define HEADER_FORMAT_ERROR "[%s][%s][%p] /error: %s\n"
#define HEADER_FORMAT_NORMAL "[%s][%s][%p]: %s\n"
#define HEADER_FORMAT_INFO "[%s][%s][%p]: /info: %s\n"

#else  /* !LOGGER_REENTRANT */

#define HEADER_FORMAT_WARNING "[%s][%s] /warning: %s\n"
#define HEADER_FORMAT_ERROR "[%s][%s] /error: %s\n"
#define HEADER_FORMAT_NORMAL "[%s][%s]: %s\n"
#define HEADER_FORMAT_INFO "[%s][%s]: /info: %s\n"

#endif  /* LOGGER_REENTRANT */



void logger_txt_backend_init_file(FILE* file)
{
  fflush(file);
}

void logger_txt_backend_write_entry(FILE* file,
                                    const log_entry* const entry)
{
  const char* header_format;
  char time_buf[LOGGER_TIME_BUFFER_SIZE+1] = {0};
  struct tm tm;
  memmove(&tm, localtime(&entry->log_tv.tv_sec), sizeof(struct tm));

  snprintf(time_buf, LOGGER_TIME_BUFFER_SIZE,
           "%04d/%02d/%02d %02d:%02d:%02d.%03ld",
           tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour,
           tm.tm_min, tm.tm_sec, (long int)entry->log_tv.tv_usec/1000);
  
  switch(entry->log_entry_type)
  {
  case LOG_ENTRY_WARNING:
    header_format = HEADER_FORMAT_WARNING; break;
  case LOG_ENTRY_ERROR:
      header_format = HEADER_FORMAT_ERROR; break;
  case LOG_ENTRY_INFO:
      header_format = HEADER_FORMAT_INFO; break;
  case LOG_ENTRY_NORMAL:
  default:
      header_format = HEADER_FORMAT_NORMAL; break;
  }
  

#ifdef LOGGER_REENTRANT
  fprintf(file,
          header_format,
          entry->log_module_name,
          time_buf,
          entry->log_thread,
          entry->log_message);
#else
  fprintf(file,
          header_format,
          entry->log_module_name,
          time_buf,
          entry->log_message);
  
#endif
}

void logger_txt_backend_fini_file(FILE* file)
{
  fflush(file);
}
