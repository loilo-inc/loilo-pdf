# PdfRendererCompat

An Open Source PDF Rendering Library based on [AOSP](https://source.android.com/).
This library was developed because of need for compatibility of `PdfRenderer` class with previous version of android.

# Compatibility

Currently >= 4.4 (API Level 19)

# Usage

This library is fully compatible with PdfRenderer class which is available from Android 5.0 (LoLiPop).

```java
// pdf file
File file = new File("file.pdf");
try (ParcelFileDescriptor fd = ParcelFileDescriptor.open(file,ParcelFileDescriptor.MODE_READ_ONLY);
     PdfRendererCompat renderer = new PdfRendererCompat(fd);
) {
    for (int i = 0; i < renderer.getPageCount(); i++) {
        try(PdfRendererCompat.Page page = renderer.openPage(i)){
             Bitmap out = Bitmap.createBitmap(page.getWidth(), page.getHeight(), Bitmap.Config.ARGB_8888);
             Matrix mat = new Matrix();
             mat.setScale(2,2);
             page.render(out, null, mat, PdfRendererCompat.Page.RENDER_MODE_FOR_DISPLAY);
        }
    }
}

```

# Original Pdfium Library

This library is forked from original pdfium library in AOSP, version of 5.0.0-r1.

# License

 Copyright 2014 The Android Open Source Project,

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
