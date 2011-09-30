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

static int process_connected_socked(int socket, fd_set* master)
{
  int do_exit = 0;
  /* buffer for client data */
  char* recv_buffer;
  /* received bytes */
  int nbytes;

  /* header of the message */
  int size; 

  recv_buffer = calloc(LOGGER_MAX_ENTRY_SIZE+1,1);
  
  /* handle data from a client */
  /* if((nbytes = recv(socket, recv_buffer, LOGGER_MAX_ENTRY_SIZE, 0)) <= 0) */
  if((nbytes = recv(socket, &size, sizeof(int), 0)) <= 0)    
  {
    /* got error or connection closed by client */
    if (nbytes == 0) /* connection closed */
    {
      printf("socket %d hung up\n", socket);
    }
    else
      printf("recv() error\n");
 
    /* close connection */
    close(socket);
    /* and remove from master set */
    FD_CLR(socket, master);
    /* probably exit? */
    /* do_exit = 1; */
  }
  else
  {
    /* we got some data from a client: buf, nbytes */
    printf("got %d bytes\n",nbytes);
    assert(nbytes == sizeof(int));
    size = ntohl(size);
    printf("received number = %d\n",size);
    nbytes = recv(socket, recv_buffer, size, 0);
    assert(nbytes == size);
    recv_buffer[size] = '\0';
    printf("%s",recv_buffer);
  }
  free(recv_buffer);
  return do_exit;
}

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

  socklen_t addrlen;
  int i;
  int do_exit = 0;
  int rc;
  /* for setsockopt() SO_REUSEADDR, below */
  int yes = 1;

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
  while (!do_exit)
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
          do_exit = process_connected_socked(i,&master);
        }
      }
    }
  }
  close(listener);
}


int main(int argc, char *argv[])
{
  int i = 1;
  if (argc > 1)
  {
    printf("Args: %d\n",argc);
    for (; i < argc; ++ i)
      printf("%s\n",argv[i]);
  }
  run();
  return 0;
}
