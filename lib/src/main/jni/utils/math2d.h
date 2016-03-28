//
// Created by Junpei on 2016/02/08.
//

#ifndef LOILONOTE_ANDROID_MATH2D_H
#define LOILONOTE_ANDROID_MATH2D_H

#include <cstdint>
#include <cmath>
#include <cfloat>

namespace loilo {

    const float EPSILON_F = 1e-10f;
    const double EPSILON_D = 1e-10;

    const double COS_0 = 1.0;
    const double SIN_0 = 0.0;

    const double PI_x_2 = M_PI * 2.0;

    static inline bool is_almost_zero(float value) {
        return std::abs(value) < EPSILON_F;
    }

    static inline bool is_almost_zero(double value) {
        return std::abs(value) < EPSILON_D;
    }

    static inline int32_t clamp(int32_t value, int32_t min, int32_t max) {
        return std::min(std::max(value, min), max);
    }

    static inline uint32_t clamp(uint32_t value, uint32_t min, uint32_t max) {
        return std::min(std::max(value, min), max);
    }

    static inline float clamp(float value, float min, float max) {
        return std::min(std::max(value, min), max);
    }

    static inline double clamp(double value, double min, double max) {
        return std::min(std::max(value, min), max);
    }

    static inline float lerp(float from, float to, float point) {
        return from + (to - from) * point;
    }

    static inline double lerp(double from, double to, double point) {
        return from + (to - from) * point;
    }

    static inline float vector_dot(float x0, float y0, float x1, float y1) {
        return (x0 * x1) + (y0 * y1);
    }

    static inline double vector_dot(double x0, double y0, double x1, double y1) {
        return (x0 * x1) + (y0 * y1);
    }

    static inline float vector_cross(float x0, float y0, float x1, float y1) {
        return (x0 * y1) - (y0 * x1);
    }

    static inline double vector_cross(double x0, double y0, double x1, double y1) {
        return (x0 * y1) - (y0 * x1);
    }

    static inline float vector_length_sq(float x, float y) {
        return vector_dot(x, y, x, y);
    }

    static inline double vector_length_sq(double x, double y) {
        return vector_dot(x, y, x, y);
    }

    static inline float vector_length(float x, float y) {
        return std::sqrt(vector_length_sq(x, y));
    }

    static inline double vector_length(double x, double y) {
        return std::sqrt(vector_length_sq(x, y));
    }

    static inline double to_radians(double degrees) {
        return degrees / 180.0 * M_PI;
    }

    static inline double to_degrees(double radians) {
        return radians * 180.0 / M_PI;
    }

    static inline double vector_degrees(double dot, double len, bool is_large) {
        const double degrees = to_degrees(std::acos(clamp(dot / len, -1.0, 1.0)));
        if (!is_large) {
            return degrees;
        }
        return 360.0 - degrees;
    }

    static inline double vector_degrees(double x0, double y0, double x1, double y1, bool is_large) {
        const double len0_x_len1 = vector_length(x0, y0) * vector_length(x1, y1);
        if (is_almost_zero(len0_x_len1)) {
            return 0.0;
        }
        const double dot = vector_dot(x0, y0, x1, y1);
        return vector_degrees(dot, len0_x_len1, is_large);
    }

    static inline float catmull_rom(float p1, float p2, float p3, float p4, float s) {
        //http://msdn.microsoft.com/en-us/library/windows/desktop/bb205486(v=vs.85).aspx

        //v1 = p2
        //v2 = p3
        //t1 = (p3 - p1) / 2
        //t2 = (p4 - p2) / 2
        //Q(s) = (2s3 - 3s2 + 1)v1 + (-2s3 + 3s2)v2 + (s3 - 2s2 + s)t1 + (s3 - s2)t2
        // ↓
        //Q(s) = (2s3 - 3s2 + 1)p2 + (-2s3 + 3s2)p3 + (s3 - 2s2 + s)(p3 - p1) / 2 + (s3 - s2)(p4 - p2)/2
        // ↓
        //Q(s) = [(-s3 + 2s2 - s)p1 + (3s3 - 5s2 + 2)p2 + (-3s3 + 4s2 + s)p3 + (s3 - s2)p4] / 2

        const float s2 = s * s;
        const float s3 = s2 * s;
        return ((-s3 + 2.f * s2 - s) * p1 + (3.f * s3 - 5.f * s2 + 2.f) * p2 +
                (-3.f * s3 + 4.f * s2 + s) * p3 + (s3 - s2) * p4) * 0.5f;
    }

    static inline double catmull_rom(double p1, double p2, double p3, double p4, double s) {
        //http://msdn.microsoft.com/en-us/library/windows/desktop/bb205486(v=vs.85).aspx

        //v1 = p2
        //v2 = p3
        //t1 = (p3 - p1) / 2
        //t2 = (p4 - p2) / 2
        //Q(s) = (2s3 - 3s2 + 1)v1 + (-2s3 + 3s2)v2 + (s3 - 2s2 + s)t1 + (s3 - s2)t2
        // ↓
        //Q(s) = (2s3 - 3s2 + 1)p2 + (-2s3 + 3s2)p3 + (s3 - 2s2 + s)(p3 - p1) / 2 + (s3 - s2)(p4 - p2)/2
        // ↓
        //Q(s) = [(-s3 + 2s2 - s)p1 + (3s3 - 5s2 + 2)p2 + (-3s3 + 4s2 + s)p3 + (s3 - s2)p4] / 2

        const double s2 = s * s;
        const double s3 = s2 * s;
        return ((-s3 + 2.0 * s2 - s) * p1 + (3.0 * s3 - 5.0 * s2 + 2.0) * p2 +
                (-3.0 * s3 + 4.0 * s2 + s) * p3 + (s3 - s2) * p4) * 0.5;
    }

    static inline bool rect_intersects(float left0, float top0, float right0, float bottom0, float left1, float top1, float right1, float bottom1){
        return left0 < right1 && left1 < right0 && top0 < bottom1 && top1 < bottom0;
    }
}

#endif //LOILONOTE_ANDROID_MATH2D_H
