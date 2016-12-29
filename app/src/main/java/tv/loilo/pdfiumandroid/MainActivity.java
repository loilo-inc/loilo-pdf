package tv.loilo.pdfiumandroid;

import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;
import android.view.Menu;
import android.view.MenuItem;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class MainActivity extends AppCompatActivity implements PdfHost {

    private ViewPager mViewPager;

    private PdfImage mPdfImage;

    @Override
    public PdfImage getPdfImage() {
        return mPdfImage;
    }

    @Override
    public void notifyError(Throwable error) {
        error.printStackTrace();
    }

    private void preparePdfImage() {
        try {
            File pdfFile = new File(getCacheDir(), "temp.pdf");
            //アセットのParcelFileDescriptorが取得できないようなので、一度書き出す。
            //final String assetFileName = "rotation.pdf";
            final String assetFileName = "manual.pdf";

            try (final InputStream in = getAssets().open(assetFileName);
                 final FileOutputStream out = new FileOutputStream(pdfFile)) {
                FileUtils.copy(in, out);
            }
            mPdfImage = new PdfImage(pdfFile);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mViewPager = (ViewPager) findViewById(R.id.main_pdf_pager);
        preparePdfImage();
        mViewPager.setAdapter(new PdfPagerAdapter(getSupportFragmentManager(), mPdfImage.getPageCount()));
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mViewPager = null;
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
