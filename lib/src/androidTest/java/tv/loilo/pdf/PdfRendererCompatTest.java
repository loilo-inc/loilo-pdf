package tv.loilo.pdf;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.io.IOException;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotSame;

@RunWith(AndroidJUnit4.class)
public class PdfRendererCompatTest {

    private Context getContext() {
        return InstrumentationRegistry.getContext();
    }

    @Test
    public void testAssert() {
        assertEquals(0, 0);
    }

    @Test
    public void testOpenPage() throws IOException, IllegalAccessException {
        AssetManager assets = getContext().getAssets();
        for (String file : assets.list(".")) {
            try (AssetFileDescriptor fd = assets.openFd(file);
                 PdfRendererCompat renderer = new PdfRendererCompat(fd.getParcelFileDescriptor())
            ) {
                for (int i = 0; i < renderer.getPageCount(); i++) {
                    PdfRendererCompat.Page page = renderer.openPage(i);
                    assertNotSame(0, page.getWidth());
                    assertNotSame(0, page.getHeight());
                    assertEquals(i, page.getIndex());
                }
            }
        }
    }
}