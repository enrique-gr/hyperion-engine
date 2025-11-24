#include "physics.h"

Vec3 PhysicsEngine::compute_acceleration(const State& state) {
    // 1. Basic Point Mass Gravity (Monopole)
    double r_sq = state.pos.x*state.pos.x + state.pos.y*state.pos.y + state.pos.z*state.pos.z;
    double r_mag = std::sqrt(r_sq);
    
    // F = -mu/r^3 * r_vec
    double mu_r3 = MU_EARTH / (r_sq * r_mag);
    Vec3 g_monopole = {
        -mu_r3 * state.pos.x,
        -mu_r3 * state.pos.y,
        -mu_r3 * state.pos.z
    };

    // 2. J2 Perturbation (Earth is squashed at poles)
    double z2 = state.pos.z * state.pos.z;
    double r2 = r_sq;
    double tx = state.pos.x / r_mag * (5 * z2 / r2 - 1);
    double ty = state.pos.y / r_mag * (5 * z2 / r2 - 1);
    double tz = state.pos.z / r_mag * (5 * z2 / r2 - 3);

    double j2_factor = 1.5 * J2 * MU_EARTH * (R_EARTH * R_EARTH) / (r2 * r2);
    
    Vec3 g_j2 = {
        j2_factor * tx,
        j2_factor * ty,
        j2_factor * tz
    };

    return g_monopole + g_j2; // Total Acceleration
}

void PhysicsEngine::step_rk4(State& s, double dt) {
    // RK4 Integration Strategy
    // k1 = f(y)
    // k2 = f(y + 0.5*dt*k1)
    // k3 = f(y + 0.5*dt*k2)
    // k4 = f(y + dt*k3)
    // y_new = y + (dt/6)*(k1 + 2k2 + 2k3 + k4)

    // K1
    Vec3 v1 = s.vel;
    Vec3 a1 = compute_acceleration(s);

    // K2
    State s2 = s;
    s2.pos = s.pos + v1 * (0.5 * dt);
    Vec3 v2 = s.vel + a1 * (0.5 * dt);
    Vec3 a2 = compute_acceleration(s2);

    // K3
    State s3 = s;
    s3.pos = s.pos + v2 * (0.5 * dt);
    Vec3 v3 = s.vel + a2 * (0.5 * dt);
    Vec3 a3 = compute_acceleration(s3);

    // K4
    State s4 = s;
    s4.pos = s.pos + v3 * dt;
    Vec3 v4 = s.vel + a3 * dt;
    Vec3 a4 = compute_acceleration(s4);

    // Final Integration
    s.pos = s.pos + (v1 + v2*2.0 + v3*2.0 + v4) * (dt / 6.0);
    s.vel = s.vel + (a1 + a2*2.0 + a3*2.0 + a4) * (dt / 6.0);
    
    // Integrate Attitude (Simplified Euler for brevity, usually uses similar RK4)
    s.att = s.att.integrate(s.omega, dt);
    s.time += dt;
}