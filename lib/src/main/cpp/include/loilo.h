//
// Created by Junpei on 2016/02/05.
//

#ifndef LOILONOTE_ANDROID_LOILO_H
#define LOILONOTE_ANDROID_LOILO_H

#include <jni.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <android/log.h>

#define LOG_TAG "loilopdf"

#define LOILOG_E(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOILOG_I(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#ifndef NDEBUG
#define LOILOG_D(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOILOG_V(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#else
#define LOILOG_D(...)
#define LOILOG_V(...)
#endif

namespace loilo {

    static inline void post_throw(JNIEnv *env, const char *name, const char *what) {
        LOILOG_E(what);
        if(env->ExceptionCheck()){
            LOILOG_E("Already throw exception.");
            return;
        }
        auto exception = env->FindClass(name);
        if(exception){
            env->ThrowNew(exception, what);
            env->DeleteLocalRef(exception);
        }
    }

    static inline void post_runtime_exception(JNIEnv *env, const char *what) {
        post_throw(env, "java/lang/RuntimeException", what);
    }

    static inline void post_arithmetic_exception(JNIEnv *env, const char *what){
        post_throw(env, "java/lang/ArithmeticException", what);
    }

    static inline void post_illegal_argument_exception (JNIEnv *env, const char *what){
        post_throw(env, "java/lang/IllegalArgumentException", what);
    }

    static inline void post_index_out_of_bounds_exception(JNIEnv *env, const char *what){
        post_throw(env, "java/lang/IndexOutOfBoundsException", what);
    }

    static inline void post_out_of_memory_error(JNIEnv *env, const char *what){
        post_throw(env, "java/lang/OutOfMemoryError", what);
    }

    static inline void post_error(JNIEnv *env, const char *what){
        post_throw(env, "java/lang/Error", what);
    }

    template<typename t_func>
    static inline void run(JNIEnv *env, t_func func) {
        try {
            func();
        } catch (const std::bad_alloc & e){
            post_out_of_memory_error(env, e.what());
        } catch (const std::invalid_argument &e) {
            post_illegal_argument_exception(env, e.what());
        } catch (const std::out_of_range & e){
            post_index_out_of_bounds_exception(env, e.what());
        } catch (const std::range_error &e){
            post_arithmetic_exception(env, e.what());
        } catch (const std::overflow_error &e){
            post_arithmetic_exception(env, e.what());
        } catch (const std::underflow_error &e){
            post_arithmetic_exception(env, e.what());
        } catch (const std::exception &e) {
            post_runtime_exception(env, e.what());
        } catch (...) {
            post_error(env, "Unexpected error.");
        }
    }

    template<typename t_result, typename t_func>
    static inline t_result call(JNIEnv *env, t_func func) {
        try {
            return func();
        } catch (const std::bad_alloc & e){
            post_out_of_memory_error(env, e.what());
            return 0;
        } catch (const std::invalid_argument &e) {
            post_illegal_argument_exception(env, e.what());
            return 0;
        } catch (const std::out_of_range & e){
            post_index_out_of_bounds_exception(env, e.what());
            return 0;
        } catch (const std::range_error &e){
            post_arithmetic_exception(env, e.what());
            return 0;
        } catch (const std::overflow_error &e){
            post_arithmetic_exception(env, e.what());
            return 0;
        } catch (const std::underflow_error &e){
            post_arithmetic_exception(env, e.what());
            return 0;
        } catch (const std::exception &e) {
            post_runtime_exception(env, e.what());
            return 0;
        } catch (...) {
            post_error(env, "Unexpected error.");
            return 0;
        }
    }
}

#endif //LOILONOTE_ANDROID_LOILO_H
