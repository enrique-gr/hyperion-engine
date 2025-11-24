#pragma once
#include <cmath>
#include <iostream>

// --- VECTOR 3 ---
struct Vec3 {
    double x, y, z;

    Vec3 operator+(const Vec3& other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3 operator-(const Vec3& other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3 operator*(double scalar) const { return {x * scalar, y * scalar, z * scalar}; }
    
    // Cross Product
    Vec3 cross(const Vec3& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    double magnitude() const { return std::sqrt(x*x + y*y + z*z); }
    
    Vec3 normalized() const {
        double m = magnitude();
        if (m < 1e-9) return {0,0,0};
        return {x/m, y/m, z/m};
    }
};

// --- QUATERNION ---
struct Quat {
    double w, x, y, z;

    // Normalize to ensure valid rotation
    void normalize() {
        double mag = std::sqrt(w*w + x*x + y*y + z*z);
        w /= mag; x /= mag; y /= mag; z /= mag;
    }

    // Quaternion Integration from Angular Velocity (omega)
    // dq/dt = 0.5 * q * omega
    Quat integrate(const Vec3& omega, double dt) const {
        // Pure quaternion from omega
        double qw = 0;
        double qx = omega.x;
        double qy = omega.y;
        double qz = omega.z;

        // Quaternion multiplication: q_new = q_old + 0.5 * (q_old * omega_quat) * dt
        double nw = w + 0.5 * (-x*qx - y*qy - z*qz) * dt;
        double nx = x + 0.5 * ( w*qx + y*qz - z*qy) * dt;
        double ny = y + 0.5 * ( w*qy - x*qz + z*qx) * dt;
        double nz = z + 0.5 * ( w*qz + x*qy - y*qx) * dt;

        Quat result = {nw, nx, ny, nz};
        result.normalize();
        return result;
    }
};