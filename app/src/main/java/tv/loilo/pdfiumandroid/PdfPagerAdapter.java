package tv.loilo.pdfiumandroid;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;

final class PdfPagerAdapter extends FragmentPagerAdapter {

    private final int mPageCount;

    PdfPagerAdapter(FragmentManager fm, int pageCount) {
        super(fm);
        mPageCount = pageCount;
    }

    @Override
    public Fragment getItem(int position) {
        return PdfPageFragment.newInstance(position);
    }

    @Override
    public int getCount() {
        return mPageCount;
    }


}
