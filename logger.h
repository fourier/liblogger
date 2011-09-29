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
  LOG_ENTRY_ERROR = 0,
  LOG_ENTRY_WARNING = 1, 
  LOG_ENTRY_NORMAL = 2,
  LOG_ENTRY_INFO = 3
} log_entry_type;

typedef enum
{
  LOG_LEVEL_ERRORS = 0,
  LOG_LEVEL_WARNINGS = 1,
  LOG_LEVEL_NORMAL = 2,
  LOG_LEVEL_ALL = 3
} log_level_type;

void logger_set_log_level(int level);
void logger_init();
void logger_fini();
void logger_write(const char* name,int entry_type, const char* format, ...);

#endif /* _LOGGER_H_ */
