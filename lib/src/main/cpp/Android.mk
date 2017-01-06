LOILO_BASE_PATH := $(call my-dir)

#include $(LOILO_BASE_PATH)/external/libjpeg-turbo/Android.mk
#include $(LOILO_BASE_PATH)/external/zlib/Android.mk
#include $(LOILO_BASE_PATH)/external/libpng/Android.mk

#include $(LOILO_BASE_PATH)/external/giflib/Android.mk
#include $(LOILO_BASE_PATH)/external/webp/include/Android.mk
#include $(LOILO_BASE_PATH)/external/expat/lib/Android.mk
#include $(LOILO_BASE_PATH)/external/freetype/include/Android.mk
include $(LOILO_BASE_PATH)/external/sfntly/cpp/src/Android.mk
#include $(LOILO_BASE_PATH)/external/dng_sdk/Android.mk
#include $(LOILO_BASE_PATH)/external/piex/Android.mk
#include $(LOILO_BASE_PATH)/external/pdfium/core/Android.mk
#include $(LOILO_BASE_PATH)/external/pdfium/third_party/Android.mk
#include $(LOILO_BASE_PATH)/external/pdfium/fpdfsdk/Android.mk

