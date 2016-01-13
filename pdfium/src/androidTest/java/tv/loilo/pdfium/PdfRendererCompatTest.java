package tv.loilo.pdfium;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.test.suitebuilder.annotation.SmallTest;

import org.junit.Test;
import org.junit.runner.RunWith;
import static org.junit.Assert.*;

import java.io.IOException;

@RunWith(AndroidJUnit4.class)
@SmallTest
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