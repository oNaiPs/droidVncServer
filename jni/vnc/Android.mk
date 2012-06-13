LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LIBVNCSERVER_ROOT:=./LibVNCServer-0.9.9

LIBVNCSERVER_SRC_FILES:= \
	$(LIBVNCSERVER_ROOT)/libvncserver/main.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rfbserver.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rfbregion.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/auth.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/sockets.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/stats.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/corre.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rfbssl_openssl.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rfbcrypto_openssl.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/hextile.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/rre.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/translate.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/cutpaste.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/httpd.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/cursor.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/font.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/draw.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/websockets.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/selbox.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/cargs.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/ultra.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/scale.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/zlib.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/zrle.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/zrleoutstream.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/zrlepalettehelper.c \
	$(LIBVNCSERVER_ROOT)/libvncserver/tight.c \
	$(LIBVNCSERVER_ROOT)/common/d3des.c \
	$(LIBVNCSERVER_ROOT)/common/vncauth.c \
	$(LIBVNCSERVER_ROOT)/common/minilzo.c \
	$(LIBVNCSERVER_ROOT)/common/zywrletemplate.c \
	$(LIBVNCSERVER_ROOT)/common/turbojpeg.c

LOCAL_CFLAGS  +=  -Wall \
									-O3 \
									-DLIBVNCSERVER_WITH_WEBSOCKETS \
									-DLIBVNCSERVER_HAVE_LIBPNG \
									-DLIBVNCSERVER_HAVE_ZLIB \
									-DLIBVNCSERVER_HAVE_LIBJPEG

LOCAL_LDLIBS +=  -llog -lz -ldl 

LOCAL_SRC_FILES += \
									 $(LIBVNCSERVER_SRC_FILES)\
									 droidvncserver.c \
									 gui.c \
									 inputMethods/input.c \
									 screenMethods/adb.c \
									 screenMethods/framebuffer.c \
									 screenMethods/gralloc.c \
									 screenMethods/flinger.c \
									 suinput/suinput.c 

LOCAL_C_INCLUDES += \
										$(LOCAL_PATH) \
										$(LOCAL_PATH)/screenMethods \
										$(LOCAL_PATH)/inputMethods \
										$(LOCAL_PATH)/suinput \
										$(LOCAL_PATH)/../libpng \
										$(LOCAL_PATH)/../jpeg \
										$(LOCAL_PATH)/../jpeg-turbo \
										$(LOCAL_PATH)/../openssl/include \
										$(LOCAL_PATH)/$(LIBVNCSERVER_ROOT)/libvncserver \
										$(LOCAL_PATH)/$(LIBVNCSERVER_ROOT)/common \
										$(LOCAL_PATH)/$(LIBVNCSERVER_ROOT)/rfb \
										$(LOCAL_PATH)/$(LIBVNCSERVER_ROOT)/ \
										$(LOCAL_PATH)/../../nativeMethods/

LOCAL_STATIC_LIBRARIES := libjpeg libpng libssl_static libcrypto_static

LOCAL_MODULE := androidvncserver

include $(BUILD_EXECUTABLE)
