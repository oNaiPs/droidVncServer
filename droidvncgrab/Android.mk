LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES = \
									droidVncGrab.cpp

#LOCAL_CFLAGS += -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_C_INCLUDES +=	$(LOCAL_PATH)

LOCAL_PRELINK_MODULE:=false #override prelink map
LOCAL_MODULE:= droidvncgrab_sdk$(PLATFORM_SDK_VERSION)
LOCAL_MODULE_TAGS:= optional

ifeq ($(PLATFORM_SDK_VERSION),9)
LOCAL_SHARED_LIBRARIES := libsurfaceflinger_client libui libbinder libutils  libcutils #libcrypto libssl libhardware
else ifeq ($(PLATFORM_SDK_VERSION),15)
LOCAL_SHARED_LIBRARIES := libgui libui libbinder libcutils
else
#add here more sdk versions
LOCAL_SHARED_LIBRARIES := libsurfaceflinger_client libui libbinder libutils  libcutils #libcrypto libssl libhardware
endif

include $(BUILD_SHARED_LIBRARY)
