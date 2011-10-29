INCLUDES = -I$(top_srcdir)

if WITH_TIGHTVNC_FILETRANSFER
TIGHTVNCFILETRANSFERHDRS=tightvnc-filetransfer/filelistinfo.h \
	tightvnc-filetransfer/filetransfermsg.h \
	tightvnc-filetransfer/handlefiletransferrequest.h \
	tightvnc-filetransfer/rfbtightproto.h

TIGHTVNCFILETRANSFERSRCS = tightvnc-filetransfer/rfbtightserver.c \
	tightvnc-filetransfer/handlefiletransferrequest.c \
	tightvnc-filetransfer/filetransfermsg.c \
	tightvnc-filetransfer/filelistinfo.c
endif

if WITH_WEBSOCKETS
WEBSOCKETSSRCS = websockets.c md5.c
endif

includedir=$(prefix)/include/rfb
#include_HEADERS=rfb.h rfbconfig.h rfbint.h rfbproto.h keysym.h rfbregion.h

include_HEADERS=../rfb/rfb.h ../rfb/rfbconfig.h ../rfb/rfbint.h \
	../rfb/rfbproto.h ../rfb/keysym.h ../rfb/rfbregion.h ../rfb/rfbclient.h

noinst_HEADERS=d3des.h ../rfb/default8x16.h zrleoutstream.h \
	zrlepalettehelper.h zrletypes.h private.h minilzo.h lzoconf.h scale.h \
	$(TIGHTVNCFILETRANSFERHDRS)

EXTRA_DIST=tableinit24.c tableinittctemplate.c tabletranstemplate.c \
	tableinitcmtemplate.c tabletrans24template.c \
	zrleencodetemplate.c

if HAVE_LIBZ
ZLIBSRCS = zlib.c zrle.c zrleoutstream.c zrlepalettehelper.c zywrletemplate.c
if HAVE_LIBJPEG
TIGHTSRCS = tight.c
else
if HAVE_LIBPNG
TIGHTSRCS = tight.c
endif
endif
endif

LIB_SRCS = main.c rfbserver.c rfbregion.c auth.c sockets.c $(WEBSOCKETSSRCS) \
	stats.c corre.c hextile.c rre.c translate.c cutpaste.c \
	httpd.c cursor.c font.c \
	draw.c selbox.c d3des.c vncauth.c cargs.c minilzo.c ultra.c scale.c \
	$(ZLIBSRCS) $(TIGHTSRCS) $(TIGHTVNCFILETRANSFERSRCS)

libvncserver_la_SOURCES=$(LIB_SRCS)
libvncserver_la_LIBADD=@SSL_LIBS@ @CRYPT_LIBS@

lib_LTLIBRARIES=libvncserver.la

if HAVE_RPM
$(PACKAGE)-$(VERSION).tar.gz: dist

# Rule to build RPM distribution package
rpm: $(PACKAGE)-$(VERSION).tar.gz libvncserver.spec
	cp $(PACKAGE)-$(VERSION).tar.gz @RPMSOURCEDIR@
	rpmbuild -ba libvncserver.spec
endif


