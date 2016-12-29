# loilo-pdf

An Open Source PDF Rendering Library for Android based on [pdfium](https://android.googlesource.com/platform/external/pdfium/) @ [AOSP](https://source.android.com/).
This library was developed because of need for compatibility of `PdfRenderer` class with previous version of android.

## Compatibility

Currently >= 4.4 (API Level 19)

## Usage

This library is fully compatible with PdfRenderer class which is available from Android 5.0 (lollipop).

```java
// pdf file
File file = new File("file.pdf");
try (ParcelFileDescriptor fd = ParcelFileDescriptor.open(file,ParcelFileDescriptor.MODE_READ_ONLY);
     PdfRendererCompat renderer = new PdfRendererCompat(fd);
) {
    for (int i = 0; i < renderer.getPageCount(); i++) {
        try(PdfRendererCompat.Page page = renderer.openPage(i)){
             Bitmap out = Bitmap.createBitmap(100, 100, Bitmap.Config.ARGB_8888);
             Matrix mat = new Matrix();
             mat.setScale(2,2);
             page.render(out, null, mat, PdfRendererCompat.Page.RENDER_MODE_FOR_DISPLAY);
        }
    }
}

```

## Install

PdfRenderer is available on Marven Central.

**!! Sorry, v1.0.0 contains a critical bug. Use previous version.**

via Gradle

```
~~dependencies {~~
~~  compile 'tv.loilo.pdf:pdfcompat:1.0.0'~~
~~}~~

repositories {
  maven {
    url 'http://oss.sonatype.org/content/repositories/snapshots'
  }		
}
		
dependencies {
  compile 'tv.loilo.pdf:pdfcompat:0.2.1-SNAPSHOT'
}
```

## Original Pdfium Library

This library is forked from original pdfium library in AOSP, version of 5.0.0-r1.

## LICENSE

Apache License Version 2.0
