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


#include <assert.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <arpa/inet.h> 
#include <unistd.h>    
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "logger.h"

#define LOGGER_MAX_QUEUE_OF_PENDING_CONNECTIONS 50

static void run()
{
  /* master file descriptor list */
  fd_set master;
  /* temp file descriptor list for select() */
  fd_set read_fds;
  /* server address */
  struct sockaddr_in serveraddr;
  /* client address */
  struct sockaddr_in clientaddr;
  /* maximum file descriptor number */
  int fdmax;
  /* listening socket descriptor */
  int listener;
  /* newly accept()ed socket descriptor */
  int newfd = 0;
  /* buffer for client data */
  char* recv_buffer;
  int nbytes;

  socklen_t addrlen;
  int i;
  int doExit = 0;
  int rc;
  /* for setsockopt() SO_REUSEADDR, below */
  int yes = 1;

  recv_buffer = calloc(LOGGER_MAX_ENTRY_SIZE+1,1);
    
  /* clear the master and temp sets */
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  
  /* create listener */
  listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  assert(listener != -1);

  /* set reuse_add option */
  rc = setsockopt(listener,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  (char*)(&yes),
                  sizeof(int));
  assert(rc != -1);
  /* bind */
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = INADDR_ANY;
  serveraddr.sin_port = htons(LOGGER_SERVER_PORT);
  memset(&(serveraddr.sin_zero), 0, sizeof(serveraddr.sin_zero));

  rc = bind(listener, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
  assert(rc != -1);

  /* mark socket as accepting connections */
  rc = listen(listener, LOGGER_MAX_QUEUE_OF_PENDING_CONNECTIONS);
  assert(rc != -1);

  /* add the listener to the master set */
  FD_SET(listener, &master);
  /* keep track of the biggest file descriptor */
  fdmax = listener; /* so far, it's this one */


  /* Run loop */
  while (!doExit)
  {
    /* copy it */
    read_fds = master;

    rc = select(fdmax+1, &read_fds, NULL, NULL, NULL);
    assert(rc != -1);
    /* run through the existing connections looking for data to be read */
    for(i = 0; i <= fdmax; i++)
    {
      if(FD_ISSET(i, &read_fds))
      { 
        if (i == listener)
        {
          /* handle new connections */
          addrlen = sizeof(clientaddr);
          newfd = accept(listener, (struct sockaddr *)&clientaddr, &addrlen);
          assert (newfd != -1);
          
          FD_SET(newfd, &master); /* add to master set */
          if(newfd > fdmax) /* keep track of the maximum  */
            fdmax = newfd;
          printf("New connection from %s on socket %d\n", inet_ntoa(clientaddr.sin_addr), newfd);
        }
        else
        {
          /* handle data from a client */
          if((nbytes = recv(i, recv_buffer, LOGGER_MAX_ENTRY_SIZE, 0)) <= 0)
          {
            /* got error or connection closed by client */
            if (nbytes == 0) /* connection closed */
            {
              printf("socket %d hung up\n", i);
            }
            else
              printf("recv() error\n");
 
            /* close connection */
            close(i);
            /* and remove from master set */
            FD_CLR(i, &master);
          }
          else
          {
            printf("got %d bytes\n",nbytes);
            /* we got some data from a client: buf, nbytes */
            recv_buffer[nbytes] = '\0';
            printf("%s",recv_buffer);
            /* handle quit event */
            if ( !strcmp(recv_buffer,"quit\r\n"))
            {
              doExit = 1;
              break;
            }
          }
        }
      }
    }
  }
  close(listener);
  free(recv_buffer);
}


int main(/* int argc, char *argv[] */)
{
  run();
  return 0;
}
