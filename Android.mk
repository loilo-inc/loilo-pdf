LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# By default, the build system generates ARM target binaries in thumb mode,
# where each instruction is 16 bits wide.  Defining this variable as arm
# forces the build system to generate object files in 32-bit arm mode.  This
# is the same setting previously used by libjpeg.
# TODO (msarett): Run performance tests to determine whether arm mode is still
#                 preferred to thumb mode for libjpeg-turbo.
LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    jcapimin.c jcapistd.c jccoefct.c jccolor.c jcdctmgr.c jchuff.c \
    jcinit.c jcmainct.c jcmarker.c jcmaster.c jcomapi.c jcparam.c \
    jcphuff.c jcprepct.c jcsample.c jctrans.c jdapimin.c jdapistd.c \
    jdatadst.c jdatasrc.c jdcoefct.c  jdcolor.c jddctmgr.c jdhuff.c \
    jdinput.c jdmainct.c jdmarker.c jdmaster.c jdmerge.c jdphuff.c \
    jdpostct.c jdsample.c jdtrans.c jerror.c jfdctflt.c jfdctfst.c \
    jfdctint.c jidctflt.c jidctfst.c jidctint.c jidctred.c jmemmgr.c \
    jmemnobs.c jquant1.c jquant2.c jutils.c

# ARM v7 NEON
LOCAL_SRC_FILES_arm += simd/jsimd_arm_neon.S simd/jsimd_arm.c

# If we are certain that the ARM v7 device has NEON (and there is no need for
# a runtime check), we can indicate that with a flag.
ifeq ($(strip $(TARGET_ARCH)),arm)
  ifeq ($(ARCH_ARM_HAVE_NEON),true)
    LOCAL_CFLAGS += -D__ARM_HAVE_NEON__
  endif
endif

# ARM v8 64-bit NEON
# TODO (msarett): Figure out why this won't compile on Nexus 9.
# LOCAL_SRC_FILES_arm64 += simd/jsimd_arm64_neon.S simd/jsimd_arm64.c

# TODO (msarett): x86 and x86_64 SIMD.  Cross-compiling these assembly files
#                 on Linux for Android is very tricky.  This will require a
#                 YASM or NASM as a dependency.

# TODO (msarett): MIPS SIMD.  This is available in upstream libjpeg-turbo,
#                 but has not been cherry picked into the version used by
#                 Android.

LOCAL_CFLAGS += -O3 -fstrict-aliasing
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)

ifneq (,$(TARGET_BUILD_APPS))
  # Unbundled branch, built against NDK.
  LOCAL_SDK_VERSION := 17
endif

# Build as a static library.
LOCAL_MODULE := libjpeg-turbo_static
include $(BUILD_STATIC_LIBRARY)

# Also build as a shared library.
include $(CLEAR_VARS)

ifneq (,$(TARGET_BUILD_APPS))
  # Unbundled branch, built against NDK.
  LOCAL_SDK_VERSION := 17
endif

LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
LOCAL_WHOLE_STATIC_LIBRARIES = libjpeg-turbo_static
LOCAL_MODULE := libjpeg-turbo
include $(BUILD_SHARED_LIBRARY)
