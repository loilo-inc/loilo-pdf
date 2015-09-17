#include <jni.h>


#include <android/bitmap.h>
#include <android/log.h>
#include <utils/Mutex.h>

extern "C" {
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <string.h>
}

#define LOG_TAG "pdfRenderer"
#define LOGI(...)   __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...)   __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

#include <fpdfview.h>
#include <fsdk_rendercontext.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-non-virtual-dtor"
#pragma GCC diagnostic pop

#define ll_jniThrowException(env,clazz,msg) \
  jclass clsj = env->FindClass("L#clazz;");\
  if (clsj!=NULL) {\
    env->ThrowNew(clsj, msg);\
    env->DeleteLocalRef(clsj);\
  }

using namespace android;

#ifdef __cplusplus
extern "C"
{
#endif

static const int RENDER_MODE_FOR_DISPLAY = 1;
static const int RENDER_MODE_FOR_PRINT = 2;

static struct {
    jfieldID x;
    jfieldID y;
} gPointClassInfo;

static Mutex sLibraryLock;

static int sLibraryReferenceCount = 0;

static void initializeLibraryIfNeeded(){
    Mutex::Autolock lock(sLibraryLock);
    if(sLibraryReferenceCount == 0){
        LOGD("Init FPDF library");
        FPDF_InitLibrary(NULL);
    }
    sLibraryReferenceCount++;
}

static void destroyLibraryIfNeeded(){
    Mutex::Autolock lock(sLibraryLock);
    sLibraryReferenceCount--;
    if(sLibraryReferenceCount == 0){
        LOGD("Destroy FPDF library");
        FPDF_DestroyLibrary();
    }
}

static int getBlock(void* param, unsigned long position, unsigned char* outBuffer,
        unsigned long size) {
    const int fd = reinterpret_cast<intptr_t>(param);
    const int readCount = pread(fd, outBuffer, size, position);
    if (readCount < 0) {
        LOGE("Cannot read from file descriptor. Error:%d", errno);
        return 0;
    }
    return 1;
}

inline long getFileSize(int fd){
    struct stat file_state;
    if(fstat(fd, &file_state) >= 0){
        return (long)(file_state.st_size);
    }else{
        LOGE("Error getting file size");
        return 0;
    }
}
static void DropContext(void* data) {
    delete (CRenderContext*) data;
}

JNIEXPORT jlong JNICALL Java_tv_loilo_pdfium_PdfRenderer_nativeCreate
(JNIEnv *env, jobject thiz, jint fd, jlong size) {
    initializeLibraryIfNeeded();

    FPDF_FILEACCESS loader;
    loader.m_FileLen = size;
    loader.m_Param = reinterpret_cast<void *>(intptr_t(fd));
    loader.m_GetBlock = &getBlock;

    FPDF_DOCUMENT document = FPDF_LoadCustomDocument(&loader, NULL);

    if (!document) {
        const long error = FPDF_GetLastError();
        switch (error) {
            case FPDF_ERR_PASSWORD:
            case FPDF_ERR_SECURITY: {
                ll_jniThrowException(env, "java/lang/SecurityException",
                                     "cannot create document. Error:");// %ld, error);
                return -1;
            }
                break;
            default: {
                ll_jniThrowException(env, "java/io/IOException",
                                     "cannot create document. Error:");// %ld", error);
                return -1;
            }
                break;
        }
        destroyLibraryIfNeeded();
        return -1;
    }

    return reinterpret_cast<jlong>(document);
}

JNIEXPORT jlong JNICALL Java_tv_loilo_pdfium_PdfRenderer_nativeOpenPageAndGetSize
(JNIEnv *env, jobject thiz, jlong documentPtr, jint pageIndex, jdoubleArray outSize) {
    FPDF_DOCUMENT document = reinterpret_cast<FPDF_DOCUMENT>(documentPtr);

    FPDF_PAGE page = FPDF_LoadPage(document, pageIndex);

    if (!page) {
        ll_jniThrowException(env, "java/lang/IllegalStateException",
                             "cannot load page");
        return -1;
    }

    double width = 0;
    double height = 0;

    const int result = FPDF_GetPageSizeByIndex(document, pageIndex, &width, &height);

    if (!result) {
        ll_jniThrowException(env, "java/lang/IllegalStateException",
                             "cannot get page size");
        return -1;
    }

    double *outPtr = env->GetDoubleArrayElements(outSize, 0);
    outPtr[0] = width;
    outPtr[1] = height;
    env->ReleaseDoubleArrayElements(outSize,outPtr,0);

    return reinterpret_cast<jlong>(page);
}

static void renderPageBitmap(FPDF_BITMAP bitmap, FPDF_PAGE page, int destLeft, int destTop,
                             int destRight, int destBottom, float transform[], int flags) {
    // Note: this code ignores the currently unused RENDER_NO_NATIVETEXT,
    // FPDF_RENDER_LIMITEDIMAGECACHE, FPDF_RENDER_FORCEHALFTONE, FPDF_GRAYSCALE,
    // and FPDF_ANNOT flags. To add support for that refer to FPDF_RenderPage_Retail
    // in fpdfview.cpp

    CRenderContext * pContext = FX_NEW CRenderContext;

    CPDF_Page *pPage = (CPDF_Page *) page;
    pPage->SetPrivateData((void *) 1, pContext, DropContext);

    CFX_FxgeDevice * fxgeDevice = FX_NEW CFX_FxgeDevice;
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

    renderOptions->m_AddFlags = flags >> 8;
    renderOptions->m_pOCContext = new CPDF_OCContext(pPage->m_pDocument, usage);

    fxgeDevice->SaveState();

    FX_RECT clip;
    clip.left = destLeft;
    clip.right = destRight;
    clip.top = destTop;
    clip.bottom = destBottom;
    fxgeDevice->SetClip_Rect(&clip);

    CPDF_RenderContext * pageContext = FX_NEW CPDF_RenderContext;
    pContext->m_pContext = pageContext;
    pageContext->Create(pPage);

    CFX_AffineMatrix matrix;
    if (!transform) {
        pPage->GetDisplayMatrix(matrix, destLeft, destTop, destRight - destLeft,
                                destBottom - destTop, 0);
    } else {
        // PDF's coordinate system origin is left-bottom while
        // in graphics it is the top-left, so remap the origin.
        matrix.Set(1, 0, 0, -1, 0, pPage->GetPageHeight());
        // SkScalar transformValues[6];
        // transform->asAffine(transformValues);
        // matrix.Concat(transformValues[SkMatrix::kAScaleX], transformValues[SkMatrix::kASkewY],
        //         transformValues[SkMatrix::kASkewX], transformValues[SkMatrix::kAScaleY],
        //         transformValues[SkMatrix::kATransX], transformValues[SkMatrix::kATransY]);
        /*
         enum {
            kMScaleX, kMSkewX,  kMTransX,
            kMSkewY,  kMScaleY, kMTransY,
            kMPersp0, kMPersp1, kMPersp2
        };
        enum {
            kAScaleX,
            kASkewY,
            kASkewX,
            kAScaleY,
            kATransX,
            kATransY
        };
         */
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

JNIEXPORT void JNICALL Java_tv_loilo_pdfium_PdfRenderer_nativeRenderPage
(JNIEnv *env,jobject thiz, jlong documentPtr ,jlong pagePtr, jobject jbitmap,
    jint destLeft, jint destTop , jint destRight, jint destBottom , jfloatArray matrix, jint renderMode) {
AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, jbitmap, &info ) < 0) {
        LOGE("failed AndroidBitmap_getInfo");
        return;
    }
    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGD("invalid format. only ANDROID_BITMAP_FORMAT_RGBA_8888 is accepted.");
        return;
    }
    void *pixels;
    if (AndroidBitmap_lockPixels(env, jbitmap, &pixels) < 0) {
        LOGE("failed AndroidBitmap_lockPixels");
        return;
    }
    FPDF_PAGE page = reinterpret_cast<FPDF_PAGE>(pagePtr);
    FPDF_BITMAP bitmap = FPDFBitmap_CreateEx(info.width, info.height,
                                                  FPDFBitmap_BGRA, pixels, info.stride);
    if (!bitmap) {
        LOGE("Erorr creating bitmap");
        return;
    }

    float *transform = NULL;
    if (matrix) {
        size_t transform_len = env->GetArrayLength(matrix);
        if (transform_len != 9) {
            ll_jniThrowException(env, "java/lang/InvalidArgumentsException", "transform length is not 9");
            return;
        }
        transform = new float[9];
        jfloat *matrixBody = env->GetFloatArrayElements(matrix, 0);
        for (int i = 0; i < transform_len; i++) {
            transform[i] = matrixBody[i];
        }
        env->ReleaseFloatArrayElements(matrix,matrixBody,0);
    }

    int renderFlags = 0;
    if (renderMode == RENDER_MODE_FOR_DISPLAY) {
        renderFlags |= FPDF_LCD_TEXT;
    } else if (renderMode == RENDER_MODE_FOR_PRINT) {
        renderFlags |= FPDF_PRINTING;
    }
    renderPageBitmap(bitmap, page, destLeft, destTop, destRight,
            destBottom, transform, renderFlags);
    AndroidBitmap_unlockPixels(env, jbitmap);
    delete[] transform;
}

JNIEXPORT void JNICALL Java_tv_loilo_pdfium_PdfRenderer_nativeClosePage
(JNIEnv * env,jobject thiz, jlong pagePtr) {
    FPDF_PAGE page = reinterpret_cast<FPDF_PAGE>(pagePtr);
    FPDF_ClosePage(page);
}

JNIEXPORT void JNICALL Java_tv_loilo_pdfium_PdfRenderer_nativeClose
(JNIEnv* env, jobject thiz, jlong documentPtr) {
    FPDF_DOCUMENT document = reinterpret_cast<FPDF_DOCUMENT>(documentPtr);
    FPDF_CloseDocument(document);
    destroyLibraryIfNeeded();
}

JNIEXPORT jint JNICALL Java_tv_loilo_pdfium_PdfRenderer_nativeGetPageCount
(JNIEnv *env, jobject thiz, jlong documentPtr) {
    FPDF_DOCUMENT document = reinterpret_cast<FPDF_DOCUMENT>(documentPtr);
    return FPDF_GetPageCount(document);
}

JNIEXPORT jboolean JNICALL Java_tv_loilo_pdfium_PdfRenderer_nativeScaleForPrinting
(JNIEnv *env, jobject thiz, jlong documentPtr) {
    FPDF_DOCUMENT document = reinterpret_cast<FPDF_DOCUMENT>(documentPtr);
    return FPDF_VIEWERREF_GetPrintScaling(document);
}
#ifdef __cplusplus
}
#endif
