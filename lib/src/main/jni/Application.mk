APP_PLATFORM := android-19
#APP_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays -fexceptions -frtti -Wno-non-virtual-dtor -Wall
#APP_CPPFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays -fexceptions -frtti -Wno-non-virtual-dtor -Wall
APP_CFLAGS += -Wno-error=format-security
APP_STL := gnustl_static
APP_OPTIM := release
APP_ABI := armeabi armeabi-v7a arm64-v8a x86 x86_64
#APP_ABI := armeabi