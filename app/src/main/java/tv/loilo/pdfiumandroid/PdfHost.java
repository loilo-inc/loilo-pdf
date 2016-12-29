package tv.loilo.pdfiumandroid;

interface PdfHost {
    PdfImage getPdfImage();
    void notifyError(Throwable error);
}
