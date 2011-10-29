/*
droid vnc server - Android VNC server
Copyright (C) 2009 Jose Pereira <onaips@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ADB_CONNECT_METHOD
#define ADB_CONNECT_METHOD

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
// #include <cutils/properties.h>

#include "common.h"



#define A_CNXN 0x4e584e43
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257


#define DDMS_RAWIMAGE_VERSION 1

struct _message {
    unsigned int command;       /* command identifier constant      */
    unsigned int arg0;          /* first argument                   */
    unsigned int arg1;          /* second argument                  */
    unsigned int data_length;   /* length of payload (0 is allowed) */
    unsigned int data_crc32;    /* crc32 of data payload            */
    unsigned int magic;         /* command ^ 0xffffffff             */
} __attribute__((packed));



// void error(const char *msg);
// ssize_t write_socket(int fd, const void *buf, size_t count);
// ssize_t read_socket(int fd, void *buf, size_t count);
// void send_connect_string();

int initADB();
void updateADBFrame();

unsigned int *adbbuf;

#endif
