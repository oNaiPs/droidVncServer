

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

local_c_flags +=  -Wall   -O3 -DLIBVNCSERVER_WITH_WEBSOCKETS -DLIBVNCSERVER_HAVE_LIBPNG

local_src_files:= \
	input.c \
	gui.c \
	adb_method.c \
	framebuffer_method.c \
	suinput.c \
	droidvncserver.c \
 gralloc_method.c \
	libvncserver/libvncserver/scale.c \
	libvncserver/libvncserver/main.c \
	libvncserver/libvncserver/rfbserver.c \
	libvncserver/libvncserver/rfbregion.c \
	libvncserver/libvncserver/auth.c \
	libvncserver/libvncserver/sockets.c \
	libvncserver/libvncserver/stats.c \
	libvncserver/libvncserver/corre.c \
	libvncserver/libvncserver/hextile.c \
	libvncserver/libvncserver/rre.c \
	libvncserver/libvncserver/translate.c \
	libvncserver/libvncserver/cutpaste.c \
	libvncserver/libvncserver/httpd.c \
	libvncserver/libvncserver/cursor.c \
	libvncserver/libvncserver/font.c \
	libvncserver/libvncserver/draw.c \
	libvncserver/libvncserver/selbox.c \
	libvncserver/libvncserver/minilzo.c \
	libvncserver/libvncserver/vncauth.c \
	libvncserver/libvncserver/d3des.c \
	libvncserver/libvncserver/md5.c \
	libvncserver/libvncserver/cargs.c \
	libvncserver/libvncserver/ultra.c \
	libvncserver/libvncserver/zlib.c \
	libvncserver/libvncserver/zrle.c \
	libvncserver/libvncserver/zrleoutstream.c \
	libvncserver/libvncserver/zrlepalettehelper.c \
	libvncserver/libvncserver/tight.c \
	libvncserver/libvncserver/zywrletemplate.c \
	libvncserver/libvncserver/websockets.c

ginger_up := displaybinder.cpp
           

local_c_includes := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/libvncserver/libvncserver \
	$(LOCAL_PATH)/libvncserver \
	$(LOCAL_PATH)/libvncserver/common \
	$(LOCAL_PATH)/../../zlib \
	$(LOCAL_PATH)/../../jpeg \
	$(LOCAL_PATH)/../../openssl/include \
	$(LOCAL_PATH)/../../libpng \
 
#######################################

# target
include $(CLEAR_VARS)
LOCAL_SRC_FILES += $(local_src_files)
LOCAL_CFLAGS += $(local_c_flags) -DANDROID_FROYO
LOCAL_C_INCLUDES += $(local_c_includes)


LOCAL_MODULE:= androidvncserver_froyo
LOCAL_MODULE_TAGS:= optional

LOCAL_STATIC_LIBRARIES := libcutils libz libpng  jpeg
LOCAL_SHARED_LIBRARIES := libcrypto libssl libhardware

include $(BUILD_EXECUTABLE)

#######################################

# target
include $(CLEAR_VARS)
LOCAL_SRC_FILES += $(local_src_files) $(ginger_up)
LOCAL_CFLAGS += $(local_c_flags)
LOCAL_C_INCLUDES += $(local_c_includes)


LOCAL_MODULE:= androidvncserver_gingerup
LOCAL_MODULE_TAGS:= optional

LOCAL_STATIC_LIBRARIES := libcutils libz libpng  jpeg
LOCAL_SHARED_LIBRARIES := libcrypto libssl  libhardware libsurfaceflinger_client libui 

include $(BUILD_EXECUTABLE)
