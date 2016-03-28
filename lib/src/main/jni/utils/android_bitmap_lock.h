//
// Created by pepeotoito on 2016/02/06.
//

#ifndef LOILONOTE_ANDROID_ANDROID_BITMAP_LOCK_H
#define LOILONOTE_ANDROID_ANDROID_BITMAP_LOCK_H

#include <stdexcept>
#include <android/bitmap.h>

namespace loilo {
    class android_bitmap_lock {

        JNIEnv *_env;
        jobject _bitmap;
        void *_pixels;

    public:
        android_bitmap_lock(JNIEnv *env, jobject bitmap);

        ~android_bitmap_lock();

        void inline * pixels();
    };

    android_bitmap_lock::android_bitmap_lock(JNIEnv *env, jobject bitmap)
            : _env(env), _bitmap(bitmap), _pixels(nullptr) {

        const auto result = AndroidBitmap_lockPixels(env, bitmap, &_pixels);
        if (result < 0) {
            throw std::runtime_error("Failed to lock bitmap pixels.");
        }
    }

    android_bitmap_lock::~android_bitmap_lock() {
        if (_pixels) {
            AndroidBitmap_unlockPixels(_env, _bitmap);
            _pixels = nullptr;
        }
    }

    void inline *android_bitmap_lock::pixels() {
        return _pixels;
    }
}

#endif //LOILONOTE_ANDROID_ANDROID_BITMAP_LOCK_H
