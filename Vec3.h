#include <cmath>

struct Vec3
{
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Vector operations
    Vec3 operator+(const Vec3& v) const
    {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }

    Vec3 operator-(const Vec3& v) const
    {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }

    Vec3 operator*(float s) const
    {
        return Vec3(x * s, y * s, z * s);
    }

    // Returns normalized version of the vector
    Vec3 normalize() const
    {
        float mag = sqrt(x * x + y * y + z * z);
        return Vec3(x / mag, y / mag, z / mag);
    }

    // Dot product calculation
    float dot(const Vec3& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }
};