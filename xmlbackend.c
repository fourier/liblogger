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

#include "xmlbackend.h"

static const char XML_HEADER_STRING[] = "<?xml version=\"1.0\" encoding="
  "\"UTF-8\" standalone=\"no\"?>\n";
static const char XML_HEADER_DTD[] = "<!DOCTYPE log SYSTEM \"logger.dtd\">\n";
static const char XML_LOG_BEGIN[] = "<log>\n";
static const char XML_LOG_END[] = "</log>\n";
static const char XML_RECORD_BEGIN[] = "<record>\n";
static const char XML_RECORD_END[] =  "</record>\n";
static const char XML_DATE_BEGIN[] =  "  <date>";
static const char XML_DATE_END[] =  "</date>\n";
static const char XML_MILLIS_BEGIN[] =  "  <millis>";
static const char XML_MILLIS_END[] =  "</millis>\n";
static const char XML_LOGGER_BEGIN[] =  "  <logger>";
static const char XML_LOGGER_END[] =  "</logger>\n";
static const char XML_LEVEL_BEGIN[] =   "  <level>";
static const char XML_LEVEL_END[] =   "</level>\n";
static const char XML_THREAD_BEGIN[] =   "  <thread>";
static const char XML_THREAD_END[] =   "</thread>\n";
static const char XML_MESSAGE_BEGIN[] =   "  <message>";
static const char XML_MESSAGE_END[] =   "</message>\n";

static void logger_xml_write_date(FILE* file,
                             const log_entry* const entry)
{
  struct tm tm;
  memmove(&tm, localtime(&entry->log_tv.tv_sec), sizeof(struct tm));
  fwrite(XML_DATE_BEGIN, sizeof(XML_DATE_BEGIN)-1, 1, file);
  fprintf(file,
          "%04d-%02d-%02d %02d:%02d:%02d",
          tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
          tm.tm_hour, tm.tm_min, tm.tm_sec);
  fwrite(XML_DATE_END, sizeof(XML_DATE_END)-1, 1, file);
}

static void logger_xml_write_millis(FILE* file,
                               const log_entry* const entry)
{
  unsigned long millis;
  fwrite(XML_MILLIS_BEGIN, sizeof(XML_MILLIS_BEGIN)-1, 1, file);
  millis = entry->log_tv.tv_sec*1000000+entry->log_tv.tv_usec/1000;
  fprintf(file, "%lu", millis);
  fwrite(XML_MILLIS_END, sizeof(XML_MILLIS_END)-1, 1, file);
}

static void logger_xml_write_logger(FILE* file,
                               const log_entry* const entry)
{
  fwrite(XML_LOGGER_BEGIN, sizeof(XML_LOGGER_BEGIN)-1, 1, file);
  fwrite(entry->log_module_name, 1, strlen(entry->log_module_name), file);
  fwrite(XML_LOGGER_END, sizeof(XML_LOGGER_END)-1, 1, file);
}

static void logger_xml_write_level(FILE* file,
                             const log_entry* const entry)
{
  fwrite(XML_LEVEL_BEGIN, sizeof(XML_LEVEL_BEGIN)-1, 1, file);
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
  fwrite(XML_LEVEL_END, sizeof(XML_LEVEL_END)-1, 1, file);
}

#ifdef LOGGER_MT
static void logger_xml_write_thread(FILE* file,
                               const log_entry* const entry)
{
  fwrite(XML_THREAD_BEGIN, sizeof(XML_THREAD_BEGIN)-1, 1,  file);
  fprintf(file,"%ld", (long)entry->log_thread);
  fwrite(XML_THREAD_END, sizeof(XML_THREAD_END)-1, 1, file);
}
#endif

static void logger_xml_write_message(FILE* file,
                                const log_entry* const entry)
{
  fwrite(XML_MESSAGE_BEGIN, sizeof(XML_MESSAGE_BEGIN)-1, 1, file);
  fwrite(entry->log_message,strlen(entry->log_message), 1, file);
  fwrite(XML_MESSAGE_END, sizeof(XML_MESSAGE_END)-1, 1, file);
}

void logger_xml_backend_init_file(FILE* file)
{
  fwrite(XML_HEADER_STRING, sizeof(XML_HEADER_STRING)-1, 1, file);
  fwrite(XML_HEADER_DTD, sizeof(XML_HEADER_DTD)-1, 1, file);
  fwrite(XML_LOG_BEGIN, sizeof(XML_LOG_BEGIN)-1, 1, file);
}


void logger_xml_backend_write_entry(FILE* file,
                                    const log_entry* const entry)
{
  fwrite(XML_RECORD_BEGIN, sizeof(XML_RECORD_BEGIN)-1, 1, file);
  logger_xml_write_date(file,entry);
  logger_xml_write_millis(file,entry);
  logger_xml_write_logger(file,entry);
  logger_xml_write_level(file,entry);
#ifdef LOGGER_MT
  logger_xml_write_thread(file,entry);
#endif
  logger_xml_write_message(file,entry);
  fwrite(XML_RECORD_END, sizeof(XML_RECORD_END)-1, 1, file);
}

void logger_xml_backend_fini_file(FILE* file)
{
  fwrite(XML_LOG_END, sizeof(XML_LOG_END)-1, 1, file);  
}
