package tv.loilo.pdfiumandroid;


import android.graphics.Bitmap;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.content.Loader;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;

import tv.loilo.promise.Dispatcher;
import tv.loilo.promise.Promise;
import tv.loilo.promise.Result;
import tv.loilo.promise.support.PromiseLoader;
import tv.loilo.promise.support.PromiseLoaderCallbacks;

public final class PdfPageFragment extends Fragment {
    private static final String TAG_PAGE_INDEX = "page_index";

    private int mPageIndex;

    private PdfHost getPdfHost() {
        return (PdfHost) getActivity();
    }

    private ImageView mImageView;

    private final PromiseLoaderCallbacks<Bitmap> mLoaderCallbacks = new PromiseLoaderCallbacks<Bitmap>() {
        @Override
        public Loader<Result<Bitmap>> onCreateLoader(int id, Bundle args) {
            final PdfHost pdfHost = getPdfHost();
            return new PromiseLoader<Bitmap>(getContext()) {
                @NonNull
                @Override
                protected Promise<Bitmap> onCreatePromise() throws Exception {
                    return pdfHost.getPdfImage().promiseGetPageAt(mPageIndex);
                }
            };
        }

        @Override
        public void onLoadFinished(Loader<Result<Bitmap>> loader, final Result<Bitmap> data) {
            Dispatcher.getMainDispatcher().post(new Runnable() {
                @Override
                public void run() {
                    if (isResumed()) {
                        if (data.getCancelToken().isCanceled()) {
                            return;
                        }
                        final Exception e = data.getException();
                        if (e != null) {
                            getPdfHost().notifyError(e);
                            return;
                        }
                        mImageView.setImageBitmap(data.getValue());

                    }
                }
            });
        }

        @Override
        public void onLoaderReset(Loader<Result<Bitmap>> loader) {

        }
    };


    public PdfPageFragment() {
    }

    public static PdfPageFragment newInstance(int pageIndex) {
        PdfPageFragment fragment = new PdfPageFragment();
        Bundle args = new Bundle();
        args.putInt(TAG_PAGE_INDEX, pageIndex);
        fragment.setArguments(args);
        return fragment;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            mPageIndex = getArguments().getInt(TAG_PAGE_INDEX);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        final View contentView = inflater.inflate(R.layout.fragment_pdf_page, container, false);
        mImageView = (ImageView) contentView.findViewById(R.id.pdf_page_image_view);
        return contentView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        mImageView = null;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);

        getLoaderManager().initLoader(0, Bundle.EMPTY, mLoaderCallbacks);
    }
}
