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
#include <stdarg.h>
#include <string.h>

#ifdef LOGGER_REENTRANT
#include <pthread.h>
#endif

#include "logger.h"
#include "rtclock.h"
#include "simplebackend.h"
#include "txtbackend.h"
#include "xmlbackend.h"
#include "sexpbackend.h"
#include "jsonbackend.h"


/* Compile-time configuration options */

#ifndef LOGGER_REENTRANT
struct timespec logger_event_start_ev;
#endif 

typedef struct 
{
  logger_parameters log_params;
  logger_backend_init_file log_init_file_fun;
  logger_backend_write_entry log_write_entry_fun;
  logger_backend_fini_file log_fini_file_fun;
} logger_parameters_private;


/*
 * Globals
 */

/* Configuration values */
logger_parameters_private* logger_global_params = 0;


/* output file */
FILE* logger_file = 0;

/* Lock to prevent simultanious writing */
#ifdef LOGGER_REENTRANT
pthread_mutex_t logger_lock = PTHREAD_MUTEX_INITIALIZER;
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


static void logger_process_log_entry(const log_entry* const entry)
{
  if (logger_file)
  {
    logger_global_params->log_write_entry_fun(logger_file, entry);
    fflush(logger_file);
  }
}


static void logger_set_initial_time()
{
}

static void logger_rotate_log(const char* filename, int rotate_count)
{
  int len = strlen(filename);
  char* name_from;
  char* name_to;
  if (rotate_count <= 0)
    return;
  len += 16;
  name_from = malloc(len);
  name_to   = malloc(len);
  len = rotate_count;
  while(rotate_count--)
  {
    if ( rotate_count )
      sprintf(name_from, "%s.%d",filename,rotate_count);
    else
      strcpy(name_from,filename);
    sprintf(name_to,"%s.%d",filename,rotate_count + 1);

    if (rotate_count+1 == len)
      remove(name_from);
    else
      rename(name_from,name_to);
  }
  free(name_from);
  free(name_to);
}

static void logger_open_file()
{
  if (!logger_global_params)
    logger_init();
  if (logger_global_params && logger_global_params->log_params.log_file_path)
  {
    if ( logger_global_params->log_params.log_rotate_count )
      logger_rotate_log(logger_global_params->log_params.log_file_path,
                        logger_global_params->log_params.log_rotate_count);
    if (logger_file)
    {
      fflush(logger_file);
      fclose(logger_file);
    }
    logger_file = fopen(logger_global_params->log_params.log_file_path,
                        (logger_global_params->log_params.log_rotate_count < 0 ?
                         "wt+" : "at"));
    if (!logger_file)
      fprintf(stderr,"Could not open %s for writing",
              logger_global_params->log_params.log_file_path);
    else
      logger_global_params->log_init_file_fun(logger_file);
  }
}

static void logger_close_file()
{
  if (logger_file)
  {
    logger_global_params->log_fini_file_fun(logger_file);
    fflush(logger_file);
    fclose(logger_file);
  }
}

static void logger_init_private()
{
  switch(logger_global_params->log_params.log_format)
  {
  case LOG_FORMAT_TXT:
    logger_global_params->log_init_file_fun = logger_txt_backend_init_file;
    logger_global_params->log_write_entry_fun = logger_txt_backend_write_entry; 
    logger_global_params->log_fini_file_fun = logger_txt_backend_fini_file;
    break;
  case LOG_FORMAT_SIMPLE:
    logger_global_params->log_init_file_fun = logger_simple_backend_init_file;
    logger_global_params->log_write_entry_fun = logger_simple_backend_write_entry; 
    logger_global_params->log_fini_file_fun = logger_simple_backend_fini_file;
    break;
  case LOG_FORMAT_XML:
    logger_global_params->log_init_file_fun = logger_xml_backend_init_file;
    logger_global_params->log_write_entry_fun = logger_xml_backend_write_entry; 
    logger_global_params->log_fini_file_fun = logger_xml_backend_fini_file;
    break;
  case LOG_FORMAT_JSON:
    logger_global_params->log_init_file_fun = logger_json_backend_init_file;
    logger_global_params->log_write_entry_fun = logger_json_backend_write_entry; 
    logger_global_params->log_fini_file_fun = logger_json_backend_fini_file;
    break;
  case LOG_FORMAT_SEXP:
    logger_global_params->log_init_file_fun = logger_sexp_backend_init_file;
    logger_global_params->log_write_entry_fun = logger_sexp_backend_write_entry; 
    logger_global_params->log_fini_file_fun = logger_sexp_backend_fini_file;
    break;  
  default:
    break;
  } 
  logger_set_initial_time();
  logger_open_file();
}


void logger_set_log_level(log_level_type log_level)
{
  if (!logger_global_params)
    logger_init_with_loglevel(log_level);
  else
    logger_global_params->log_params.log_level = log_level;
}


void logger_init()
{
  logger_init_with_loglevel(LOG_LEVEL_NORMAL);
}


void logger_init_with_loglevel(log_level_type log_level )
{
  logger_parameters params;
  memset(&params,0,sizeof(params));
  params.log_level = log_level;
  params.log_format = LOG_FORMAT_SIMPLE;
  params.log_rotate_count = 0;
  params.log_file_path = 0;
  logger_init_with_params(&params);
}


void logger_init_with_logname(const char* log_name)
{
  logger_parameters params;
  memset(&params,0,sizeof(params));
  params.log_level = LOG_LEVEL_NORMAL;
  params.log_format = LOG_FORMAT_SIMPLE;
  params.log_file_path = log_name;
  params.log_rotate_count = 0;
  logger_init_with_params(&params);
}


void logger_init_with_params(const logger_parameters* params)
{
  if ( !logger_global_params )  /* avoid double initialization */
  {
    logger_global_params = calloc(sizeof(logger_parameters_private),1);
    logger_global_params->log_params.log_format = params->log_format;
    logger_global_params->log_params.log_level = params->log_level;
    logger_global_params->log_params.log_rotate_count = params->log_rotate_count;
    logger_global_params->log_params.log_file_path = params->log_file_path ?
      logger_strdup(params->log_file_path) : (const char*)0;
    
    logger_init_private();
  }
}


void logger_fini()
{
  if (logger_global_params)
  {
    logger_close_file();
    if ( logger_global_params->log_params.log_file_path )
      free((char*)logger_global_params->log_params.log_file_path);
    free(logger_global_params);
  }

}


void logger_write(const char* name,int entry_type, const char* format, ...)
{
  struct timeval tv;
  log_entry entry;
  va_list vl;
  
  if (!logger_global_params)
    logger_init();

  if (logger_global_params->log_params.log_format != LOG_FORMAT_SIMPLE)
    gettimeofday( &tv,0);
 
  if (entry_type == LOG_ENTRY_NORMAL ||
      entry_type <= (int)logger_global_params->log_params.log_level)
  {
#ifdef LOGGER_REENTRANT
    entry.log_thread = (void*)pthread_self();
#endif
    entry.log_message = malloc(LOGGER_MAX_ENTRY_SIZE);

    va_start(vl, format);
    vsnprintf((char*)entry.log_message,LOGGER_MAX_ENTRY_SIZE-1,format,vl);
    va_end(vl);
    entry.log_module_name = name;
    entry.log_entry_type = entry_type;
    memcpy(&entry.log_tv, &tv, sizeof(tv));
#ifdef LOGGER_REENTRANT
    pthread_mutex_lock(&logger_lock);
#endif
    logger_process_log_entry(&entry);
#ifdef LOGGER_REENTRANT
    pthread_mutex_unlock(&logger_lock);
#endif
    free((char*)entry.log_message);
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
  if ( diff_micsec < 0)
  {
    diff_sec--;
    diff_micsec = 1000000+diff_micsec;
  }
  if ( diff_sec)
    logger_write(name,entry_type,"Ended: %s, seconds: %ld, microseconds: %ld",
                 ev_name, diff_sec, diff_micsec);
  else
    logger_write(name,entry_type,"Ended: %s, microseconds: %ld",
                 ev_name, diff_micsec);
}
#endif

