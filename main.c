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

#include "logger.h"

int main(int argc, char *argv[])
{
  logger_parameters params;
  params.log_level = LOG_LEVEL_NORMAL;
  params.log_format = LOG_FORMAT_SEXP;
  if ( argc > 1)
  {
    params.log_file_path = argv[1];
    params.log_rotate_count = argc > 2 ? atoi(argv[2]) : 0;
    logger_init_with_params(&params);
  }
  else
    logger_init();
  printf("Log level ALL\n");
  logger_set_log_level(LOG_LEVEL_ALL);

  LOGERROR("some error!");
  
  logger_write(__FILE__,LOG_ENTRY_NORMAL,"the message: %s","normal");
  logger_write("err",LOG_ENTRY_ERROR,"the message: %s","error");
  logger_write("warn",LOG_ENTRY_WARNING,"the message: %s","warning");
  logger_write("info",LOG_ENTRY_INFO,"the message: %s","info");

  printf("Log level NORMAL\n");
  logger_set_log_level(LOG_LEVEL_NORMAL);

  logger_write("norm",LOG_ENTRY_NORMAL,"the message: %s","normal");
  logger_write("err",LOG_ENTRY_ERROR,"the message: %s","error");
  logger_write("warn",LOG_ENTRY_WARNING,"the message: %s","warning");
  logger_write("info",LOG_ENTRY_INFO,"the message: %s","info");
  
  printf("Log level ERRORS\n");
  logger_set_log_level(LOG_LEVEL_ERRORS);
  logger_event_start("main",LOG_ENTRY_NORMAL,"logging");
  logger_write("norm",LOG_ENTRY_NORMAL,"the message: %s","normal");
  logger_write("err",LOG_ENTRY_ERROR,"the message: %s","error");
  logger_write("warn",LOG_ENTRY_WARNING,"the message: %s","warning");
  logger_write("info",LOG_ENTRY_INFO,"the message: %s","info");
  logger_event_end("main",LOG_ENTRY_NORMAL,"logging");
  logger_fini();

  return 0;
}
