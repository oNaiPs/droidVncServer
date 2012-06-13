LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES = \
									gralloc.c

LOCAL_C_INCLUDES +=	\
										$(LOCAL_PATH)\
										$(LOCAL_PATH)/..

LOCAL_PRELINK_MODULE:=false #override prelink map
LOCAL_MODULE := libdvnc_gralloc_sdk$(PLATFORM_SDK_VERSION) 
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../libs/$(TARGET_CPU_ABI)

LOCAL_SHARED_LIBRARIES := libhardware libcutils

include $(BUILD_SHARED_LIBRARY)
