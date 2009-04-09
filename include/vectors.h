//============================================================================//
//============================================================================//

#ifndef VECTORS_H
#define VECTORS_H


#include <math.h>
#include <stdio.h>
#include <string>


template <typename T> T typed_cos(T rads);
template<> inline float  typed_cos<float> (float  rads) { return cosf(rads); }
template<> inline double typed_cos<double>(double rads) { return cos(rads);  }

template <typename T> T typed_sin(T rads);
template<> inline float  typed_sin<float> (float  rads) { return sinf(rads); }
template<> inline double typed_sin<double>(double rads) { return sin(rads);  }

template <typename T> T typed_sqrt(T rads);
template<> inline float  typed_sqrt<float> (float  rads) { return sqrtf(rads); }
template<> inline double typed_sqrt<double>(double rads) { return sqrt(rads);  }

template <typename T> inline std::string tostring(T value, const char* fmt) {
  char buf[256];
  if (fmt == NULL) { fmt = "%.9g"; }
  snprintf(buf, sizeof(buf), fmt, (double)value);
  return std::string(buf);
}


//============================================================================//
//============================================================================//
//
//  vec2
//

template <typename T>
class vec2 {
  public:
    T x, y;

  public:
    vec2()              : x((T)0), y((T)0) {}
    vec2(const vec2& v) : x(v.x),  y(v.y)  {}
    vec2(const T v[2])  : x(v[0]), y(v[1]) {}
    vec2(T _x, T _y)    : x(_x),   y(_y)   {}

    inline vec2& operator=(const vec2& v) { x = v.x; y = v.y; return *this; }

    inline       T* data()       { return &x; }
    inline const T* data() const { return &x; }

    inline operator       T*()       { return &x; }
    inline operator const T*() const { return &x; }

    inline       T& operator[](int index)       { return ((T*)&x)[index]; }
    inline const T& operator[](int index) const { return ((T*)&x)[index]; }

    inline vec2 operator-() const { return vec2(-x, -y); }

    vec2& operator+=(const vec2& v) { x += v.x; y += v.y; return *this; }
    vec2& operator-=(const vec2& v) { x -= v.x; y -= v.y; return *this; }
    vec2& operator*=(const vec2& v) { x *= v.x; y *= v.y; return *this; }
    vec2& operator/=(const vec2& v) { x /= v.x; y /= v.y; return *this; }

    vec2& operator+=(T s) { x += s; y += s; return *this; }
    vec2& operator-=(T s) { x -= s; y -= s; return *this; }
    vec2& operator*=(T s) { x *= s; y *= s; return *this; }
    vec2& operator/=(T s) { x /= s; y /= s; return *this; }

    vec2 operator+(const vec2& v) const { return vec2(x + v.x, y + v.y); }
    vec2 operator-(const vec2& v) const { return vec2(x - v.x, y - v.y); }
    vec2 operator*(const vec2& v) const { return vec2(x * v.x, y * v.y); }
    vec2 operator/(const vec2& v) const { return vec2(x / v.x, y / v.y); }

    vec2 operator+(T s) const { return vec2(x + s, y + s); }
    vec2 operator-(T s) const { return vec2(x - s, y - s); }
    vec2 operator*(T s) const { return vec2(x * s, y * s); }
    vec2 operator/(T s) const { return vec2(x / s, y / s); }

    bool operator<(const vec2& v) const {
      if (x < v.x) { return true;  }
      if (x > v.x) { return false; }
      if (y < v.y) { return true;  }
      if (y > v.y) { return false; }
      return false;
    }
    bool operator==(const vec2& v) const {
      return ((x == v.x) && (y == v.y));
    }
    bool operator!=(const vec2& v) const {
      return ((x != v.x) || (y != v.y));
    }

    static T dot(const vec2& a, const vec2& b) {
      return ((a.x * b.x) + (a.y * b.y));
    }
    T dot(const vec2& v) const {
      return dot(*this, v);
    }

    T lenSqr() const { return dot(*this, *this);    }
    T length() const { return typed_sqrt(lenSqr()); }

    std::string tostring(const char* fmt = NULL, const char* sep = NULL) const {
      if (sep == NULL) { sep = " "; }
      return ::tostring(x, fmt) + sep +
             ::tostring(y, fmt);
    }
};


template <typename T>
vec2<T> operator*(T s, const vec2<T>& in) { vec2<T> v(in); v *= s; return v; }


//============================================================================//
//============================================================================//
//
//  vec3
//

template <typename T>
class vec3 {
  public:
    T x, y, z;

  public:
    vec3()                       : x((T)0), y((T)0), z((T)0) {}
    vec3(const vec3& v)          : x(v.x),  y(v.y),  z(v.z)  {}
    vec3(const T v[3])           : x(v[0]), y(v[1]), z(v[2]) {}
    vec3(T _x, T _y, T _z)       : x(_x),   y(_y),   z(_z)   {}
    vec3(const vec2<T>& v, T _z) : x(v.x),  y(v.y),  z(_z)   {}

    inline vec3& operator=(const vec3& v) {
      x = v.x; y = v.y; z = v.z; return *this;
    }

    inline       T* data()       { return &x; }
    inline const T* data() const { return &x; }

    inline operator       T*()       { return &x; }
    inline operator const T*() const { return &x; }

    inline       T& operator[](int index)       { return ((T*)&x)[index]; }
    inline const T& operator[](int index) const { return ((T*)&x)[index]; }

    inline vec3 operator-() const { return vec3(-x, -y, -z); }

    vec3& operator+=(const vec3& v) {
      x += v.x; y += v.y; z += v.z; return *this;
    }
    vec3& operator-=(const vec3& v) {
      x -= v.x; y -= v.y; z -= v.z; return *this;
    }
    vec3& operator*=(const vec3& v) {
      x *= v.x; y *= v.y; z *= v.z; return *this;
    }
    vec3& operator/=(const vec3& v) {
      x /= v.x; y /= v.y; z /= v.z; return *this;
    }
    vec3& operator+=(T s) { x += s; y += s; z += s; return *this; }
    vec3& operator-=(T s) { x -= s; y -= s; z -= s; return *this; }
    vec3& operator*=(T s) { x *= s; y *= s; z *= s; return *this; }
    vec3& operator/=(T s) { x /= s; y /= s; z /= s; return *this; }

    vec3 operator+(const vec3& v) const {
      return vec3(x + v.x, y + v.y, z + v.z);
    }
    vec3 operator-(const vec3& v) const {
      return vec3(x - v.x, y - v.y, z - v.z);
    }
    vec3 operator*(const vec3& v) const {
      return vec3(x * v.x, y * v.y, z * v.z);
    }
    vec3 operator/(const vec3& v) const {
      return vec3(x / v.x, y / v.y, z / v.z);
    }

    vec3 operator+(T s) const { return vec3(x + s, y + s, z + s); }
    vec3 operator-(T s) const { return vec3(x - s, y - s, z - s); }
    vec3 operator*(T s) const { return vec3(x * s, y * s, z * s); }
    vec3 operator/(T s) const { return vec3(x / s, y / s, z / s); }

    bool operator<(const vec3& v) const {
      if (x < v.x) { return true;  }
      if (x > v.x) { return false; }
      if (y < v.y) { return true;  }
      if (y > v.y) { return false; }
      if (z < v.z) { return true;  }
      if (z > v.z) { return false; }
      return false;
    }
    bool operator==(const vec3& v) const {
      return ((x == v.x) && (y == v.y) && (z == v.z));
    }
    bool operator!=(const vec3& v) const {
      return ((x != v.x) || (y != v.y) || (z != v.z));
    }

    static T dot(const vec3& a, const vec3& b) {
      return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z));
    }
    T dot(const vec3& v) const {
      return dot(*this, v);
    }

    static vec3 cross(const vec3& a, const vec3& b) {
      return vec3(
        ((a.y * b.z) - (a.z * b.y)),
        ((a.z * b.x) - (a.x * b.z)),
        ((a.x * b.y) - (a.y * b.x))
      );
    }
    vec3 cross(const vec3& v) const {
      return cross(*this, v);
    }

    T lenSqr() const { return dot(*this, *this);    }
    T length() const { return typed_sqrt(lenSqr()); }

    static bool normalize(vec3& v) {
      const T len = v.length();
      if (len == (T)0) {
        return false;
      }
      const T scale = ((T)1 / len);
      v *= scale;
      return true;
    }

    vec3 normalize() const {
      vec3 v(*this); normalize(v); return v;
    }

    std::string tostring(const char* fmt = NULL, const char* sep = NULL) const {
      if (sep == NULL) { sep = " "; }
      return ::tostring(x, fmt) + sep +
             ::tostring(y, fmt) + sep +
             ::tostring(z, fmt);
    }
};


template <typename T>
vec3<T> operator*(T s, const vec3<T>& in) { vec3<T> v(in); v *= s; return v; }


//============================================================================//
//============================================================================//
//
//  vec4
//

template <typename T>
class vec4 {
  public:
    T x, y, z, w;

  public:
    vec4()                       : x((T)0), y((T)0), z((T)0), w((T)1) {}
    vec4(const vec4& v)          : x(v.x),  y(v.y),  z(v.z),  w(v.w)  {}
    vec4(const T v[4])           : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}
    vec4(T _x, T _y, T _z, T _w) : x(_x),   y(_y),   z(_z),   w(_w)   {}
    vec4(const vec3<T>& v, T _w) : x(v.x),  y(v.y),  z(v.z),  w(_w)   {}

    inline vec4& operator=(const vec4& v) {
      x = v.x; y = v.y; z = v.z; w = v.w; return *this;
    }

    inline       T* data()       { return &x; }
    inline const T* data() const { return &x; }

    inline operator       T*()       { return &x; }
    inline operator const T*() const { return &x; }

    inline       T& operator[](int index)       { return ((T*)&x)[index]; }
    inline const T& operator[](int index) const { return ((T*)&x)[index]; }

    inline vec4 operator-() const { return vec4(-x, -y, -z, -w); }

    vec4& operator+=(const vec4& v) {
      x += v.x; y += v.y; z += v.z; w += v.w; return *this;
    }
    vec4& operator-=(const vec4& v) {
      x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this;
    }
    vec4& operator*=(const vec4& v) {
      x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this;
    }
    vec4& operator/=(const vec4& v) {
      x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this;
    }

    vec4& operator+=(T s) { x += s; y += s; z += s; w += s; return *this; }
    vec4& operator-=(T s) { x -= s; y -= s; z -= s; w -= s; return *this; }
    vec4& operator*=(T s) { x *= s; y *= s; z *= s; w *= s; return *this; }
    vec4& operator/=(T s) { x /= s; y /= s; z /= s; w /= s; return *this; }

    vec4 operator+(const vec4& v) const {
      return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
    }
    vec4 operator-(const vec4& v) const {
      return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
    }
    vec4 operator*(const vec4& v) const {
      return vec4(x * v.x, y * v.y, z * v.z, w * v.w);
    }
    vec4 operator/(const vec4& v) const {
      return vec4(x / v.x, y / v.y, z / v.z, w / v.w);
    }

    vec4 operator+(T s) const { return vec4(x + s, y + s, z + s, w + s); }
    vec4 operator-(T s) const { return vec4(x - s, y - s, z - s, w - s); }
    vec4 operator*(T s) const { return vec4(x * s, y * s, z * s, w * s); }
    vec4 operator/(T s) const { return vec4(x / s, y / s, z / s, w / s); }

    bool operator<(const vec4& v) const {
      if (x < v.x) { return true;  }
      if (x > v.x) { return false; }
      if (y < v.y) { return true;  }
      if (y > v.y) { return false; }
      if (z < v.z) { return true;  }
      if (z > v.z) { return false; }
      if (w < v.w) { return true;  }
      if (w > v.w) { return false; }
      return false;
    }
    bool operator==(const vec4& v) const {
      return ((x == v.x) && (y == v.y) && (z == v.z) && (w == v.w));
    }
    bool operator!=(const vec4& v) const {
      return ((x != v.x) || (y != v.y) || (z != v.z) || (w != v.w));
    }

    static T dot(const vec4& a, const vec4& b) {
      return ((a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w));
    }
    T dot(const vec4& v) const {
      return dot(*this, v);
    }

    T lenSqr() const { return dot(*this, *this);    }
    T length() const { return typed_sqrt(lenSqr()); }

    std::string tostring(const char* fmt = NULL, const char* sep = NULL) const {
      if (sep == NULL) { sep = " "; }
      return ::tostring(x, fmt) + sep +
             ::tostring(y, fmt) + sep +
             ::tostring(z, fmt) + sep +
             ::tostring(w, fmt);
    }
};


template <typename T>
vec4<T> operator*(T s, const vec4<T>& in) { vec4<T> v(in); v *= s; return v; }


//============================================================================//
//============================================================================//
//
//  Easier to type
//

typedef vec2<float> fvec2;
typedef vec3<float> fvec3;
typedef vec4<float> fvec4;

typedef vec2<double> dvec2;
typedef vec3<double> dvec3;
typedef vec4<double> dvec4;


//============================================================================//
//============================================================================//

#endif // VECTORS_H

//============================================================================//
//============================================================================//

