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

#include "jsonbackend.h"

static const char JSON_BEGIN[] = "{\n";
static const char JSON_END[] = "}";
static const char JSON_LOG_BEGIN[] = "    \"log\": {\n";
static const char JSON_LOG_END[] = "    }\n";
static const char JSON_RECORD_BEGIN[] = "        \"record\": {\n";
static const char JSON_RECORD_END[] =  "        }\n";
static const char JSON_DATE_BEGIN[] =  "            \"date\": ";
static const char JSON_DATE_END[] =  ",\n";
static const char JSON_MILLIS_BEGIN[] =  "            \"millis\": ";
static const char JSON_MILLIS_END[] =  ",\n";
static const char JSON_LOGGER_BEGIN[] =  "            \"logger\": ";
static const char JSON_LOGGER_END[] =  ",\n";
static const char JSON_LEVEL_BEGIN[] =   "            \"level\": ";
static const char JSON_LEVEL_END[] =   ",\n";
static const char JSON_THREAD_BEGIN[] =   "            \"thread\": ";
static const char JSON_THREAD_END[] =   ",\n";
static const char JSON_MESSAGE_BEGIN[] =   "            \"message\": ";
static const char JSON_MESSAGE_END[] =   "\n";

static void logger_json_write_date(FILE* file,
                             const log_entry* const entry)
{
  struct tm tm;
  memmove(&tm, localtime(&entry->log_tv.tv_sec), sizeof(struct tm));
  fwrite(JSON_DATE_BEGIN, sizeof(JSON_DATE_BEGIN)-1, 1, file);
  fprintf(file,
          "\"%04d-%02d-%02d %02d:%02d:%02d\"",
          tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
          tm.tm_hour, tm.tm_min, tm.tm_sec);
  fwrite(JSON_DATE_END, sizeof(JSON_DATE_END)-1, 1, file);
}

static void logger_json_write_millis(FILE* file,
                               const log_entry* const entry)
{
  unsigned long millis;
  fwrite(JSON_MILLIS_BEGIN, sizeof(JSON_MILLIS_BEGIN)-1, 1, file);
  millis = entry->log_tv.tv_sec*1000000+entry->log_tv.tv_usec/1000;
  fprintf(file, "%lu", millis);
  fwrite(JSON_MILLIS_END, sizeof(JSON_MILLIS_END)-1, 1, file);
}

static void logger_json_write_logger(FILE* file,
                               const log_entry* const entry)
{
  fwrite(JSON_LOGGER_BEGIN, sizeof(JSON_LOGGER_BEGIN)-1, 1, file);
  fprintf(file,"\"%s\"",entry->log_module_name);
  fwrite(JSON_LOGGER_END, sizeof(JSON_LOGGER_END)-1, 1, file);
}

static void logger_json_write_level(FILE* file,
                             const log_entry* const entry)
{
  fwrite(JSON_LEVEL_BEGIN, sizeof(JSON_LEVEL_BEGIN)-1, 1, file);
  switch(entry->log_entry_type)
  {
  case LOG_ENTRY_ERROR:
    fprintf(file, "\"%s\"", "ERROR"); break;
  case LOG_ENTRY_WARNING:
    fprintf(file, "\"%s\"", "WARNING"); break;
  case LOG_ENTRY_INFO:
    fprintf(file, "\"%s\"", "INFO"); break;
  case LOG_ENTRY_NORMAL:
  default:
    fprintf(file, "\"%s\"", "NORMAL"); break;
  }
  fwrite(JSON_LEVEL_END, sizeof(JSON_LEVEL_END)-1, 1, file);
}

#ifdef LOGGER_MT
static void logger_json_write_thread(FILE* file,
                               const log_entry* const entry)
{
  fwrite(JSON_THREAD_BEGIN, sizeof(JSON_THREAD_BEGIN)-1, 1,  file);
  fprintf(file,"%ld", (long)entry->log_thread);
  fwrite(JSON_THREAD_END, sizeof(JSON_THREAD_END)-1, 1, file);
}
#endif

static void logger_json_write_message(FILE* file,
                                const log_entry* const entry)
{
  fwrite(JSON_MESSAGE_BEGIN, sizeof(JSON_MESSAGE_BEGIN)-1, 1, file);
  fprintf(file,"\"%s\"", entry->log_message);
  fwrite(JSON_MESSAGE_END, sizeof(JSON_MESSAGE_END)-1, 1, file);
}

void logger_json_backend_init_file(FILE* file)
{
  fwrite(JSON_BEGIN, sizeof(JSON_BEGIN)-1, 1, file);
  fwrite(JSON_LOG_BEGIN, sizeof(JSON_LOG_BEGIN)-1, 1, file);
}


void logger_json_backend_write_entry(FILE* file,
                                    const log_entry* const entry)
{
  fwrite(JSON_RECORD_BEGIN, sizeof(JSON_RECORD_BEGIN)-1, 1, file);
  logger_json_write_date(file,entry);
  logger_json_write_millis(file,entry);
  logger_json_write_logger(file,entry);
  logger_json_write_level(file,entry);
#ifdef LOGGER_MT
  logger_json_write_thread(file,entry);
#endif
  logger_json_write_message(file,entry);
  fwrite(JSON_RECORD_END, sizeof(JSON_RECORD_END)-1, 1, file);
}

void logger_json_backend_fini_file(FILE* file)
{
  fwrite(JSON_LOG_END, sizeof(JSON_LOG_END)-1, 1, file);
  fwrite(JSON_END, sizeof(JSON_END)-1, 1, file);
}
