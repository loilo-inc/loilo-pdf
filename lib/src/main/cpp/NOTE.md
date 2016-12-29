#環境

* /external以下はAndroid.mkで別でビルドします(後述のpdfiumのビルド方法 参照)

  * ビルドすると、/objと/libsができます

  * Android NDKのバージョンを変更するたびにリビルドする必要があります

* jni部分は gradle + cmake でビルドします。Android Studioでビルドできます。

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

  * ソースコードはAOSPのものを使用する。depot_toolでビルドするのではなく、Android.mkファイルをndkを使用してビルドする。（pdfium/README.mdは本家のもので編集されていない）
  ただし、pdfium以外のモジュールのHEADは新しいビルドシステム（soong?, kati?）に移行していってるみたい。Android.mkがAndroid.bpに置き換えられている。
  次に更新するときはまたビルド方法を悩むことになりそう。

    ```
    mkdir external
    cd external

    git clone https://android.googlesource.com/platform/external/zlib
    cd zlib
    git tag -l
    git checkout -b android-7.1.1_r6 refs/tags/android-7.1.1_r6
    cd ../

    git clone https://android.googlesource.com/platform/external/libpng
    git checkout -b android-7.1.1_r6 refs/tags/android-7.1.1_r6
    cd libpng
    git tag -l
    git checkout -b android-7.1.1_r6 refs/tags/android-7.1.1_r6
    cd ../

    git clone https://android.googlesource.com/platform/external/freetype
    cd freetype
    git tag -l
    git checkout -b android-7.1.1_r6 refs/tags/android-7.1.1_r6
    cd ../

    git clone https://android.googlesource.com/platform/external/pdfium
    cd pdfium
    git tag -l
    git checkout -b android-7.1.1_r6 refs/tags/android-7.1.1_r6
    cd ../
    ```
## Android.mkを編集

  * PC用のビルド等、不要なビルド箇所をコメントアウトする

  * soファイルの出力名の末尾に`-loilo`をつける

    * アプリでsoファイルをロードするときに、同名のsoファイルがプラットフォームにあった場合にそちらが優先されてロードされてしまうらしい。so hellだ。そのため、soファイルはユニークな名前にしておく。

  * ndkでは処理できないVariablesを使わないように修正する（LOCAL_SRC_FILES_armなど）

    * おそらくAOSPで使用しているtoolchainが違う？

## externalフォルダ直下にAndroid.mkを作成

  * pdfiumのビルドの依存関係を1つのAndroid.mkにまとめる。これで相対パスの解決や、LOCAL_SHARED_LIBRARIESが解決できるようになる。

## ndkでビルド

  * Application.mkは使えないみたいなので、ndk-buildのパラメータで全部指定する。

    ```
    ndk-build -B NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk APP_OPTIM=release APP_PLATFORM=android-19 APP_STL=gnustl_static APP_CPPFLAGS="-std=c++11" APP_ABI="armeabi armeabi-v7a arm64-v8a x86 x86_64"
    ```
