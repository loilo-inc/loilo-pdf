package tv.loilo.pdfiumandroid;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ImageView;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import tv.loilo.pdfium.PdfRendererCompat;

public class MainActivity extends AppCompatActivity {

    private ImageView imageView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = (ImageView)findViewById(R.id.main_image_view);
    }

    @Override
    protected void onStart() {
        super.onStart();
        File tmp = new File(getCacheDir(), "manual.pdf");
        try {
            InputStream ins = getAssets().open("manual.pdf");
            BufferedInputStream bis = new BufferedInputStream(ins);
            FileOutputStream bos = new FileOutputStream(tmp);
            BufferedOutputStream bfw = new BufferedOutputStream(bos);
            // tmp copy
            final byte[] buffer = new byte[256];
            int read = 0;
            long len = 0;
            while ((read = bis.read(buffer)) != -1) {
                bfw.write(buffer, 0, read);
                len += read;
            }
            bfw.flush();
            ParcelFileDescriptor fd = ParcelFileDescriptor.open(tmp, ParcelFileDescriptor.MODE_READ_ONLY);
            PdfRendererCompat renderer = new PdfRendererCompat(fd);
            PdfRendererCompat.Page page1 = renderer.openPage(1);
            assert renderer.getPageCount() == 38;
            Bitmap out = Bitmap.createBitmap(page1.getWidth(), page1.getHeight(), Bitmap.Config.ARGB_8888);
            Matrix x2 = new Matrix();
            x2.setScale(2, 2);
            page1.render(out, null, x2, PdfRendererCompat.Page.RENDER_MODE_FOR_DISPLAY);
            imageView.setImageBitmap(out);
            page1.close();
            renderer.close();
            fd.close();
            ins.close();
            bis.close();
            bos.close();
            bfw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }



    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }


    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        return id == R.id.action_settings || super.onOptionsItemSelected(item);
    }
}
