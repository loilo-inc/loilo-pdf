package tv.loilo.pdfiumandroid;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.os.ParcelFileDescriptor;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.ImageView;
import tv.loilo.pdfium.PdfRendererCompat;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

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
        File outFile = new File(getFilesDir(), "manual.pdf");
        if (!outFile.exists()) {
            try (InputStream is = getAssets().open("manual.pdf");
                 FileOutputStream os = openFileOutput(outFile.getName(), Context.MODE_PRIVATE)
            ) {
                byte[] buffer = new byte[1024];
                int len = 0;
                while ((len = is.read(buffer)) > 0) {
                    os.write(buffer,0,len);
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        try (ParcelFileDescriptor fd = ParcelFileDescriptor.open(outFile, ParcelFileDescriptor.MODE_READ_ONLY);
             PdfRendererCompat renderer = new PdfRendererCompat(fd);
             PdfRendererCompat.Page page1 = renderer.openPage(0)
        ){
            assert renderer.getPageCount() == 38;
            Bitmap out = Bitmap.createBitmap(page1.getWidth(), page1.getHeight(), Bitmap.Config.ARGB_8888);
            Matrix x2 = new Matrix();
            x2.setScale(2,2);
            page1.render(out, null, x2, PdfRendererCompat.Page.RENDER_MODE_FOR_DISPLAY);
            imageView.setImageBitmap(out);
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
