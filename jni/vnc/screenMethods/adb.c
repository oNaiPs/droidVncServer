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

//This method is a stub to grab the screen from the ADB daemon 
//running on all Android devices
//
//STATE - NOT WORKING
//TODO execute adb in custom tcp port on method initialization
//we must kill previous adb process or it wont be able to bind to listener install

#include "adb.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "common.h"

#define A_CNXN 0x4e584e43
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257

#define DDMS_RAWIMAGE_VERSION 1

#define DDMS_RAWIMAGE_VERSION 1

struct _message {
    unsigned int command;       /* command identifier constant      */
    unsigned int arg0;          /* first argument                   */
    unsigned int arg1;          /* second argument                  */
    unsigned int data_length;   /* length of payload (0 is allowed) */
    unsigned int data_crc32;    /* crc32 of data payload            */
    unsigned int magic;         /* command ^ 0xffffffff             */
} __attribute__((packed));

char connect_string[] = {
  0x43, 0x4e, 0x58, 0x4e, 0x00, 0x00, 0x00, 0x01, 
  0x00, 0x10, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 
  0x32, 0x02, 0x00, 0x00, 0xbc, 0xb1, 0xa7, 0xb1, 
  0x68, 0x6f, 0x73, 0x74, 0x3a, 0x3a, 0x00 };

char framebuffer_string[] = {
  0x4f, 0x50, 0x45, 0x4e, 0x01, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 
  0xbf, 0x04, 0x00, 0x00, 0xb0, 0xaf, 0xba, 0xb1, 
  0x66, 0x72, 0x61, 0x6d, 0x65, 0x62, 0x75, 0x66, 
  0x66, 0x65, 0x72, 0x3a, 0x00 };

char okay_string[24];

struct fbinfo *fb_info;
struct _message *message,*okay_message;
int sockfd;

unsigned int *adbbuf = NULL;

ssize_t write_socket(int fd, const void *buf, size_t count)
{
  int n = write(fd,buf,count);

  if (n < 0) 
  perror("ERROR writing to socket");

  return n;
}


void read_socket(int fd, void *buf, size_t count)
{
  int n=0;

  while (count>0) {
    n=read(fd,buf,count);

    if (n < 0)
    L("ERROR reading from socket\n");

    count -= n;
  }
}

void send_connect_string(void)
{
  write_socket(sockfd,connect_string,sizeof(connect_string));
  read_socket(sockfd,message,sizeof(struct _message));

  if (message->command!=A_CNXN)
    L("bad A_CNXN response\n");

  //lets read,  i don't want this
  read_socket(sockfd,message,message->data_length);  
}

void send_framebuffer_string(void);

int initADB(void)
{
  L("--Initializing adb access method--\n");
  pid_t pid;
  int portno;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  adbbuf=NULL;

  //    property_set("service.adb.tcp.port", "5555");
  
  switch (pid = fork()) {
    case -1:
    perror("adb::fork()");
    exit(EXIT_FAILURE);
    case 0: // in the child
    system("killall adbd");
    system("setprop service.adb.tcp.port -1");
    system("/sbin/adbd");
    exit(0);
    break;
  }

  sleep(1);
  message = malloc(sizeof(struct _message));
  okay_message = malloc(sizeof(struct _message));

  portno = 5555;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
  {
    L("adb ERROR opening socket\n");
    return -1;
  }
  #ifdef ANDROID
  server = gethostbyname("127.0.0.1");
  #else
  server = gethostbyname("192.168.10.6");
  #endif
  if (server == NULL) {
    L("adb ERROR, no such host\n");
    return -1;
  }

  //     L("2\n");

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
  {
    L("adb ERROR connecting\n");
    return -1;
  }

  //     L("3\n");
  send_connect_string();
  //     L("4\n");
return 0;
 }



void send_framebuffer_string(void)//returns the fb struct size from adb
{
  int n;
  char *buffer=NULL;
  write_socket(sockfd,framebuffer_string,sizeof(framebuffer_string));
  read_socket(sockfd,okay_message,sizeof(struct _message));

  if (okay_message->command!=A_OKAY)
  {
    L("---\ncommand=%32X\narg0=%32X\narg1=%32X\ndata_len=%d\n---\n",message->command,message->arg0,message->arg1,message->data_length);
    L("bad OKAY response on send_framebuffer_string\n");

  }

  n=okay_message->arg0;
  okay_message->arg0=okay_message->arg1;
  okay_message->arg1=n;

  //     L("arg0=%08X\narg1=%08X\n",okay_message->arg0,okay_message->arg1);
  //we now have our okay message

  //lets receive fb info...
  read_socket(sockfd,message,sizeof(struct _message));


  if (message->command!=A_WRTE)
  L("bad WRTE response\n");

  //     L("---\ncommand=%32X\narg0=%32X\narg1=%32X\ndata_len=%d\n---\n",message->command,message->arg0,message->arg1,message->data_length);

  //     if (message->data_length!=sizeof(struct _message))
  //       error("Size should match...");

  buffer=(char*)malloc(sizeof(char)*message->data_length);

  struct fbinfo displayInfo;

  read_socket(sockfd,&displayInfo,sizeof(struct fbinfo));

  //     L("sizeof(struct fbinfo)=%d\n",sizeof(struct fbinfo));

  //      L("ADB framebuffer method:\nversion=%d\nbpp=%d\nsize=%d bytes\nwidth=%dpx\nheight=%dpx\nsize=%d\n",
  // 	    displayInfo.version,displayInfo.bpp,displayInfo.size,displayInfo.width,displayInfo.height,displayInfo.size);

  //      L("sizeof(struct _message)=%d\n",sizeof(struct _message));
  write_socket(sockfd,okay_message,sizeof(struct _message));
  if (adbbuf==NULL)
  adbbuf=(unsigned int*)malloc(displayInfo.size);

  screenformat.bitsPerPixel = displayInfo.bpp;
  screenformat.width = displayInfo.width;
  screenformat.height = displayInfo.height;
  screenformat.redMax = displayInfo.red_length;
  screenformat.greenMax = displayInfo.green_length;
  screenformat.blueMax = displayInfo.blue_length;
  screenformat.alphaMax = displayInfo.alpha_length;
  screenformat.redShift = displayInfo.red_offset;
  screenformat.greenShift = displayInfo.green_offset;
  screenformat.blueShift = displayInfo.blue_offset;
  screenformat.alphaShift = displayInfo.alpha_offset;
  screenformat.size = displayInfo.size;
}

unsigned int *readBufferADB(void)
{
  int n=0;
  int count=0;

  send_framebuffer_string();

  //   L("1\n");

  while (message->command!=A_CLSE)
  {
    read_socket(sockfd,message,sizeof(struct _message));

    //         L("---\ncommand=%32X\narg0=%32X\narg1=%32X\ndata_len=%d\n---\n",message->command,message->arg0,message->arg1,message->data_length);

    if (message->command==A_CLSE)
    break;
    else if (message->command!=A_WRTE)
    {
      L("Weird command received... %08X\n",message->command);
      L("---\ncommand=%32X\narg0=%32X\narg1=%32X\ndata_len=%d\n---\n",message->command,message->arg0,message->arg1,message->data_length);
    }


    char * s=(char*)adbbuf+count;
    read_socket(sockfd,s,message->data_length);
    count+=message->data_length;

    //       L("ahah %d %d\n",message->data_length,n);

    n=write_socket(sockfd,okay_message,sizeof(struct _message));
    if (n<0)
    break;
  }

  // L("Exit updating...\n");
  // L("Received %d bytes\n",count);

  //     if (resp!=A_OKAY)
  //      error("bad OKAY response");

  return adbbuf;
}

void closeADB()
{
  close(sockfd);
}

// test start point
// int main(int argc, char *argv[])
// {
//     message=(struct _message*)malloc(sizeof(struct _message));
//     okay_message=(struct _message*)malloc(sizeof(struct _message));
//     
//     connect_to_adb();
//     
// //     adbbuf=calloc(fb_info->size,1);
//     
//     update_frame();
//     
//     
//     close(sockfd);
//     return 0;
// }
