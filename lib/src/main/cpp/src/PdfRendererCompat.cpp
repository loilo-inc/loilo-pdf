/*
 * Copyright 2014 The Android Open Source Project,
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "loilo.h"
#include <mutex>
#include "android_bitmap_lock.h"
#include "fpdfview.h"
#include "fsdk_rendercontext.h"

extern "C" {

JNIEXPORT jlong JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeCreate(JNIEnv *env, jobject thiz,
                                                                         jint fd, jlong size);

JNIEXPORT jint JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeGetPageCount(JNIEnv *env,
                                                                              jobject thiz,
                                                                              jlong documentPtr);

JNIEXPORT void JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeClose(JNIEnv *env, jobject thiz,
                                                                       jlong documentPtr);

JNIEXPORT jlong JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeOpenPageAndGetSize(JNIEnv *env,
                                                                                     jobject thiz,
                                                                                     jlong documentPtr,
                                                                                     jint pageIndex,
                                                                                     jdoubleArray outSize);

JNIEXPORT void JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeRenderPage(JNIEnv *env,
                                                                            jobject thiz,
                                                                            jlong documentPtr,
                                                                            jlong pagePtr,
                                                                            jobject jbitmap,
                                                                            jint destLeft,
                                                                            jint destTop,
                                                                            jint destRight,
                                                                            jint destBottom,
                                                                            jfloatArray matrix,
                                                                            jint renderMode);

JNIEXPORT jboolean JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeScaleForPrinting(JNIEnv *env,
                                                                                      jobject thiz,
                                                                                      jlong documentPtr);

JNIEXPORT void JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeClosePage(JNIEnv *env,
                                                                           jobject thiz,
                                                                           jlong pagePtr);
}

static std::mutex sLibraryLock;
volatile static int sLibraryReferenceCount = 0;

static const int RENDER_MODE_FOR_DISPLAY = 1;
static const int RENDER_MODE_FOR_PRINT = 2;

static void initializeLibraryIfNeeded() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
    std::lock_guard<std::mutex> lock(sLibraryLock);
#pragma clang diagnostic pop

    if (sLibraryReferenceCount == 0) {
        LOILOG_D("Init FPDF library");
        FPDF_InitLibrary(NULL);
    }
    sLibraryReferenceCount++;
}

static void destroyLibraryIfNeeded() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
    std::lock_guard<std::mutex> lock(sLibraryLock);
#pragma clang diagnostic pop
    sLibraryReferenceCount--;
    if (sLibraryReferenceCount == 0) {
        LOILOG_D("Destroy FPDF library");
        FPDF_DestroyLibrary();
    }
}

static int getBlock(void *param, unsigned long position, unsigned char *outBuffer,
                    unsigned long size) {
    const int fd = static_cast<int>(reinterpret_cast<intptr_t>(param));
    const ssize_t readCount = pread(fd, outBuffer, size, position);
    if (readCount < 0) {
        LOILOG_E("Cannot read from file descriptor. Error:%d", errno);
        return 0;
    }
    return 1;
}

static void dropContext(void *data) {
    delete (CRenderContext *) data;
}

static void renderPageBitmap(FPDF_BITMAP bitmap, FPDF_PAGE page, int destLeft, int destTop,
                             int destRight, int destBottom, float transform[], int flags) {
    // Note: this code ignores the currently unused RENDER_NO_NATIVETEXT,
    // FPDF_RENDER_LIMITEDIMAGECACHE, FPDF_RENDER_FORCEHALFTONE, FPDF_GRAYSCALE,
    // and FPDF_ANNOT flags. To add support for that refer to FPDF_RenderPage_Retail
    // in fpdfview.cpp

    CRenderContext *pContext = FX_NEW CRenderContext;

    CPDF_Page *pPage = (CPDF_Page *) page;
    pPage->SetPrivateData((void *) 1, pContext, dropContext);

    CFX_FxgeDevice *fxgeDevice = FX_NEW CFX_FxgeDevice;
    pContext->m_pDevice = fxgeDevice;

    // Reverse the bytes (last argument TRUE) since the Android
    // format is ARGB while the renderer uses BGRA internally.
    fxgeDevice->Attach((CFX_DIBitmap *) bitmap, 0, TRUE);

    CPDF_RenderOptions *renderOptions = pContext->m_pOptions;

    if (!renderOptions) {
        renderOptions = FX_NEW CPDF_RenderOptions;
        pContext->m_pOptions = renderOptions;
    }

    if (flags & FPDF_LCD_TEXT) {
        renderOptions->m_Flags |= RENDER_CLEARTYPE;
    } else {
        renderOptions->m_Flags &= ~RENDER_CLEARTYPE;
    }

    const CPDF_OCContext::UsageType usage = (flags & FPDF_PRINTING)
                                            ? CPDF_OCContext::Print : CPDF_OCContext::View;

    renderOptions->m_AddFlags = static_cast<FX_DWORD >(flags) >> 8;
    renderOptions->m_pOCContext = new CPDF_OCContext(pPage->m_pDocument, usage);

    fxgeDevice->SaveState();

    FX_RECT clip;
    clip.left = destLeft;
    clip.right = destRight;
    clip.top = destTop;
    clip.bottom = destBottom;
    fxgeDevice->SetClip_Rect(&clip);

    CPDF_RenderContext *pageContext = FX_NEW CPDF_RenderContext;
    pContext->m_pContext = pageContext;
    pageContext->Create(pPage);

    CFX_AffineMatrix matrix;
    if (!transform) {
        pPage->GetDisplayMatrix(matrix, destLeft, destTop, destRight - destLeft,
                                destBottom - destTop, 0);
    } else {
        pPage->GetDisplayMatrix(matrix, 0, 0, static_cast<int >(pPage->GetPageWidth()),
                                static_cast<int>(pPage->GetPageHeight()), 0);
        matrix.Concat(
                transform[0], transform[3],
                transform[1], transform[4],
                transform[2], transform[5]
        );
    }
    pageContext->AppendObjectList(pPage, &matrix);

    pContext->m_pRenderer = FX_NEW CPDF_ProgressiveRenderer;
    pContext->m_pRenderer->Start(pageContext, fxgeDevice, renderOptions, NULL);

    fxgeDevice->RestoreState();

    pPage->RemovePrivateData((void *) 1);

    delete pContext;
}

JNIEXPORT jlong JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeCreate(JNIEnv *env,
                                                                         jobject /* thiz */,
                                                                         jint fd, jlong size) {
    return loilo::call<jlong>(env, [=]() -> jlong {

        initializeLibraryIfNeeded();

        FPDF_FILEACCESS loader = {0};
        loader.m_FileLen = static_cast<unsigned long>(size);
        loader.m_Param = reinterpret_cast<void *>(intptr_t(fd));
        loader.m_GetBlock = &getBlock;

        FPDF_DOCUMENT document = FPDF_LoadCustomDocument(&loader, NULL);

        if (!document) {
            const long error = FPDF_GetLastError();
            switch (error) {
                case FPDF_ERR_PASSWORD: {
                    destroyLibraryIfNeeded();
                    loilo::post_throw(env, "java/lang/SecurityException",
                                      "Failed to create document. error = FPDF_ERR_PASSWORD");
                    return 0;
                }
                case FPDF_ERR_SECURITY: {
                    destroyLibraryIfNeeded();
                    loilo::post_throw(env, "java/lang/SecurityException",
                                      "Failed to create document. error = FPDF_ERR_SECURITY");
                    return 0;
                }
                default: {
                    destroyLibraryIfNeeded();
                    loilo::post_throw(env, "java/io/IOException", "Failed to create document.");
                    return 0;
                }
            }
        }

        return reinterpret_cast<jlong>(document);
    });
}

JNIEXPORT jint JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeGetPageCount(JNIEnv *env,
                                                                              jobject /* thiz */,
                                                                              jlong documentPtr) {
    return loilo::call<jint>(env, [=]() -> jint {
        FPDF_DOCUMENT document = reinterpret_cast<FPDF_DOCUMENT>(documentPtr);
        const int pageCount = FPDF_GetPageCount(document);
        return static_cast<jint>(pageCount);
    });
}

JNIEXPORT void JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeClose(JNIEnv *env,
                                                                       jobject /* thiz */,
                                                                       jlong documentPtr) {
    loilo::run(env, [=]() {
        FPDF_DOCUMENT document = reinterpret_cast<FPDF_DOCUMENT>(documentPtr);
        FPDF_CloseDocument(document);
        destroyLibraryIfNeeded();
    });
}

JNIEXPORT jlong JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeOpenPageAndGetSize(JNIEnv *env,
                                                                                     jobject /* thiz */,
                                                                                     jlong documentPtr,
                                                                                     jint pageIndex,
                                                                                     jdoubleArray outSize) {
    return loilo::call<jlong>(env, [=]() -> jlong {

        FPDF_DOCUMENT document = reinterpret_cast<FPDF_DOCUMENT>(documentPtr);

        FPDF_PAGE page = FPDF_LoadPage(document, pageIndex);

        if (!page) {
            loilo::post_throw(env, "java/lang/IllegalStateException", "Failed to load page.");
            return 0;
        }

        double width = 0;
        double height = 0;

        const int result = FPDF_GetPageSizeByIndex(document, pageIndex, &width, &height);

        if (!result) {
            loilo::post_throw(env, "java/lang/IllegalStateException", "Failed to get page size.");
            return 0;
        }

        double *outPtr = env->GetDoubleArrayElements(outSize, JNI_FALSE);
        outPtr[0] = width;
        outPtr[1] = height;
        env->ReleaseDoubleArrayElements(outSize, outPtr, 0);

        return reinterpret_cast<jlong>(page);
    });
}

JNIEXPORT jboolean JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeScaleForPrinting(JNIEnv *env,
                                                                                      jobject /* thiz */,
                                                                                      jlong documentPtr) {
    return loilo::call<jboolean>(env, [=]() -> jboolean {
        FPDF_DOCUMENT document = reinterpret_cast<FPDF_DOCUMENT>(documentPtr);
        FPDF_BOOL ret = FPDF_VIEWERREF_GetPrintScaling(document);
        if (ret) {
            return JNI_TRUE;
        } else {
            return JNI_FALSE;
        }
    });
}

JNIEXPORT void JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeClosePage(JNIEnv *env,
                                                                           jobject /* thiz */,
                                                                           jlong pagePtr) {
    loilo::run(env, [=]() {
        FPDF_PAGE page = reinterpret_cast<FPDF_PAGE>(pagePtr);
        FPDF_ClosePage(page);
    });
}

JNIEXPORT void JNICALL Java_tv_loilo_pdf_PdfRendererCompat_nativeRenderPage(JNIEnv *env,
                                                                            jobject /* thiz */,
                                                                            jlong /* documentPtr */,
                                                                            jlong pagePtr,
                                                                            jobject jbitmap,
                                                                            jint destLeft,
                                                                            jint destTop,
                                                                            jint destRight,
                                                                            jint destBottom,
                                                                            jfloatArray matrix,
                                                                            jint renderMode) {
    loilo::run(env, [=]() {
        AndroidBitmapInfo info;
        if (AndroidBitmap_getInfo(env, jbitmap, &info) < 0) {
            throw std::runtime_error("Failed to get bitmap info.");
        }

        if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
            throw std::invalid_argument(
                    "Invalid format. only ANDROID_BITMAP_FORMAT_RGBA_8888 is accepted.");
        }

        loilo::android_bitmap_lock lock(env, jbitmap);
        {
            FPDF_PAGE page = reinterpret_cast<FPDF_PAGE>(pagePtr);
            FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(info.width, info.height, FPDFBitmap_BGRA,
                                                     lock.pixels(), info.stride);

            float *transform = NULL;
            if (matrix) {
                jsize transformLength = env->GetArrayLength(matrix);
                if (transformLength != 9) {
                    throw std::invalid_argument("The matrix size is not 9.");
                }
                transform = new float[9];
                jfloat *matrixBody = env->GetFloatArrayElements(matrix, JNI_FALSE);
                for (int i = 0; i < transformLength; ++i) {
                    transform[i] = matrixBody[i];
                }
                env->ReleaseFloatArrayElements(matrix, matrixBody, JNI_ABORT);
            }

            int renderFlags = 0;
            if (renderMode == RENDER_MODE_FOR_DISPLAY) {
                renderFlags |= FPDF_LCD_TEXT;
            } else if (renderMode == RENDER_MODE_FOR_PRINT) {
                renderFlags |= FPDF_PRINTING;
            }
            renderPageBitmap(bitmap, page, destLeft, destTop, destRight, destBottom, transform,
                             renderFlags);
            if (transform) {
                delete[] transform;
            }
        }
    });
}
