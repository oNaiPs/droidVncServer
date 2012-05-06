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

#include "droidVncGrab.h"
#include "screenFormat.h"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <binder/IMemory.h>
#include <surfaceflinger/ISurfaceComposer.h>
#include <surfaceflinger/SurfaceComposerClient.h>

using namespace android;

ScreenshotClient *screenshotClient=NULL;

extern "C" screenFormat getScreenFormat()
{
	//get format on PixelFormat struct
	PixelFormat f=screenshotClient->getFormat();

	PixelFormatInfo pf;
	getPixelFormatInfo(f,&pf);

	screenFormat format;

	format.bitsPerPixel = pf.bitsPerPixel;
	format.width = screenshotClient->getWidth();
	format.height =     screenshotClient->getHeight();
	format.size = pf.bitsPerPixel*format.width*format.height/CHAR_BIT;
	format.redShift = pf.l_red;
	format.redMax = pf.h_red;
	format.greenShift = pf.l_green;
	format.greenMax = pf.h_green-pf.h_red;
	format.blueShift = pf.l_blue;
	format.blueMax = pf.h_blue-pf.h_green;
	format.alphaShift = pf.l_alpha;
	format.alphaMax = pf.h_alpha-pf.h_blue;

	return format;
}


extern "C" int initScreenGrab()
{
	int errno;

	L("--Initializing gingerbread access method--\n");

	screenshotClient=new ScreenshotClient();
	errno=screenshotClient->update();
	if (errno != NO_ERROR) {
		L("screen capture failed: %s\n", strerror(-errno));
		return -1;
	}

	return 0;
}

extern "C" unsigned int * getScreenBuffer()
{
	screenshotClient->update();
	return (unsigned int*)screenshotClient->getPixels();
}

