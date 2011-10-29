

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
	libvncserver-kanaka/libvncserver/scale.c \
	libvncserver-kanaka/libvncserver/main.c \
	libvncserver-kanaka/libvncserver/rfbserver.c \
	libvncserver-kanaka/libvncserver/rfbregion.c \
	libvncserver-kanaka/libvncserver/auth.c \
	libvncserver-kanaka/libvncserver/sockets.c \
	libvncserver-kanaka/libvncserver/stats.c \
	libvncserver-kanaka/libvncserver/corre.c \
	libvncserver-kanaka/libvncserver/hextile.c \
	libvncserver-kanaka/libvncserver/rre.c \
	libvncserver-kanaka/libvncserver/translate.c \
	libvncserver-kanaka/libvncserver/cutpaste.c \
	libvncserver-kanaka/libvncserver/httpd.c \
	libvncserver-kanaka/libvncserver/cursor.c \
	libvncserver-kanaka/libvncserver/font.c \
	libvncserver-kanaka/libvncserver/draw.c \
	libvncserver-kanaka/libvncserver/selbox.c \
	libvncserver-kanaka/libvncserver/minilzo.c \
	libvncserver-kanaka/libvncserver/vncauth.c \
	libvncserver-kanaka/libvncserver/d3des.c \
	libvncserver-kanaka/libvncserver/md5.c \
	libvncserver-kanaka/libvncserver/cargs.c \
	libvncserver-kanaka/libvncserver/ultra.c \
	libvncserver-kanaka/libvncserver/zlib.c \
	libvncserver-kanaka/libvncserver/zrle.c \
	libvncserver-kanaka/libvncserver/zrleoutstream.c \
	libvncserver-kanaka/libvncserver/zrlepalettehelper.c \
	libvncserver-kanaka/libvncserver/tight.c \
	libvncserver-kanaka/libvncserver/zywrletemplate.c \
	libvncserver-kanaka/libvncserver/websockets.c

ginger_up := displaybinder.cpp
           

local_c_includes := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/libvncserver-kanaka/libvncserver \
	$(LOCAL_PATH)/libvncserver-kanaka \
	$(LOCAL_PATH)/libvncserver-kanaka/common \
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
