/*
droid vnc server - Android VNC server
Copyright (C) 2011 Jose Pereira <onaips@gmail.com>

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

#include "gralloc.h"
#include "../common.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/fb.h>
#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <cutils/log.h>

#define r(fd, ptr, size) (read((fd), (ptr), (size)) != (int)(size))
#define w(fd, ptr, size) (write((fd), (ptr), (size)) != (int)(size))

struct gralloc_module_t *gralloc;
struct framebuffer_device_t *fbdev = 0;
struct alloc_device_t *allocdev = 0;
buffer_handle_t buf = 0;
unsigned char* data = 0;
int stride;

static int fill_format(int format)
{
    // bpp, red, green, blue, alpha

    static const int format_map[][9] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0},   // INVALID
        {32, 0, 8, 8, 8, 16, 8, 24, 8}, // HAL_PIXEL_FORMAT_RGBA_8888
        {32, 0, 8, 8, 8, 16, 8, 0, 0}, // HAL_PIXEL_FORMAT_RGBX_8888
        {24, 16, 8, 8, 8, 0, 8, 0, 0},  // HAL_PIXEL_FORMAT_RGB_888
        {16, 11, 5, 5, 6, 0, 5, 0, 0},  // HAL_PIXEL_FORMAT_RGB_565
        {32, 16, 8, 8, 8, 0, 8, 24, 8}, // HAL_PIXEL_FORMAT_BGRA_8888
        {16, 11, 5, 6, 5, 1, 5, 0, 1},  // HAL_PIXEL_FORMAT_RGBA_5551
        {16, 12, 4, 8, 4, 4, 4, 0, 4}   // HAL_PIXEL_FORMAT_RGBA_4444
    };
    const int *p;

    if (format == 0 || format > HAL_PIXEL_FORMAT_RGBA_4444)
        return -ENOTSUP;

    p = format_map[format];

    screenformat.bitsPerPixel = *(p++);
    screenformat.redShift = *(p++);
    screenformat.redMax = *(p++);
    screenformat.greenShift = *(p++);
    screenformat.greenMax = *(p++);
    screenformat.blueShift = *(p++);
    screenformat.blueMax = *(p++);
    screenformat.alphaShift = *(p++);
    screenformat.alphaMax = *(p++);

    return 0;
}

#define CHECK_RV if (rv != 0){close_gralloc();return -1;}
#define CHECK_RV_P if (rv != 0){close_gralloc();return NULL;}
    
int init_gralloc()
{
    printf("--Initializing gralloc access method--\n");
      
    int linebytes;
    int rv;

    rv = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const hw_module_t**)&gralloc);
    
    CHECK_RV;
  
    rv = framebuffer_open(&gralloc->common, &fbdev);
  
    CHECK_RV;
  
    if (!fbdev->read) {
        rv = -ENOTSUP;
        close_gralloc();
        return rv;
    }
 
    rv = gralloc_open(&gralloc->common, &allocdev);
    
    CHECK_RV;

    rv = allocdev->alloc(allocdev, fbdev->width, fbdev->height,
                         fbdev->format, GRALLOC_USAGE_SW_READ_OFTEN,
                         &buf, &stride);


    rv = fbdev->read(fbdev, buf);

    CHECK_RV;
    
    rv = gralloc->lock(gralloc, buf, GRALLOC_USAGE_SW_READ_OFTEN, 0, 0,
                       fbdev->width, fbdev->height, (void**)&data);
    CHECK_RV;
    
    rv = fill_format(fbdev->format);
    
    CHECK_RV;

    stride *= (screenformat.bitsPerPixel >> 3);
    linebytes = fbdev->width * (screenformat.bitsPerPixel >> 3);
    screenformat.size = linebytes * fbdev->height;
    screenformat.width = fbdev->width;
    screenformat.height = fbdev->height;
    
  // point of no return: don't attempt alternative means of reading
    // after this
    rv = 0;

    L("Stride=%d   Linebytes=%d %p\n",stride,linebytes,fbdev->setUpdateRect);
    
    if (data)
        gralloc->unlock(gralloc, buf);
    
    L("Copy %d bytes\n",screenformat.size);
     
    L("Returning rv=%d\n",rv);
    return rv;
}


void close_gralloc()
{
    if (buf)
        allocdev->free(allocdev, buf);
    if (allocdev)
        gralloc_close(allocdev);
    if (fbdev)
        framebuffer_close(fbdev);
}

screenFormat getscreenformat_gralloc()
{
  return screenformat;
}

unsigned char *readfb_gralloc ()
{
    int rv;

    rv = fbdev->read(fbdev, buf);

    CHECK_RV_P;
    
    rv = gralloc->lock(gralloc, buf, GRALLOC_USAGE_SW_READ_OFTEN, 0, 0,
                       fbdev->width, fbdev->height, (void**)&data);
    CHECK_RV_P;

    if (data)
        gralloc->unlock(gralloc, buf);

    return data;
}


