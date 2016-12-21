# pdfiumのビルド方法

## 事前準備

### ndk-bundleにパスを通す

* macの場合

  * terminal.appを起動

  * emacs でファイル編集
    ```
    emacs ~/.bashrc
    ```
    下記を書き込む
    ```
    export PATH=$PATH:$ndk-bundleのパス
    ```
    （emacs保存）ctrl+x, ctrl+s
    （emacs終了）ctrl+x, ctrl+c

  * .bashrc読み込み
    ```
    source ~/.bashrc
    echo $PATH
    ```

## pdfiumのソースコードをチェックアウト
リポジトリ：https://android.googlesource.com/platform/external/pdfium/
```
git clone https://android.googlesource.com/platform/external/pdfium
```
ブランチはlolipop-mr1-releaseあたりを使う

## ビルド
正しいビルド方法は分からないが、とりあえず下記で上手くいった。
本来はdepot_toolやgyp等のchromiumのビルドツールを使うのだろうか？

* libpdfiumcoreをビルド

  * ndk-build実行
    ```
    ndk-build NDK_PROJECT_PATH=./core APP_BUILD_SCRIPT=./core/Android.mk APP_OPTIM=release APP_PLATFORM=android-19 APP_STL=gnustl_static APP_ABI="armeabi armeabi-v7a arm64-v8a x86 x86_64"
    ```

* libpdfiumをビルド

  * /fpdfsdk/Android.mkを修正
    ```
    LOCAL_PATH:= $(call my-dir)

    ### ここから
    include $(CLEAR_VARS)
    LOCAL_MODULE := libpdfiumcore
    LOCAL_SRC_FILES := ../core/obj/local/$(TARGET_ARCH_ABI)/libpdfiumcore.a
    include $(PREBUILT_STATIC_LIBRARY)
    ### ここまでを追加

    include $(CLEAR_VARS)

    LOCAL_MODULE := libpdfium

    LOCAL_ARM_MODE := arm
    LOCAL_SDK_VERSION := 19
    LOCAL_NDK_STL_VARIANT := gnustl_static

    LOCAL_CFLAGS += -O3 -fstrict-aliasing -fprefetch-loop-arrays -fexceptions
    LOCAL_CFLAGS += -Wno-non-virtual-dtor -Wall
    LOCAL_CFLAGS += -DFOXIT_CHROME_BUILD

    LOCAL_STATIC_LIBRARIES := libpdfiumcore

    # TODO: figure out why turning on exceptions requires manually linking libdl
    LOCAL_SHARED_LIBRARIES := libdl

    LOCAL_SRC_FILES := \
        src/fpdf_dataavail.cpp \
        src/fpdf_ext.cpp \
        src/fpdf_flatten.cpp \
        src/fsdk_rendercontext.cpp \
        src/fpdf_progressive.cpp \
        src/fpdf_searchex.cpp \
        src/fpdf_sysfontinfo.cpp \
        src/fpdf_transformpage.cpp \
        src/fpdfdoc.cpp \
        src/fpdfeditimg.cpp \
        src/fpdfeditpage.cpp \
        src/fpdfoom.cpp \
        src/fpdfppo.cpp \
        src/fpdfsave.cpp \
        src/fpdfview.cpp \
        src/fpdftext.cpp

    LOCAL_C_INCLUDES := \
        ../core
        #external/pdfium/core パスが解決できないので上記に書き換え

    include $(BUILD_SHARED_LIBRARY)

    ```

  * ndk-build実行
    ```
    ndk-build NDK_PROJECT_PATH=./fpdfsdk APP_BUILD_SCRIPT=./fpdfsdk/Android.mk APP_OPTIM=release APP_PLATFORM=android-19 APP_STL=gnustl_static APP_ABI="armeabi armeabi-v7a arm64-v8a x86 x86_64"
    ```
