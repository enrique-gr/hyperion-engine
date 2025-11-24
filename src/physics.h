#pragma once
#include "math_core.h"

// Standard gravitational parameter for Earth (m^3/s^2)
const double MU_EARTH = 3.986004418e14; 
// Earth radius (m)
const double R_EARTH = 6378137.0;
// J2 Perturbation Constant (Earth Oblateness)
const double J2 = 1.08262668e-3;

struct State {
    Vec3 pos;   // ECEF Position (m)
    Vec3 vel;   // ECEF Velocity (m/s)
    Quat att;   // Attitude (Quaternion)
    Vec3 omega; // Angular Rates (rad/s)
    double time; // Mission Time (s)
};

// Derivative struct for RK4
struct StateDerivative {
    Vec3 d_pos;
    Vec3 d_vel;
};

class PhysicsEngine {
public:
    // Runge-Kutta 4 Integration Step
    static void step_rk4(State& state, double dt);

private:
    // Calculate Acceleration based on state (Gravity + J2)
    static Vec3 compute_acceleration(const State& state);
};