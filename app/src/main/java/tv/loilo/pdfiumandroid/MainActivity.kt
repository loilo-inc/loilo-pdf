package tv.loilo.pdfiumandroid

import android.content.Context
import android.content.res.AssetManager
import android.graphics.Bitmap
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.os.ParcelFileDescriptor
import android.view.Menu
import android.view.MenuItem
import android.view.SurfaceHolder
import android.view.SurfaceView
import tv.loilo.pdfium.PdfRenderer
import java.io.File

public class MainActivity : AppCompatActivity() {

    var holder: SurfaceHolder? = null
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    override fun onStart() {
        super.onStart()
        val surface = (findViewById(R.id.main_surface) as SurfaceView)
        holder = surface.getHolder()
        val outFile = File(getFilesDir(), "manual.pdf")
        if (!outFile.exists()) {
            getAssets().open("manual.pdf").use { inst ->
                openFileOutput(outFile.name, Context.MODE_WORLD_READABLE).use { oust ->
                    val buffer = ByteArray(1024)
                    var len: Int
                    while (true) {
                        len = inst.read(buffer)
                        if (len != -1) {
                            oust.write(buffer, 0, len)
                        } else {
                            break
                        }
                    }
                }
            }
        }
        val renderer = PdfRenderer(ParcelFileDescriptor.open(outFile, ParcelFileDescriptor.MODE_READ_ONLY))
        assert(renderer.getPageCount() == 38)
        val page1 = renderer.openPage(0)
        val out = Bitmap.createBitmap(
                page1.getWidth(),
                page1.getHeight(),
                Bitmap.Config.ARGB_8888
        )
        page1.render(out,null,null,0)
        val canvas = holder!!.lockCanvas()
        canvas.drawBitmap(out,0f,0f,null)
        holder!!.unlockCanvasAndPost(canvas)
    }

    override fun onCreateOptionsMenu(menu: Menu): Boolean {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu)
        return true
    }

    override fun onOptionsItemSelected(item: MenuItem): Boolean {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        val id = item.getItemId()

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true
        }

        return super.onOptionsItemSelected(item)
    }
}
