/*
  * $Id$
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.*
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * This project is an adaptation of the original fbvncserver for the iPAQ
 * and Zaurus.
 *
 * This is a modification from letsgoustc source, to handle uinput events and more
 *
 */
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
 
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <sys/stat.h>
#include <sys/sysmacros.h>             /* For makedev() */

#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>

#include <assert.h>
#include <errno.h>
//android log
#include <android/log.h> 

/* libvncserver */
#include "rfb/rfb.h"
#include "libvncserver/scale.h"
#include "rfb/keysym.h"
#include "suinput.h"

#define SOCK_PATH  "org.onaips.vnc.localsocket"

#define CONCAT2(a,b) a##b
#define CONCAT2E(a,b) CONCAT2(a,b)
#define CONCAT3(a,b,c) a##b##c
#define CONCAT3E(a,b,c) CONCAT3(a,b,c)
 

#define BUS_VIRTUAL 0x06

/*****************************************************************************/
/* Android does not use /dev/fb0. */
 
char VNC_PASSWORD[256] = "";
char framebuffer_device[256] = "/dev/graphics/fb0";
int VNC_PORT=5901;

static struct fb_var_screeninfo scrinfo;
static struct fb_fix_screeninfo fscrinfo;
static int fbfd = -1;
static int inputfd = -1;

static unsigned int *fbmmap = MAP_FAILED;
static unsigned int *vncbuf;
static unsigned int *cmpbuf;
 
/* Android already has 5900 bound natively. */

static rfbScreenInfoPtr vncscr;

int idle=0,standby=0,change=0,test_mode=0;
static int rotation=0,scaling=100; 
/*****************************************************************************/

static void (*update_screen)(void)=NULL;
static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl);
static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl);
static void rotate();
     
/*****************************************************************************/
      
void update_fb_info()      
{  
       
  if (ioctl(fbfd, FBIOGET_VSCREENINFO, &scrinfo) != 0)
  {
      __android_log_print(ANDROID_LOG_INFO,"VNC","ioctl error\n");
      exit(EXIT_FAILURE);
  } 
} 
 
#define PIXEL_TO_VIRTUALPIXEL(i,j) ((j+scrinfo.yoffset)*scrinfo.xres_virtual+i+scrinfo.xoffset)
#define OUT 32 
#include "update_screen.c"
#define OUT 8 
#include "update_screen.c" 
#define OUT 16
#include "update_screen.c"
    
    
inline size_t roundUpToPageSize(size_t x) {
    return (x + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);
}   
  
static void init_fb(void)
{  
    size_t bytespp;
       
    if ((fbfd = open(framebuffer_device, O_RDWR)) == -1)
    { 
        __android_log_print(ANDROID_LOG_INFO,"VNC","cannot open fb device %s\n", framebuffer_device);
	return;  
        //  exit(EXIT_FAILURE);
    } 
         
    update_fb_info();
    
      if (ioctl(fbfd, FBIOGET_FSCREENINFO, &fscrinfo) != 0)
  {
      __android_log_print(ANDROID_LOG_INFO,"VNC","ioctl error\n");
      exit(EXIT_FAILURE);
  }
    
    bytespp = scrinfo.bits_per_pixel /CHAR_BIT;
    
     __android_log_print(ANDROID_LOG_INFO,"VNC", "line_lenght=%d xres=%d, yres=%d, xresv=%d, yresv=%d, xoffs=%d, yoffs=%d, bpp=%d\n",
                        (int)fscrinfo.line_length,(int)scrinfo.xres, (int)scrinfo.yres,
                        (int)scrinfo.xres_virtual, (int)scrinfo.yres_virtual,
                        (int)scrinfo.xoffset, (int)scrinfo.yoffset,
                        (int)scrinfo.bits_per_pixel);
     
     __android_log_print(ANDROID_LOG_INFO,"VNC","colourmap_rgb=%d:%d:%d    lenght=%d:%d:%d",scrinfo.red.offset,scrinfo.green.offset,scrinfo.blue.offset,scrinfo.red.length,scrinfo.green.length,scrinfo.blue.length);  
    
     
      
      
    size_t size=scrinfo.yres_virtual;
    if (size<scrinfo.yres*2)
    {
        __android_log_print(ANDROID_LOG_INFO,"VNC","Using Droid workaround\n");
	size=scrinfo.yres*2;
    }
    
    size_t fbSize = roundUpToPageSize(fscrinfo.line_length * size);

    fbmmap = mmap(NULL, fbSize , PROT_READ|PROT_WRITE ,  MAP_SHARED , fbfd, 0);
 
          
    if (fbmmap == MAP_FAILED)
    { 
        __android_log_print(ANDROID_LOG_INFO,"VNC","mmap failed\n");
        exit(EXIT_FAILURE);
    } 
} 
 
static void cleanup_fb(void) 
{
    if(fbfd != -1)
    {
        close(fbfd);
    } 
} 

static void init_input()
{
    struct input_id id = {
        BUS_VIRTUAL, /* Bus type. */
        1, /* Vendor id. */
        1, /* Product id. */
        1 /* Version id. */
    }; 
    
    if((inputfd = suinput_open("qwerty", &id)) == -1)
    {
        __android_log_print(ANDROID_LOG_INFO,"VNC","cannot create virtual kbd device.\n");
        //  exit(EXIT_FAILURE); do not exit, so we still can see the framebuffer
    }
}

static void cleanup_kbd()
{
    if(inputfd != -1)
    {
        suinput_close(inputfd);
    }
}

void send_remote_msg(char *msg)
{
  int localsocket, len;
    struct sockaddr_un remote;

    if ((localsocket = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
	__android_log_print(ANDROID_LOG_INFO,"VNC","Couldn't setup local socket!\n");
	exit(1);
    }

    char *name=SOCK_PATH;

    remote.sun_path[0] = '\0';  /* abstract namespace */
    strcpy(remote.sun_path+1, name);

    remote.sun_family = AF_UNIX;

    int nameLen = strlen(name);
    len = 1 + nameLen + offsetof(struct sockaddr_un, sun_path);

    if (connect(localsocket, (struct sockaddr *)&remote, len) == -1) {
        perror("Couldn't connect to gui");
	__android_log_print(ANDROID_LOG_INFO,"VNC","Couldn't connect to local socket!\n");
	return;
    }
    
    if (send(localsocket, msg, strlen(msg),0) == -1) {
        perror("Couldn't send to gui");
	__android_log_print(ANDROID_LOG_INFO,"VNC","Couldn't send to local socket!\n");
	return;
    }
    
    close(localsocket);
}

/*****************************************************************************/
static ClientGoneHookPtr clientGone(rfbClientPtr cl)
{
  char *header="~DISCONNECTED|";
  char *msg=malloc(sizeof(char)*(strlen(header)));
  msg[0]='\0';
  strcat(msg,header);
  send_remote_msg(msg);
  return 0;
}

static rfbNewClientHookPtr clientHook(rfbClientPtr cl)
{
  if (scaling!=100)
      rfbScalingSetup(cl, vncscr->width*scaling/100.0, vncscr->height*scaling/100.0);

  cl->clientGoneHook=clientGone;

  char *header="~CONNECTED|";
  char *msg=malloc(sizeof(char)*((strlen(cl->host)) + strlen(header)));
  msg[0]='\0';
  strcat(msg,header);
  strcat(msg,cl->host);
  send_remote_msg(msg);
  
  return RFB_CLIENT_ACCEPT;
}


void CutText(char* str,int len, struct _rfbClientRec* cl)
{
  str[len]='\0';
  char *header="~CLIP|";
  char *msg=malloc(sizeof(char)*(strlen(str) + strlen(header)));
  msg[0]='\0';
  strcat(msg,header);
  strcat(msg,str);
  send_remote_msg(msg);
}


static void init_fb_server(int argc, char **argv)
{ 
   
    __android_log_print(ANDROID_LOG_INFO,"VNC","Initializing server...\n");
    
    vncbuf = calloc(scrinfo.xres * scrinfo.yres, scrinfo.bits_per_pixel/CHAR_BIT);
    cmpbuf = calloc(scrinfo.xres * scrinfo.yres, scrinfo.bits_per_pixel/CHAR_BIT);
    
    assert(vncbuf != NULL);
    assert(cmpbuf != NULL);
     
    

    if (rotation==0 || rotation==180) 
        vncscr = rfbGetScreen(&argc, argv, scrinfo.xres, scrinfo.yres, 0 /* not used */ , 3,  scrinfo.bits_per_pixel/CHAR_BIT);
    else
        vncscr = rfbGetScreen(&argc, argv, scrinfo.yres, scrinfo.xres, 0 /* not used */ , 3,  scrinfo.bits_per_pixel/CHAR_BIT);
     
    assert(vncscr != NULL);
    
    vncscr->desktopName = "Android";
    vncscr->frameBuffer =(char *)vncbuf;
    vncscr->port = VNC_PORT;
    
    vncscr->kbdAddEvent = keyevent;
    vncscr->ptrAddEvent = ptrevent;
    vncscr->newClientHook = clientHook;

    vncscr->setXCutText = CutText;
    
    if (strcmp(VNC_PASSWORD,"")!=0)
    {
        char **passwords = (char **)malloc(2 * sizeof(char **));
        passwords[0] = VNC_PASSWORD;
        passwords[1] = NULL;
        vncscr->authPasswdData = passwords;
        vncscr->passwordCheck = rfbCheckPasswordByList;
    } 
    

    vncscr->httpDir="/data/data/org.onaips.vnc/files/";

    
    vncscr->serverFormat.redShift=scrinfo.red.offset;
    vncscr->serverFormat.greenShift=scrinfo.green.offset; 
    vncscr->serverFormat.blueShift=scrinfo.blue.offset;
     
    vncscr->serverFormat.redMax=((1<<scrinfo.red.length)-1);
    vncscr->serverFormat.greenMax=((1<<scrinfo.green.length)-1);
    vncscr->serverFormat.blueMax=((1<<scrinfo.blue.length)-1);
     
    vncscr->alwaysShared = TRUE;
    vncscr->handleEventsEagerly = TRUE;
    vncscr->deferUpdateTime = 5;

    rfbInitServer(vncscr);
    
    //assign update_screen depending on bpp
    if (vncscr->serverFormat.bitsPerPixel == 32)
    {
        __android_log_print(ANDROID_LOG_INFO,"VNC","I'm on update_screen_32");
        update_screen=&CONCAT2E(update_screen_,32);
    } 
    else if (vncscr->serverFormat.bitsPerPixel == 16)
    {
        __android_log_print(ANDROID_LOG_INFO,"VNC","I'm on update_screen_16");
        update_screen=&CONCAT2E(update_screen_,16);
    }
    else if (vncscr->serverFormat.bitsPerPixel == 8)
    { 
        __android_log_print(ANDROID_LOG_INFO,"VNC","I'm on update_screen_8");
        update_screen=&CONCAT2E(update_screen_,8);
    }
    else {
        rfbErr("Unsupported pixel depth: %d\n",
               vncscr->serverFormat.bitsPerPixel);
        return;
    }
    
    
    /* Mark as dirty since we haven't sent any updates at all yet. */
    
    rfbMarkRectAsModified(vncscr, 0, 0, scrinfo.xres, scrinfo.yres);    
}

static int keysym2scancode(rfbBool down, rfbKeySym c, rfbClientPtr cl, int *sh, int *alt);

 
static void keyevent(rfbBool down, rfbKeySym key, rfbClientPtr cl)
{
    int code;
//     __android_log_print(ANDROID_LOG_INFO,"VNC","Got keysym: %04x (down=%d)\n", (unsigned int)key, (int)down);
    
    int sh = 0;
    int alt = 0;
    
    if ((code = keysym2scancode(down, key, cl,&sh,&alt)))
    {
        
        int ret=0;
        
        if (key && down)
        {
            if (sh) suinput_press(inputfd, 42); //left shift
            if (alt) suinput_press(inputfd, 56); //left alt
            
            ret=suinput_press(inputfd,code);
            ret=suinput_release(inputfd,code);
            
            if (alt) suinput_release(inputfd, 56); //left alt
            if (sh) suinput_release(inputfd, 42); //left shift
        }
        else
            ;//ret=suinput_release(inputfd,code);
        
//     __android_log_print(ANDROID_LOG_INFO,"VNC","injectKey (%d, %d) ret=%d\n", code , down,ret);    
    }
}

// keyboard code modified from remote input by http://www.math.bme.hu/~morap/RemoteInput/

// q,w,e,r,t,y,u,i,o,p,a,s,d,f,g,h,j,k,l,z,x,c,v,b,n,m
int qwerty[] = {30,48,46,32,18,33,34,35,23,36,37,38,50,49,24,25,16,19,31,20,22,47,17,45,21,44};
//  ,!,",#,$,%,&,',(,),*,+,,,-,.,/
int spec1[] = {57,2,40,4,5,6,8,40,10,11,9,13,51,12,52,52};
int spec1sh[] = {0,1,1,1,1,1,1,0,1,1,1,1,0,0,0,1};
// :,;,<,=,>,?,@
int spec2[] = {39,39,227,13,228,53,215};
int spec2sh[] = {1,0,1,1,1,1,0};
// [,\,],^,_,`
int spec3[] = {26,43,27,7,12,399};
int spec3sh[] = {0,0,0,1,1,0};
// {,|,},~
int spec4[] = {26,43,27,215,14};
int spec4sh[] = {1,1,1,1,0};

static int keysym2scancode(rfbBool down, rfbKeySym c, rfbClientPtr cl, int *sh, int *alt)
{
    int real=1;
    if ('a' <= c && c <= 'z')
        return qwerty[c-'a'];
    if ('A' <= c && c <= 'Z')
    {
        (*sh)=1;
        return qwerty[c-'A'];
    }
    if ('1' <= c && c <= '9')
        return c-'1'+2;
    if (c == '0')
        return 11;
    if (32 <= c && c <= 47)
    { 
        (*sh) = spec1sh[c-32];
        return spec1[c-32];
    }
    if (58 <= c && c <= 64)
    {
        (*sh) = spec2sh[c-58];
         return spec2[c-58];
    } 
    if (91 <= c && c <= 96)
    { 
        (*sh) = spec3sh[c-91];
        return spec3[c-91];
    }   
    if (123 <= c && c <= 127)
    {
        (*sh) = spec4sh[c-123]; 
         return spec4[c-123];
    } 
     switch(c)
    {
    case 0xff08: return 14;// backspace
    case 0xff09: return 15;// tab
    case 1: (*alt)=1; return 34;// ctrl+a
    case 3: (*alt)=1; return 46;// ctrl+c  
    case 4: (*alt)=1; return 32;// ctrl+d
    case 18: (*alt)=1; return 31;// ctrl+r
    case 0xff0D: return 28;// enter
    case 0xff1B: return 158;// esc -> back
    case 0xFF51: return 105;// left -> DPAD_LEFT  
    case 0xFF53: return 106;// right -> DPAD_RIGHT 
    case 0xFF54: return 108;// down -> DPAD_DOWN  
    case 0xFF52: return 103;// up -> DPAD_UP
        // 		case 360: return 232;// end -> DPAD_CENTER (ball click)
    case 0xff50: return KEY_HOME;// home 
    case 0xFFC8: rfbShutdownServer(cl->screen,TRUE); return 0; //F11 disconnect
    case 0xFFC9:  
              __android_log_print(ANDROID_LOG_INFO,"VNC","F12 closing...");    
	      exit(0); //F10 closes daemon
	      break;
    case 0xffc1: down?rotate():0; return 0; // F4 rotate 
    case 0xffff: return 158;// del -> back
    case 0xff55: return 229;// PgUp -> menu
    case 0xffcf: return 127;// F2 -> search
    case 0xffe3: return 127;// left ctrl -> search
    case 0xff56: return 61;// PgUp -> call
    case 0xff57: return 107;// End -> endcall
    case 0xffc2: return 211;// F5 -> focus
    case 0xffc3: return 212;// F6 -> camera
    case 0xffc4: return 150;// F7 -> explorer
    case 0xffc5: return 155;// F8 -> envelope
        
    case 50081:
    case 225: (*alt)=1;
        if (real) return 48; //a with acute
        return 30; //a with acute -> a with ring above
    case 50049: 
    case 193:(*sh)=1; (*alt)=1; 
        if (real) return 48; //A with acute 
        return 30; //A with acute -> a with ring above
    case 50089:
    case 233: (*alt)=1; return 18; //e with acute
    case 50057:  
    case 201:(*sh)=1; (*alt)=1; return 18; //E with acute
    case 50093:
    case 0xffbf: (*alt)=1; 
        if (real) return 36; //i with acute 
        return 23; //i with acute -> i with grave
    case 50061:
    case 205: (*sh)=1; (*alt)=1; 
        if (real) return 36; //I with acute 
        return 23; //I with acute -> i with grave
    case 50099: 
    case 243:(*alt)=1; 
        if (real) return 16; //o with acute 
        return 24; //o with acute -> o with grave
    case 50067: 
    case 211:(*sh)=1; (*alt)=1; 
        if (real) return 16; //O with acute 
        return 24; //O with acute -> o with grave
    case 50102:
    case 246: (*alt)=1; return 25; //o with diaeresis
    case 50070:
    case 214: (*sh)=1; (*alt)=1; return 25; //O with diaeresis
    case 50577: 
    case 245:(*alt)=1; 
        if (real) return 19; //Hungarian o 
        return 25; //Hungarian o -> o with diaeresis
    case 50576:
    case 213: (*sh)=1; (*alt)=1; 
        if (real) return 19; //Hungarian O 
        return 25; //Hungarian O -> O with diaeresis
    case 50106:
        // 		case 0xffbe: (*alt)=1; 
        // 			if (real) return 17; //u with acute 
        // 			return 22; //u with acute -> u with grave
    case 50074:
    case 218: (*sh)=1; (*alt)=1; 
        if (real) return 17; //U with acute 
        return 22; //U with acute -> u with grave
    case 50108:
    case 252: (*alt)=1; return 47; //u with diaeresis
    case 50076: 
    case 220:(*sh)=1; (*alt)=1; return 47; //U with diaeresis
    case 50609:
    case 251: (*alt)=1; 
        if (real) return 45; //Hungarian u 
        return 47; //Hungarian u -> u with diaeresis
    case 50608:
    case 219: (*sh)=1; (*alt)=1; 
        if (real) return 45; //Hungarian U 
        return 47; //Hungarian U -> U with diaeresis
        
    }
    return 0;
}

inline void transform_touch_coordinates(int *x, int *y)
{
    int scale=4096.0;
    int old_x=*x,old_y=*y;
    
    if (rotation==0)
    {  
        *x = old_x*scale/scrinfo.xres-scale/2.0;
        *y = old_y*scale/scrinfo.yres-scale/2.0;
    }
    else if (rotation==90)
    {
        *x =old_y*scale/scrinfo.xres-scale/2.0;
        *y = (scrinfo.yres - old_x)*scale/scrinfo.yres-scale/2.0;
    }
    else if (rotation==180)
    {
        *x =(scrinfo.xres - old_x)*scale/scrinfo.xres-scale/2.0;
        *y =(scrinfo.yres - old_y)*scale/scrinfo.yres-scale/2.0;
    }
    else if (rotation==270)
    {
        *y =old_x*scale/scrinfo.yres-scale/2.0; 
        *x =(scrinfo.xres - old_y)*scale/scrinfo.xres-scale/2.0;
    }
    
}

static void ptrevent(int buttonMask, int x, int y, rfbClientPtr cl)
{
    static int leftClicked=0,rightClicked=0,middleClicked=0;
    
    transform_touch_coordinates(&x,&y);
    
    if((buttonMask & 1)&& leftClicked) {//left btn clicked and moving
        static int i=0;
        i=i+1;
        
        if (i%10==1)//some tweak to not report every move event
        {
            suinput_write(inputfd, EV_ABS, ABS_X, x);
            suinput_write(inputfd, EV_ABS, ABS_Y, y);
            suinput_write(inputfd, EV_SYN, SYN_REPORT, 0);
        }
    }
    else if (buttonMask & 1)//left btn clicked
    {
        leftClicked=1;
        
        suinput_write(inputfd, EV_ABS, ABS_X, x);
        suinput_write(inputfd, EV_ABS, ABS_Y, y);
	suinput_write(inputfd,EV_KEY,BTN_TOUCH,1);
        suinput_write(inputfd, EV_SYN, SYN_REPORT, 0);
        
        
    }
    else if (leftClicked)//left btn released
    {
        leftClicked=0;
	suinput_write(inputfd, EV_ABS, ABS_X, x);
        suinput_write(inputfd, EV_ABS, ABS_Y, y);
	suinput_write(inputfd,EV_KEY,BTN_TOUCH,0);
	suinput_write(inputfd, EV_SYN, SYN_REPORT, 0);
    }
    
    if (buttonMask & 4)//right btn clicked
    {
        rightClicked=1;
        suinput_press(inputfd,158); //back key
    }
    else if (rightClicked)//right button released
    {
        rightClicked=0;
        suinput_release(inputfd,158);
    }
    
    if (buttonMask & 2)//mid btn clicked
    {
        middleClicked=1;
        suinput_press( inputfd,KEY_END);
    }
    else if (middleClicked)// mid btn released
    {
        middleClicked=0;
        suinput_release( inputfd,KEY_END);
    }
}

void sigproc()
{ 	
    __android_log_print(ANDROID_LOG_INFO,"VNC","Cleaning up...\n");
    cleanup_fb();
    cleanup_kbd();
    exit(0); /* normal exit status */
}

 

static void rotate()
{
    rotation+=90;
    rotation=rotation%360;
    
    
    if (rotation==90 || rotation==270) 
        rfbNewFramebuffer(vncscr,(char*)vncbuf, scrinfo.yres, scrinfo.xres,0 /* not used */ , 3,  scrinfo.bits_per_pixel/CHAR_BIT);
    else
        rfbNewFramebuffer(vncscr,(char*)vncbuf, scrinfo.xres, scrinfo.yres,0 /* not used */ , 3,  scrinfo.bits_per_pixel/CHAR_BIT);
    
    vncscr->serverFormat.redShift=scrinfo.red.offset;
    vncscr->serverFormat.greenShift=scrinfo.green.offset;
    vncscr->serverFormat.blueShift=scrinfo.blue.offset;
    
    vncscr->serverFormat.redMax=((1<<scrinfo.red.length)-1);
    vncscr->serverFormat.greenMax=((1<<scrinfo.green.length)-1);
    vncscr->serverFormat.blueMax=((1<<scrinfo.blue.length)-1);  
    
    rfbMarkRectAsModified(vncscr, 0, 0, vncscr->width, vncscr->height);
      
}

void print_usage(char **argv)
{
    printf("androidvncserver [-p password] [-h]\n"
                        "-p password: Password to access server\n"
                        "-r rotation: Screen rotation (degrees) (0,90,180,270)\n"
			"-s screen scale: percentage (20,30,50,100,150)\n"
                        "-h : print this help\n"
		        "-f <device> select framebuffer device\n;");
}


int main(int argc, char **argv)
{
    signal(SIGINT, sigproc);//pipe signals
    signal(SIGKILL, sigproc);
    signal(SIGILL, sigproc);
   
    if(argc > 1)
    {
        int i=1;
	int r;
        while(i < argc)
        {
            if(*argv[i] == '-')
            {
                switch(*(argv[i] + 1))
                {
                case 'h':
                    print_usage(argv);
                    exit(0); 
                    break;
                case 'p': 
                    i++; 
                    strcpy(VNC_PASSWORD,argv[i]);
                    break;
		case 'f': 
                    i++; 
                    strcpy(framebuffer_device,argv[i]);
                    break;
		case 'P': 
                    i++; 
		    VNC_PORT=atoi(argv[i]);
                    break;
		case 'r':
                    i++; 
		    r=atoi(argv[i]);
		    if (r==0 || r==90 || r==180 || r==270)
                        rotation=r;
	 	    __android_log_print(ANDROID_LOG_INFO,"VNC","rotating to %d degrees\n",rotation);
		    break;
		case 's':
                    i++;
		    r=atoi(argv[i]); 
		    if (r>=1 && r <= 150)
                        scaling=r;
		    else 
		      scaling=100;
	 	    __android_log_print(ANDROID_LOG_INFO,"VNC","scaling to %d percent\n",scaling);
		    break;
		case 't': //test mode
		    i++;
		    test_mode=atoi(argv[i]);
		    __android_log_print(ANDROID_LOG_INFO,"VNC","In test mode! t=%d\n",test_mode);
		    break;
                }
            }
            i++;
        }
    }


    __android_log_print(ANDROID_LOG_INFO,"VNC","Initializing framebuffer device...\n");
    init_fb();
  
    __android_log_print(ANDROID_LOG_INFO,"VNC","Initializing virtual keyboard and touch device...\n");
    init_input(); 
  
    __android_log_print(ANDROID_LOG_INFO,"VNC","Initializing VNC server:\n");
    __android_log_print(ANDROID_LOG_INFO,"VNC","	width:  %d\n", (int)scrinfo.xres);
    __android_log_print(ANDROID_LOG_INFO,"VNC","	height: %d\n", (int)scrinfo.yres);
    __android_log_print(ANDROID_LOG_INFO,"VNC","	bpp:    %d\n", (int)scrinfo.bits_per_pixel);
    __android_log_print(ANDROID_LOG_INFO,"VNC","	port:   %d\n", (int)VNC_PORT);
    init_fb_server(argc, argv);
 
    while (1)
    {
        while (vncscr->clientHead == NULL)
            rfbProcessEvents(vncscr, 100000);
	  
        rfbMarkRectAsModified(vncscr, 0, 0, vncscr->width, vncscr->height);

	if (standby>40)
	  rfbProcessEvents(vncscr, 300000);
	else if (standby>30)
	  rfbProcessEvents(vncscr, 200000);
	else
	  rfbProcessEvents(vncscr, 1000);
        update_screen(); 

	
	if (idle)
	  {
	      standby=standby+1;
	      change=0;
	  }
	  else
	      standby=0;
     }

    __android_log_print(ANDROID_LOG_INFO,"VNC","Cleaning up...\n");
    cleanup_fb();
    cleanup_kbd();
}     
