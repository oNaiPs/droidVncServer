## LibVNCServer with the Tight PNG encoding

### LibVNCServer

Copyright (C) 2001-2003 Johannes E. Schindelin

[LibVNCServer](http://sourceforge.net/projects/libvncserver/) is
a library for easy implementation of a RDP/VNC server.


### Tight PNG

Copyright (C) 2010 Joel Martin

The [Tight PNG encoding](http://wiki.qemu.org/VNC_Tight_PNG) is
similar to the Tight encoding, but the basic compression (zlib) is
replaced with PNG data.

This encoding allows for simple, fast clients and bandwidth efficient
clients:

* The PNG data can be rendered directly by the client so there is
  negligible decode work needed in the VNC client itself.
* PNG images are zlib compressed internally (along with other
  optimizations) so tightPng is comparable in bandwith to tight.
* PNG images are natively supported in the browsers. This is optimal
  for web based VNC clients such as
  [noVNC](http://github.com/kanaka/noVNC)


### Usage

See INSTALL for build instructions. To build with the tightPng
encoding, you must also have libpng development libraries install. For
example:

    sudo apt-get install libpng-dev


### TODO

- Handle palette mode (non-truecolor).
