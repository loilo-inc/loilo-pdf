package tv.loilo.pdfiumandroid;

import android.support.annotation.NonNull;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

class FileUtils {

    private static final int DEFAULT_BUFFER_SIZE = 8192;

    public static long copy(@NonNull final InputStream in, @NonNull final OutputStream out) throws IOException {
        final byte[] buffer = new byte[DEFAULT_BUFFER_SIZE];
        long length = 0;
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
            length += read;
        }
        out.flush();
        return length;
    }
}
