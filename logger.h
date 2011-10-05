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


#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <sys/time.h>           /* for timeval */

/* limit log entry size to 1 Kb */
#define LOGGER_MAX_ENTRY_SIZE (1024)

/* define MODULE_NAME before including logger.h */
#ifndef MODULE_NAME
#define MODULE_NAME __FILE__
#endif

#define LOG(...) logger_write(MODULE_NAME,LOG_ENTRY_NORMAL, __VA_ARGS__);
#define LOGINFO(...) logger_write(MODULE_NAME,LOG_ENTRY_INFO, __VA_ARGS__);
#define LOGWARN(...) logger_write(MODULE_NAME,LOG_ENTRY_WARNING, __VA_ARGS__);
#define LOGERROR(...) logger_write(MODULE_NAME,LOG_ENTRY_ERROR, __VA_ARGS__);


typedef enum 
{
  LOG_ENTRY_NORMAL = 0,
  LOG_ENTRY_ERROR = 1,
  LOG_ENTRY_WARNING = 2, 
  LOG_ENTRY_INFO = 3
} log_entry_type;

typedef enum
{
  LOG_LEVEL_ERRORS = 1,
  LOG_LEVEL_NORMAL = 2,
  LOG_LEVEL_ALL = 3
} log_level_type;

typedef enum
{
  LOG_FORMAT_SIMPLE = 0,
  LOG_FORMAT_TXT,
  LOG_FORMAT_XML,
  LOG_FORMAT_JSON,
  LOG_FORMAT_SEXP
} log_format_type;


typedef struct
{
  /* default: LOG_LEVEL_NORMAL */
  log_level_type log_level;
  /* default: LOG_FORMAT_SIMPLE */
  log_format_type log_format;
  const char* log_file_path;
  /*
   * rotate count for the log file
   * 0 - do not rotate(append to the end),
   * n > 0 - rotate n times
   * < 0 - always otherwrite
   * Default: 0
   */
  int log_rotate_count;
} logger_parameters;

typedef struct 
{
  const char* log_module_name;
  int log_entry_type;
  struct timeval log_tv;
  const char* log_message;
  void* log_thread;
} log_entry;


typedef void (*logger_backend_init_file)(FILE* file);
typedef void (*logger_backend_write_entry)(FILE* file,
                                           const log_entry* const entry);
typedef void (*logger_backend_fini_file)(FILE* file);




void logger_init();
void logger_init_with_loglevel(log_level_type log_level);
void logger_init_with_logname(const char* log_name);
void logger_init_with_params(const logger_parameters* params);
void logger_fini();

void logger_set_log_level(log_level_type log_level);

void logger_write(const char* name,int entry_type, const char* format, ...);

#ifndef LOGGER_REENTRANT
void logger_event_start(const char* name, int entry_type, const char* ev_name);
void logger_event_end(const char* name, int entry_type, const char* ev_name);
#endif


#endif /* _LOGGER_H_ */
