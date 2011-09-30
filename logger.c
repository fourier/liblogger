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
#include <stdarg.h>
#include <string.h>

#ifdef LOGGER_REENTRANT
#include <pthread.h>
#endif

#include "logger.h"
#include "rtclock.h"


#define LOGGER_MAX_HEADER_SIZE 512
#define LOGGER_TIME_BUFFER_SIZE 64

/* Compile-time configuration options */

#define HEADER_FORMAT_SIMPLE_WARNING "warning: %s\n"
#define HEADER_FORMAT_SIMPLE_ERROR "error: %s\n"
#define HEADER_FORMAT_SIMPLE_NORMAL "%s\n"
#define HEADER_FORMAT_SIMPLE_INFO "info: %s\n"

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

#ifndef LOGGER_REENTRANT
struct timespec logger_event_start_ev;
#endif 


/*
 * Globals
 */

/* Configuration values */
logger_parameters* logger_global_params = 0;


/* Log buffers */
char logger_log_header[LOGGER_MAX_HEADER_SIZE+1];
char logger_log_entry[LOGGER_MAX_ENTRY_SIZE+1];

/* Lock to prevent simultanious writing */
#ifdef LOGGER_REENTRANT
pthread_mutex_t logger_lock;
#endif

static const char* logger_strdup(const char* str)
{
  char* result = 0;
  int size;
  if (str)
  {
    size = strlen(str);
    result = malloc(size + 1);
    memcpy(result,str,size);
    result[size] = 0;
  }
  return result;
}


static void logger_process_log_entry(int size)
{
#ifndef LOGGER_OMIT_STDOUT
  /* portable_gettime(&ts1); */
  printf("%s",logger_log_entry);
  fflush(stdout);
  /* portable_gettime(&ts2); */
  /* printf("microseconds output: %f\n",(ts2.tv_nsec-ts1.tv_nsec)/1000.); */
#endif
  /* TODO: implement output to the file and logging backend */
}


static void logger_write_private(const char* name,
                                 int entry_type,
                                 struct timeval* tv,
                                 const char* format,
                                 va_list args)
{
  int size;
  const char* header_format;
  char time_buf[LOGGER_TIME_BUFFER_SIZE+1] = {0};
  struct tm tm;
  memmove(&tm, localtime(&tv->tv_sec), sizeof(struct tm));

  snprintf(time_buf, LOGGER_TIME_BUFFER_SIZE,
           "%04d/%02d/%02d %02d:%02d:%02d.%03d",
           tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour,
           tm.tm_min, tm.tm_sec, tv->tv_usec/1000);
  
  switch(entry_type)
  {
  case LOG_ENTRY_WARNING:
    switch(logger_global_params->log_format)
    {
    case LOG_FORMAT_SIMPLE:
      header_format = HEADER_FORMAT_SIMPLE_WARNING; break;
    case LOG_FORMAT_NORMAL:
    default:
      header_format = HEADER_FORMAT_WARNING; break;
    }
    break;
  case LOG_ENTRY_ERROR:
    switch(logger_global_params->log_format)
    {
    case LOG_FORMAT_SIMPLE:
      header_format = HEADER_FORMAT_SIMPLE_ERROR; break;
    case LOG_FORMAT_NORMAL:
    default:
      header_format = HEADER_FORMAT_ERROR; break;
    }
    break;
  case LOG_ENTRY_INFO:
    switch(logger_global_params->log_format)
    {
    case LOG_FORMAT_SIMPLE:
      header_format = HEADER_FORMAT_SIMPLE_INFO; break;
    case LOG_FORMAT_NORMAL:
    default:
      header_format = HEADER_FORMAT_INFO; break;
    }
    break;
  case LOG_ENTRY_NORMAL:
  default:
    switch(logger_global_params->log_format)
    {
    case LOG_FORMAT_SIMPLE:
      header_format = HEADER_FORMAT_SIMPLE_NORMAL; break;
    case LOG_FORMAT_NORMAL:
    default:
      header_format = HEADER_FORMAT_NORMAL; break;
    }
    break;
  }
  

  if (logger_global_params->log_format != LOG_FORMAT_SIMPLE)
#ifdef LOGGER_REENTRANT
  snprintf(logger_log_header,
           LOGGER_MAX_HEADER_SIZE,
           header_format,
           name,
           time_buf,
           (void*)pthread_self(),
           format);
#else
  snprintf(logger_log_header,
           LOGGER_MAX_HEADER_SIZE,
           header_format,
           name,
           time_buf,
           format);
#endif
  else
    snprintf(logger_log_header,
      LOGGER_MAX_HEADER_SIZE,
      header_format,
      format);
  
#ifdef LOGGER_REENTRANT
  pthread_mutex_lock(&logger_lock);
#endif
  size = vsnprintf(logger_log_entry,
                   LOGGER_MAX_ENTRY_SIZE,
                   logger_log_header,
                   args);
  logger_process_log_entry(size);
  
#ifdef LOGGER_REENTRANT
  pthread_mutex_unlock(&logger_lock);
#endif
}


static void logger_set_initial_time()
{
}


static void logger_init_private()
{
  logger_set_initial_time();
}


void logger_set_log_level(log_level_type log_level)
{
  if (!logger_global_params)
    logger_init_with_loglevel(log_level);
  else
    logger_global_params->log_level = log_level;
}


void logger_init()
{
  logger_init_with_loglevel(LOG_LEVEL_ALL);
}


void logger_init_with_loglevel(log_level_type log_level )
{
  logger_parameters params;
  memset(&params,0,sizeof(params));
  params.log_level = log_level;
  logger_init_with_params(&params);
}


void logger_init_with_logname(const char* log_name)
{
  logger_parameters params;
  memset(&params,0,sizeof(params));
  params.log_level = LOG_LEVEL_NORMAL;
  params.log_file_path = log_name;
  logger_init_with_params(&params);
}


void logger_init_with_params(const logger_parameters* params)
{
  if ( !logger_global_params )  /* avoid double initialization */
  {
    logger_global_params = calloc(sizeof(logger_parameters),1);
    logger_global_params->log_format = params->log_format;
    logger_global_params->log_level = params->log_level;
    logger_global_params->log_rotate_count = params->log_rotate_count;
    logger_global_params->log_file_path = params->log_file_path ?
      logger_strdup(params->log_file_path) : (const char*)0;
    
    logger_init_private();
  }
}


void logger_fini()
{
  if (logger_global_params)
  {
    if ( logger_global_params->log_file_path )
      free((char*)logger_global_params->log_file_path);
    free(logger_global_params);
  }
}


void logger_write(const char* name,int entry_type, const char* format, ...)
{
  struct timeval tv;
  if (!logger_global_params)
    logger_init();

  if (logger_global_params->log_format != LOG_FORMAT_SIMPLE)
    gettimeofday( &tv,0);

  if (entry_type == LOG_ENTRY_NORMAL ||
      entry_type <= (int)logger_global_params->log_level)
  {
    va_list vl;
    va_start(vl, format);
    logger_write_private(name, entry_type, &tv, format, vl);
    va_end(vl);
  }
}

#ifndef LOGGER_REENTRANT

void logger_event_start(const char* name, int entry_type, const char* ev_name)
{
  logger_write(name,entry_type,"Started: %s", ev_name);
  portable_gettime(&logger_event_start_ev);
}


void logger_event_end(const char* name, int entry_type, const char* ev_name)
{
  struct timespec ts;
  long diff_sec, diff_micsec;
  portable_gettime(&ts);
  diff_sec = ts.tv_sec - logger_event_start_ev.tv_sec;
  diff_micsec = (ts.tv_nsec - logger_event_start_ev.tv_nsec)/1000;
    
  if ( diff_sec)
    logger_write(name,entry_type,"Ended: %s, seconds: %ld, microseconds: %dl",
    ev_name, diff_sec, diff_micsec);
  else
    logger_write(name,entry_type,"Ended: %s, microseconds: %ld",
    ev_name, diff_micsec);
}
#endif

