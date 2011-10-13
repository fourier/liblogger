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

#include "sexpbackend.h"

static const char SEXP_HEADER_STRING[] = ";; -*- Mode: lisp; -*-\n";
static const char SEXP_LOG_BEGIN[] = "(log";
static const char SEXP_LOG_END[] = ")\n";
static const char SEXP_RECORD_BEGIN[] = "\n (record ";
static const char SEXP_RECORD_END[] =  ")";
static const char SEXP_DATE_BEGIN[] =  "\n  (date ";
static const char SEXP_DATE_END[] =  ")";
static const char SEXP_MILLIS_BEGIN[] =  "\n  (millis ";
static const char SEXP_MILLIS_END[] =  ")";
static const char SEXP_LOGGER_BEGIN[] =  "\n  (logger ";
static const char SEXP_LOGGER_END[] =  ")";
static const char SEXP_LEVEL_BEGIN[] =   "\n  (level ";
static const char SEXP_LEVEL_END[] =   ")";
static const char SEXP_THREAD_BEGIN[] =   "\n  (thread ";
static const char SEXP_THREAD_END[] =   ")";
static const char SEXP_MESSAGE_BEGIN[] =   "\n  (message ";
static const char SEXP_MESSAGE_END[] =   ")";

static void logger_sexp_write_date(FILE* file,
                             const log_entry* const entry)
{
  struct tm tm;
  memmove(&tm, localtime(&entry->log_tv.tv_sec), sizeof(struct tm));
  fwrite(SEXP_DATE_BEGIN, sizeof(SEXP_DATE_BEGIN)-1, 1, file);
  fprintf(file,
          "\"%04d-%02d-%02d %02d:%02d:%02d\"",
          tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
          tm.tm_hour, tm.tm_min, tm.tm_sec);
  fwrite(SEXP_DATE_END, sizeof(SEXP_DATE_END)-1, 1, file);
}

static void logger_sexp_write_millis(FILE* file,
                               const log_entry* const entry)
{
  unsigned long millis;
  fwrite(SEXP_MILLIS_BEGIN, sizeof(SEXP_MILLIS_BEGIN)-1, 1, file);
  millis = entry->log_tv.tv_sec*1000000+entry->log_tv.tv_usec/1000;
  fprintf(file, "%lu", millis);
  fwrite(SEXP_MILLIS_END, sizeof(SEXP_MILLIS_END)-1, 1, file);
}

static void logger_sexp_write_logger(FILE* file,
                               const log_entry* const entry)
{
  fwrite(SEXP_LOGGER_BEGIN, sizeof(SEXP_LOGGER_BEGIN)-1, 1, file);
  fprintf(file, "\"%s\"", entry->log_module_name);
  fwrite(SEXP_LOGGER_END, sizeof(SEXP_LOGGER_END)-1, 1, file);
}

static void logger_sexp_write_level(FILE* file,
                             const log_entry* const entry)
{
  fwrite(SEXP_LEVEL_BEGIN, sizeof(SEXP_LEVEL_BEGIN)-1, 1, file);
  switch(entry->log_entry_type)
  {
  case LOG_ENTRY_ERROR:
    fprintf(file, "%s", "ERROR"); break;
  case LOG_ENTRY_WARNING:
    fprintf(file, "%s", "WARNING"); break;
  case LOG_ENTRY_INFO:
    fprintf(file, "%s", "INFO"); break;
  case LOG_ENTRY_NORMAL:
  default:
    fprintf(file, "%s", "NORMAL"); break;
  }
  fwrite(SEXP_LEVEL_END, sizeof(SEXP_LEVEL_END)-1, 1, file);
}

#ifdef LOGGER_REENTRANT
static void logger_sexp_write_thread(FILE* file,
                               const log_entry* const entry)
{
  fwrite(SEXP_THREAD_BEGIN, sizeof(SEXP_THREAD_BEGIN)-1, 1,  file);
  fprintf(file,"%ld", (long)entry->log_thread);
  fwrite(SEXP_THREAD_END, sizeof(SEXP_THREAD_END)-1, 1, file);
}
#endif

static void logger_sexp_write_message(FILE* file,
                                const log_entry* const entry)
{
  fwrite(SEXP_MESSAGE_BEGIN, sizeof(SEXP_MESSAGE_BEGIN)-1, 1, file);
  fprintf(file, "\"%s\"", entry->log_message);
  fwrite(SEXP_MESSAGE_END, sizeof(SEXP_MESSAGE_END)-1, 1, file);
}

void logger_sexp_backend_init_file(FILE* file)
{
  fwrite(SEXP_HEADER_STRING, sizeof(SEXP_HEADER_STRING)-1, 1, file);
  fwrite(SEXP_LOG_BEGIN, sizeof(SEXP_LOG_BEGIN)-1, 1, file);
}


void logger_sexp_backend_write_entry(FILE* file,
                                    const log_entry* const entry)
{
  fwrite(SEXP_RECORD_BEGIN, sizeof(SEXP_RECORD_BEGIN)-1, 1, file);
  logger_sexp_write_date(file,entry);
  logger_sexp_write_millis(file,entry);
  logger_sexp_write_logger(file,entry);
  logger_sexp_write_level(file,entry);
#ifdef LOGGER_REENTRANT
  logger_sexp_write_thread(file,entry);
#endif
  logger_sexp_write_message(file,entry);
  fwrite(SEXP_RECORD_END, sizeof(SEXP_RECORD_END)-1, 1, file);
}

void logger_sexp_backend_fini_file(FILE* file)
{
  fwrite(SEXP_LOG_END, sizeof(SEXP_LOG_END)-1, 1, file);  
}
