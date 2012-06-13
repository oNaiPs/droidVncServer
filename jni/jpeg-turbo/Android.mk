# Makefile for libjpeg-turbo
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
 
##################################################
###                simd                        ###
##################################################
LOCAL_PATH := $(my-dir)
include $(CLEAR_VARS)

# Set ANDROID_JPEG_USE_VENUM to true to enable VeNum optimizations
ANDROID_JPEG_USE_VENUM := true

# Disable VeNum optimizations if they are not supported on the build target
ifneq ($(ARCH_ARM_HAVE_VFP),true)
ANDROID_JPEG_USE_VENUM := false
else
ifneq ($(ARCH_ARM_HAVE_NEON),true)
ANDROID_JPEG_USE_VENUM := false
endif
endif
 
# From autoconf-generated Makefile
EXTRA_DIST = simd/nasm_lt.sh simd/jcclrmmx.asm simd/jcclrss2.asm simd/jdclrmmx.asm simd/jdclrss2.asm \
	simd/jdmrgmmx.asm simd/jdmrgss2.asm simd/jcclrss2-64.asm simd/jdclrss2-64.asm \
	simd/jdmrgss2-64.asm simd/CMakeLists.txt
 
libsimd_SOURCES_DIST = simd/jsimd_arm_neon.S \
                       asm/armv7//jdcolor-armv7.S asm/armv7/jdidct-armv7.S \
                       simd/jsimd_arm.c 
# or jsimd_none.c

LOCAL_ARM_NEON  := true

 
LOCAL_SRC_FILES := $(libsimd_SOURCES_DIST)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/simd \
                    $(LOCAL_PATH)/android
 
LOCAL_CFLAGS := -DANDROID_JPEG_USE_VENUM
AM_CFLAGS := -march=armv7-a -mfpu=neon
AM_CCASFLAGS := -march=armv7-a -mfpu=neon
 
LOCAL_MODULE_TAGS := debug
 
LOCAL_MODULE := libsimd
 
include $(BUILD_STATIC_LIBRARY)
 
######################################################
###           libjpeg.so                       ##
######################################################
 
include $(CLEAR_VARS)

# From autoconf-generated Makefile
libjpeg_SOURCES_DIST =  jcapimin.c jcapistd.c jccoefct.c jccolor.c \
        jcdctmgr.c jchuff.c jcinit.c jcmainct.c jcmarker.c jcmaster.c \
        jcomapi.c jcparam.c jcphuff.c jcprepct.c jcsample.c jctrans.c \
        jdapimin.c jdapistd.c jdatadst.c jdatasrc.c jdcoefct.c jdcolor.c \
        jddctmgr.c jdhuff.c jdinput.c jdmainct.c jdmarker.c jdmaster.c \
        jdmerge.c jdphuff.c jdpostct.c jdsample.c jdtrans.c jerror.c \
        jfdctflt.c jfdctfst.c jfdctint.c jidctflt.c jidctfst.c jidctint.c \
        jidctred.c jquant1.c jquant2.c jutils.c jmemmgr.c jmemnobs.c \
	jaricom.c jcarith.c jdarith.c \
	turbojpeg.c transupp.c jdatadst-tj.c jdatasrc-tj.c
#	turbojpeg-mapfile

#possible adds jmem-android.c jmemnobs.c jmemmgr.c jmem-ashmem.c 

LOCAL_SRC_FILES:= $(libjpeg_SOURCES_DIST)
 
LOCAL_SHARED_LIBRARIES := libcutils
LOCAL_STATIC_LIBRARIES := libsimd
 
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/android
 
LOCAL_CFLAGS := -DAVOID_TABLES  -O3 -fstrict-aliasing  -DANDROID \
        -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT -DANDROID_JPEG_USE_VENUM

#-DANDROID_TILE_BASED_DECODE -DUSE_ANDROID_ASHMEM 
 
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_STATIC_LIBRARY)
 
LOCAL_MODULE_TAGS := debug
 
LOCAL_MODULE := jpeg

include $(BUILD_STATIC_LIBRARY)

######################################################
###         cjpeg                                  ###
######################################################

#include $(CLEAR_VARS)
#
## From autoconf-generated Makefile
#cjpeg_SOURCES = cdjpeg.c cjpeg.c rdbmp.c rdgif.c \
#        rdppm.c rdswitch.c rdtarga.c
#
#LOCAL_SRC_FILES:= $(cjpeg_SOURCES)
#
#LOCAL_SHARED_LIBRARIES := libjpeg
#
#LOCAL_C_INCLUDES := $(LOCAL_PATH)  \
#                    $(LOCAL_PATH)/android
#
#LOCAL_CFLAGS := -DBMP_SUPPORTED -DGIF_SUPPORTED -DPPM_SUPPORTED -DTARGA_SUPPORTED \
#         -DANDROID -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT
#
#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLE)
#
#LOCAL_MODULE_TAGS := debug
#
#LOCAL_MODULE := cjpeg
#
#include $(BUILD_EXECUTABLE)

######################################################
###            djpeg                               ###
######################################################

#include $(CLEAR_VARS)
#
## From autoconf-generated Makefile
#djpeg_SOURCES = cdjpeg.c djpeg.c rdcolmap.c rdswitch.c \
#        wrbmp.c wrgif.c wrppm.c wrtarga.c
#
#LOCAL_SRC_FILES:= $(djpeg_SOURCES)
#
#LOCAL_SHARED_LIBRARIES := libjpeg
#
#LOCAL_C_INCLUDES := $(LOCAL_PATH)  \
#				    $(LOCAL_PATH)/android
#
#LOCAL_CFLAGS := -DBMP_SUPPORTED -DGIF_SUPPORTED -DPPM_SUPPORTED -DTARGA_SUPPORTED \
#            -DANDROID -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT
#
#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLE)
#
#LOCAL_MODULE_TAGS := debug
#
#LOCAL_MODULE := djpeg
#
#include $(BUILD_EXECUTABLE)
#
#######################################################
####            jpegtran                            ###
#######################################################
#
#include $(CLEAR_VARS)
#
## From autoconf-generated Makefile
#jpegtran_SOURCES = jpegtran.c rdswitch.c cdjpeg.c transupp.c
#
#LOCAL_SRC_FILES:= $(jpegtran_SOURCES)
#
#LOCAL_SHARED_LIBRARIES := libjpeg
#
#LOCAL_C_INCLUDES := $(LOCAL_PATH)  \
#				    $(LOCAL_PATH)/android
#
#LOCAL_CFLAGS := -DANDROID -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT
#
#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLE)
#
#LOCAL_MODULE_TAGS := debug
#
#LOCAL_MODULE := jpegtran
#
#include $(BUILD_EXECUTABLE)
#
#######################################################
####              tjunittest                        ###
#######################################################
#
#include $(CLEAR_VARS)
#
## From autoconf-generated Makefile
#tjunittest_SOURCES = tjunittest.c tjutil.c
#
#LOCAL_SRC_FILES:= $(tjunittest_SOURCES)
#
#LOCAL_SHARED_LIBRARIES := libjpeg
#
#LOCAL_C_INCLUDES := $(LOCAL_PATH) \
#				    $(LOCAL_PATH)/android
#
#LOCAL_CFLAGS := -DANDROID -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT
#
#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLE)
#
#LOCAL_MODULE_TAGS := debug
#
#LOCAL_MODULE := tjunittest
#
#include $(BUILD_EXECUTABLE)
#
#######################################################
####              tjbench                           ###
#######################################################
#
#include $(CLEAR_VARS)
#
## From autoconf-generated Makefile
#tjbench_SOURCES = tjbench.c bmp.c tjutil.c rdbmp.c rdppm.c \
#        wrbmp.c wrppm.c
#
#LOCAL_SRC_FILES:= $(tjbench_SOURCES)
#
#LOCAL_SHARED_LIBRARIES := libjpeg
#
#LOCAL_C_INCLUDES := $(LOCAL_PATH)  \
#				    $(LOCAL_PATH)/android
#
#LOCAL_CFLAGS := -DBMP_SUPPORTED -DPPM_SUPPORTED \
#         -DANDROID -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT
#
#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLE)
#
#LOCAL_MODULE_TAGS := debug
#
#LOCAL_MODULE := tjbench
#
#include $(BUILD_EXECUTABLE)
#
#######################################################
####             rdjpgcom                           ###
#######################################################
#
#include $(CLEAR_VARS)
#
## From autoconf-generated Makefile
#rdjpgcom_SOURCES = rdjpgcom.c
#
#LOCAL_SRC_FILES:= $(rdjpgcom_SOURCES)
#
#LOCAL_SHARED_LIBRARIES := libjpeg
#
#LOCAL_C_INCLUDES := $(LOCAL_PATH)  \
#				    $(LOCAL_PATH)/android
#
#LOCAL_CFLAGS :=  -DANDROID -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT
#
#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLE)
#
#LOCAL_MODULE_TAGS := debug
#
#LOCAL_MODULE := rdjpgcom
#
#include $(BUILD_EXECUTABLE)
#
#######################################################
####           wrjpgcom                            ###
#######################################################
#
#include $(CLEAR_VARS)
#
## From autoconf-generated Makefile
#wrjpgcom_SOURCES = wrjpgcom.c
#
#LOCAL_SRC_FILES:= $(wrjpgcom_SOURCES)
#
#LOCAL_SHARED_LIBRARIES := libjpeg
#
#LOCAL_C_INCLUDES := $(LOCAL_PATH) \
#				    $(LOCAL_PATH)/android
#
#LOCAL_CFLAGS := -DANDROID -DANDROID_TILE_BASED_DECODE -DENABLE_ANDROID_NULL_CONVERT
#
#LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLE)
#
#LOCAL_MODULE_TAGS := debug
#
#LOCAL_MODULE := wrjpgcom
#
#include $(BUILD_EXECUTABLE)

endif  # TARGET_SIMULATOR != true
