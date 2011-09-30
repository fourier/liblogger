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

#define LOGGER_SERVER_PORT 18999
#define LOGGER_MAX_ENTRY_SIZE (1024*512)

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
  LOG_FORMAT_NORMAL = 1
} log_format_type;
  

typedef struct
{
  log_level_type log_level;
  log_format_type log_format;
  const char* log_file_path;
  unsigned int log_rotate_count;
} logger_parameters;


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
