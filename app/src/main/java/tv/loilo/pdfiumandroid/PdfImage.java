package tv.loilo.pdfiumandroid;

import android.graphics.Bitmap;
import android.os.ParcelFileDescriptor;
import android.support.v4.util.LruCache;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicBoolean;

import tv.loilo.pdf.PdfRendererCompat;
import tv.loilo.promise.Defer;
import tv.loilo.promise.Deferred;
import tv.loilo.promise.Promise;
import tv.loilo.promise.Promises;
import tv.loilo.promise.Scheduler;
import tv.loilo.promise.WhenCallback;
import tv.loilo.promise.WhenParams;

final class PdfImage implements Closeable {

    private final ParcelFileDescriptor mFd;
    private final PdfRendererCompat mRenderer;
    private final Scheduler mScheduler;
    private final AtomicBoolean mIsClosed;
    private final LruCache<Integer, Bitmap> mCache;

    private PdfImage(ParcelFileDescriptor fd) throws IOException {
        mFd = fd;
        mRenderer = new PdfRendererCompat(mFd);
        mScheduler = new Scheduler(1);
        mIsClosed = new AtomicBoolean();
        mCache = new LruCache<>(3);
    }

    PdfImage(File file) throws IOException {
        this(ParcelFileDescriptor.open(file, ParcelFileDescriptor.MODE_READ_ONLY));
    }

    public int getPageCount() {
        return mRenderer.getPageCount();
    }

    private Bitmap getPageAt(int index) {
        try (final PdfRendererCompat.Page page = mRenderer.openPage(index)) {
            final Bitmap bitmap = Bitmap.createBitmap(page.getWidth(), page.getHeight(), Bitmap.Config.ARGB_8888);
            page.render(bitmap, null, null, PdfRendererCompat.Page.RENDER_MODE_FOR_DISPLAY);
            return bitmap;
        }
    }

    Promise<Bitmap> promiseGetPageAt(final int index) {
        return Promises.when(new WhenCallback<Bitmap>() {
            @Override
            public Deferred<Bitmap> run(WhenParams params) throws Exception {
                if (mIsClosed.get()) {
                    throw new RuntimeException("Already closed.");
                }
                Bitmap bitmap = mCache.get(index);
                if (bitmap == null) {
                    bitmap = getPageAt(index);
                    mCache.put(index, bitmap);
                }
                return Defer.success(bitmap);
            }
        }).promiseOn(mScheduler);
    }

    @Override
    public void close() throws IOException {
        Promises.when(new WhenCallback<Void>() {
            @Override
            public Deferred<Void> run(WhenParams params) throws Exception {
                if (!mIsClosed.getAndSet(true)) {
                    mRenderer.close();
                    mFd.close();
                    mCache.evictAll();
                }
                return Defer.success(null);
            }
        }).submitOn(mScheduler);
    }
}
