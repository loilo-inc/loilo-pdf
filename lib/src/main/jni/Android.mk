LOCAL_PATH := $(call my-dir)

#Prebuild fpdfsdk
include $(CLEAR_VARS)
LOCAL_MODULE := libpdfium
LOCAL_SRC_FILES := $(LOCAL_PATH)/external/pdfium/fpdfsdk/libs/$(TARGET_ARCH_ABI)/libpdfium.so
include $(PREBUILT_SHARED_LIBRARY)

#Main JNI library
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_SDK_VERSION := 19
LOCAL_NDK_STL_VARIANT := gnustl_static

LOCAL_MODULE := libloilopdf

LOCAL_CFLAGS += -O3 -fstrict-aliasing -fexceptions -frtti -Wno-non-virtual-dtor -Wall -DHAVE_PTHREADS
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/external/pdfium/core/include
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/external/pdfium/fpdfsdk/include
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/external/utils

LOCAL_CPPFLAGS += -std=c++11 -O3 -fstrict-aliasing -fexceptions -frtti -Wno-non-virtual-dtor -Wall -DHAVE_PTHREADS
#LOCAL_CPPFLAGS += -I$(LOCAL_PATH)/external/pdfium/core/include
#LOCAL_CPPFLAGS += -I$(LOCAL_PATH)/external/pdfium/fpdfsdk/include
#LOCAL_CPPFLAGS += -I$(LOCAL_PATH)/external/utils

LOCAL_SHARED_LIBRARIES += libpdfium
LOCAL_LDLIBS += -llog -landroid -ljnigraphics

LOCAL_SRC_FILES :=  $(LOCAL_PATH)/PdfRendererCompat.cpp

include $(BUILD_SHARED_LIBRARY)