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

#include "common.h"
#include "framebuffer_method.h"
#include "gralloc_method.h"
#include "adb_method.h"

#include "gui.h"
#include "input.h"
#include "displaybinder.h"


#define CONCAT2(a,b) a##b
#define CONCAT2E(a,b) CONCAT2(a,b)
#define CONCAT3(a,b,c) a##b##c
#define CONCAT3E(a,b,c) CONCAT3(a,b,c)
 

char VNC_PASSWORD[256] = "";
int VNC_PORT=5901;
/* Android already has 5900 bound natively in some devices. */


unsigned int *cmpbuf;
unsigned int *vncbuf;

static rfbScreenInfoPtr vncscr;

int idle=0,standby=0, rotation=0,scaling=100;
char *rhost=NULL;
int rport=5500;
void (*update_screen)(void)=NULL;

enum method_type {AUTO,FRAMEBUFFER,ADB,GRALLOC
#ifndef ANDROID_FROYO
,GINGERBREAD};
#else
};
#endif 

enum method_type method=AUTO;

#define PIXEL_TO_VIRTUALPIXEL_FB(i,j) ((j+scrinfo.yoffset)*scrinfo.xres_virtual+i+scrinfo.xoffset)
#define PIXEL_TO_VIRTUALPIXEL(i,j) ((j*displayInfo.width)+i)

#define OUT 8 
#include "update_screen.c" 
#undef OUT

#define OUT 16
#include "update_screen.c"
#undef OUT

// #define OUT 24
// #include "update_screen.c"
// #undef OUT

#define OUT 32 
#include "update_screen.c"
#undef OUT

inline int getCurrentRotation()
{
  return rotation;
}

inline int isIdle()
{
 return idle;
}

void setIdle(int i)
{
 idle=i; 
}

ClientGoneHookPtr clientGone(rfbClientPtr cl)
{
  sendMsgToGui("~DISCONNECTED|\n");
  return 0;
}

rfbNewClientHookPtr clientHook(rfbClientPtr cl)
{
  if (scaling!=100)
  {
    rfbScalingSetup(cl, vncscr->width*scaling/100.0, vncscr->height*scaling/100.0);
    L("Scaling to w=%d  h=%d\n",(int)(vncscr->width*scaling/100.0), (int)(vncscr->height*scaling/100.0));
//     rfbSendNewScaleSize(cl);
  }

  cl->clientGoneHook=(ClientGoneHookPtr)clientGone;

  char *header="~CONNECTED|";
  char *msg=malloc(sizeof(char)*((strlen(cl->host)) + strlen(header)+1));
  msg[0]='\0';
  strcat(msg,header);
  strcat(msg,cl->host);
  strcat(msg,"\n");
  sendMsgToGui(msg);
  free (msg);
    
  return RFB_CLIENT_ACCEPT;
}


void CutText(char* str,int len, struct _rfbClientRec* cl)
{
  str[len]='\0';
  char *header="~CLIP|\n";
  char *msg=malloc(sizeof(char)*(strlen(str) + strlen(header)+1));
  msg[0]='\0';
  strcat(msg,header);
  strcat(msg,str);
  strcat(msg,"\n");
  sendMsgToGui(msg);
  free(msg);
}

void sendServerStarted(){
  sendMsgToGui("~SERVERSTARTED|\n");
}

void sendServerStopped()
{
    sendMsgToGui("~SERVERSTOPPED|\n");
}

void initVncServer(int argc, char **argv)
{ 
      
    vncbuf = calloc(displayInfo.width * displayInfo.height, displayInfo.bpp/CHAR_BIT);
    cmpbuf = calloc(displayInfo.width * displayInfo.height, displayInfo.bpp/CHAR_BIT);
    
    
    assert(vncbuf != NULL);
    assert(cmpbuf != NULL);
     

    if (rotation==0 || rotation==180) 
        vncscr = rfbGetScreen(&argc, argv, displayInfo.width , displayInfo.height, 0 /* not used */ , 3,  displayInfo.bpp/CHAR_BIT);
    else
        vncscr = rfbGetScreen(&argc, argv, displayInfo.height, displayInfo.width, 0 /* not used */ , 3,  displayInfo.bpp/CHAR_BIT);
     
    assert(vncscr != NULL);
    
    vncscr->desktopName = "Android";
    
    
    vncscr->frameBuffer =(char *)vncbuf;

    
    vncscr->port = VNC_PORT;
    
    vncscr->kbdAddEvent = keyEvent;
    vncscr->ptrAddEvent = ptrEvent;
    vncscr->newClientHook = (rfbNewClientHookPtr)clientHook;

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
    vncscr->sslcertfile="self.pem";
    
    vncscr->serverFormat.redShift=displayInfo.red_offset;
    vncscr->serverFormat.greenShift=displayInfo.green_offset; 
    vncscr->serverFormat.blueShift=displayInfo.blue_offset;
     
    vncscr->serverFormat.redMax=((1<<displayInfo.red_length)-1);
    vncscr->serverFormat.greenMax=((1<<displayInfo.green_length)-1);
    vncscr->serverFormat.blueMax=((1<<displayInfo.blue_length)-1);
     
    vncscr->serverFormat.bitsPerPixel=displayInfo.bpp;
    
    vncscr->alwaysShared = TRUE;
    vncscr->handleEventsEagerly = TRUE;
    vncscr->deferUpdateTime = 5;

    rfbInitServer(vncscr);
    
    //assign update_screen depending on bpp
    
    if (vncscr->serverFormat.bitsPerPixel == 32)
        update_screen=&CONCAT2E(update_screen_,32);
//     else if (vncscr->serverFormat.bitsPerPixel == 24)
//         update_screen=&CONCAT2E(update_screen_,24);
    else if (vncscr->serverFormat.bitsPerPixel == 16)
        update_screen=&CONCAT2E(update_screen_,16);
    else if (vncscr->serverFormat.bitsPerPixel == 8)
        update_screen=&CONCAT2E(update_screen_,8);
    else {
        L("Unsupported pixel depth: %d\n",
               vncscr->serverFormat.bitsPerPixel);
	
	sendMsgToGui("~SHOW|Unsupported pixel depth, please send bug report.\n");
        return;
    }
    
    
    /* Mark as dirty since we haven't sent any updates at all yet. */
    
    rfbMarkRectAsModified(vncscr, 0, 0, vncscr->width, vncscr->height);
}

  

void rotate(int value)
{
  
  L("rotate()");
  
  
  if (value==-1 || 
    ((value==90 || value==270) && (rotation==0 || rotation==180)) ||
    ((value==0 || value==180) && (rotation==90 || rotation==270)))
  {
    int h=vncscr->height;
    int w=vncscr->width;
    
    	vncscr->width = h;
	vncscr->paddedWidthInBytes = h * displayInfo.bpp / CHAR_BIT;
	vncscr->height = w;

	{
		rfbClientIteratorPtr iterator;
		rfbClientPtr cl;
		iterator = rfbGetClientIterator(vncscr);
		while ((cl = rfbClientIteratorNext(iterator)) != NULL)
			cl->newFBSizePending = 1;
	}
  }
  
  if (value==-1)
  {
    rotation+=90;
    rotation=rotation%360;
  }
  else
  {
  rotation=value;  
  }
  
  rfbMarkRectAsModified(vncscr, 0, 0, vncscr->width, vncscr->height);
      
}


void close_app()
{ 	
    L("Cleaning up...\n");
    cleanupFramebuffer();
    cleanupInput();
    sendServerStopped();
    unbindIPCserver();
    exit(0); /* normal exit status */
}


void extractReverseHostPort(char *str)
{
  int len=strlen(str);
  char *p;
 /* copy in to host */
	rhost = (char *) malloc(len+1);
	if (! rhost) {
		L("reverse_connect: could not malloc string %d\n", len);
		exit(-1);
	}
	strncpy(rhost, str, len);
	rhost[len] = '\0';

	/* extract port, if any */
	if ((p = strrchr(rhost, ':')) != NULL) {
		rport = atoi(p+1);
		if (rport < 0) {
			rport = -rport;
		} else if (rport < 20) {
			rport = 5500 + rport;
		}
		*p = '\0';
	} 
}

void printUsage(char **argv)
{
    L("\nandroidvncserver [parameters]\n"
                        "-f <device>\t- Framebuffer device (only with -m fb, default is /dev/graphics/fb0)\n"
			"-h\t\t- Print this help\n"
			"-m <method>\t- Display grabber method\n\tfb: framebuffer\n\tgb: gingerbread+ devices\n\tadb: slower, but should be compatible with all devices\n"
			"-p <password>\t- Password to access server\n"
			"-r <rotation>\t- Screen rotation (degrees) (0,90,180,270)\n"
			"-R <host:port>\t- Host for reverse connection\n" 
			"-s <scale>\t- Scale percentage (20,30,50,100,150)\n\n" );
}


int main(int argc, char **argv)
{
    signal(SIGINT, close_app);//pipe signals
    signal(SIGKILL, close_app);
    signal(SIGILL, close_app);
   
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
                    printUsage(argv);
                    exit(0); 
                    break;
                case 'p': 
                    i++; 
                    strcpy(VNC_PASSWORD,argv[i]);
                    break;
		case 'f': 
                    i++; 
                    setFramebufferDevice(argv[i]);
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
	 	    L("rotating to %d degrees\n",rotation);
		    break;
		case 's':
                    i++;
		    r=atoi(argv[i]); 
		    if (r>=1 && r <= 150)
                        scaling=r;
		    else 
		      scaling=100;
	 	    L("scaling to %d percent\n",scaling);
		    break;
		case 'm':
		    i++;
		    if (strcmp(argv[i],"adb")==0)
		    {
		      method=ADB;
		      L("ADB display grabber selected\n");
		    }
		    else if (strcmp(argv[i],"fb")==0)
		    {
		      method=FRAMEBUFFER;
		      L("Framebuffer display grabber selected\n");
		    }
		    else if (strcmp(argv[i],"gralloc")==0)
		    {
		      method=GRALLOC;
		      L("Gralloc display grabber selected\n");
		    }
		    #ifndef ANDROID_FROYO
		    else if (strcmp(argv[i],"gingerbread")==0)
		    {
		      method=GINGERBREAD;
		      L("Gingerbread display grabber selected\n");
		    }
		    #endif
		    else 
		    {  
		      L("Grab method \"%s\" not found, reverting to default.\n",argv[i]);
		    }
		    break;
		case 'R':
		    i++;
		    extractReverseHostPort(argv[i]);
		    break;
                }
            }
            i++;
        }
    }
    
    L("Initializing grabber method...\n");
    
    if (method==AUTO)
    {
     L("No grabber method selected, auto-detecting..."); 
        do
	{
	  #ifndef ANDROID_FROYO
	  if (initGingerbreadMethod()!=-1)
	    method=GINGERBREAD;
	  else 
	  #endif
	  if (init_gralloc()!=-1)
	    method=GRALLOC;

	  else if (initFramebuffer()!=-1)
	    method=FRAMEBUFFER;
	  else if (initADB()!=-1)
	  {
	    method=ADB;
	    updateADBFrame();
	  } 
	  break;
	}
	while (0);
    }
    else if (method==FRAMEBUFFER)
	initFramebuffer();
    else if (method==ADB)
    {
        initADB(); 
        updateADBFrame();
    }
    else if (method==GRALLOC)
       init_gralloc();
    #ifndef ANDROID_FROYO
    else if (method==GINGERBREAD)
       initGingerbreadMethod();
    #endif

    L("Initializing virtual keyboard and touch device...\n");
    initInput(); 
  
    L("Initializing VNC server:\n");
    L("	width:  %d\n", (int)displayInfo.width);
    L("	height: %d\n", (int)displayInfo.height);
    L("	bpp:    %d\n", (int)displayInfo.bpp);
    L("	port:   %d\n", (int)VNC_PORT);
    
    
    L("Colourmap_rgba=%d:%d:%d:%d    lenght=%d:%d:%d:%d\n",displayInfo.red_offset,displayInfo.green_offset,displayInfo.blue_offset,displayInfo.alpha_offset,
    displayInfo.red_length,displayInfo.green_length,displayInfo.blue_length,displayInfo.alpha_length);  
     
    initVncServer(argc, argv);
 
    bindIPCserver();
    sendServerStarted();

      

    if (rhost) 
    {
      	rfbClientPtr cl;
      		cl = rfbReverseConnection(vncscr, rhost, rport);
		if (cl == NULL)
		{
		  char *str=malloc(255*sizeof(char));
		  sprintf(str,"~SHOW|Couldn't connect to remote host:\n%s\n",rhost);
		  
		  L("Couldn't connect to remote host: %s\n",rhost);
		  sendMsgToGui(str);
		  free(str);
		}
		else
		{
			cl->onHold = FALSE;
			rfbStartOnHoldClient(cl);
		}
    }

    rfbRunEventLoop(vncscr,-1,TRUE);

    while (1)
    {
        usleep(300000*(standby/2.0));
	
	
	if (idle)
	   standby++;
	else
	   standby=2;

	if (vncscr->clientHead == NULL)
	  continue;
	
	
 	update_screen(); 
     }

    close_app();
}     

    
 