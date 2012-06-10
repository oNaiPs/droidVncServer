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
#if 0
#include "displaybinder.h" 
#include "common.h"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <binder/IMemory.h>
#include <surfaceflinger/ISurfaceComposer.h>
#include <surfaceflinger/SurfaceComposerClient.h>

using namespace android;

ScreenshotClient *screenshotclient;

extern "C" int initGingerbreadMethod()
{

  L("--Initializing gingerbread access method--\n");
  
  screenshotclient=new ScreenshotClient();
  int err=screenshotclient->update();
   if (err != NO_ERROR) {
        L("screen capture failed: %s\n", strerror(-err));
	//mandar msg incompatible
        return -1;
    }

    PixelFormat f=screenshotclient->getFormat();

    PixelFormatInfo pf;
    getPixelFormatInfo(f,&pf);
    
    
    displayInfo.bpp = pf.bitsPerPixel;
    displayInfo.width = screenshotclient->getWidth();
    displayInfo.height =     screenshotclient->getHeight();;
    displayInfo.size = pf.bitsPerPixel*displayInfo.width*displayInfo.height/CHAR_BIT;
    displayInfo.red_offset = pf.l_red;
    displayInfo.red_length = pf.h_red;
    displayInfo.green_offset = pf.l_green;
    displayInfo.green_length = pf.h_green-pf.h_red;
    displayInfo.blue_offset = pf.l_blue;
    displayInfo.blue_length = pf.h_blue-pf.h_green;
    displayInfo.alpha_offset = pf.l_alpha;
    displayInfo.alpha_length = pf.h_alpha-pf.h_blue;

    return 0;
    }

extern "C" void updateScreen()
{
    screenshotclient->update();
    gingerbuf=(unsigned int*)screenshotclient->getPixels();
}
#endif
