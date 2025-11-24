#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include "physics.h"
#include "network.h"

// --- SHARED RESOURCES ---
std::atomic<bool> running(true);
std::mutex state_mutex;
State shared_state;

// --- CONFIGURATION ---
const double PHYSICS_DT = 0.0001; // 10 kHz Physics
const int TELEMETRY_PORT = 8080;
const std::string TELEMETRY_IP = "127.0.0.1";

// --- PHYSICS THREAD ---
void physics_loop() {
    std::cout << "[PHYSICS] Thread initializing @ " << (1.0/PHYSICS_DT) << " Hz" << std::endl;
    
    // Initial Conditions: Low Earth Orbit (LEO)
    State local_state;
    local_state.pos = {R_EARTH + 400000, 0, 0}; // 400km Altitude
    local_state.vel = {0, 7660, 0};             // Orbital Velocity
    local_state.att = {1, 0, 0, 0};
    local_state.omega = {0.01, 0.05, 0.001};    // Slow tumble
    local_state.time = 0.0;

    while (running) {
        auto start = std::chrono::high_resolution_clock::now();

        // 1. Step Physics
        PhysicsEngine::step_rk4(local_state, PHYSICS_DT);

        // 2. Update Shared State (Thread Safe)
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            shared_state = local_state;
        }

        // 3. Busy Wait for precision timing (Spinlock)
        while (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() < PHYSICS_DT) {
            // Spin
        }
    }
    std::cout << "[PHYSICS] Thread stopping..." << std::endl;
}

// --- COMMS THREAD ---
void comms_loop() {
    std::cout << "[COMMS] UDP Stream target: " << TELEMETRY_IP << ":" << TELEMETRY_PORT << std::endl;
    UdpSender sender(TELEMETRY_IP, TELEMETRY_PORT);

    while (running) {
        State snapshot;
        
        // 1. Read Shared State
        {
            std::lock_guard<std::mutex> lock(state_mutex);
            snapshot = shared_state;
        }

        // 2. Send Packet
        sender.send_state(snapshot);

        // 3. Log to Console (1 Hz)
        static int counter = 0;
        if (counter++ % 50 == 0) {
            double alt_km = (snapshot.pos.magnitude() - R_EARTH) / 1000.0;
            std::cout << "T+" << snapshot.time << "s | ALT: " << alt_km << " km | V: " << snapshot.vel.magnitude() << " m/s" << std::endl;
        }

        // 50 Hz Telemetry Rate
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::cout << "[COMMS] Thread stopping..." << std::endl;
}

int main() {
    std::cout << "==========================================" << std::endl;
    std::cout << "   HYPERION 6-DOF SIMULATION ENGINE       " << std::endl;
    std::cout << "==========================================" << std::endl;

    // If you see errors here, ensure you are using MSVC or MinGW-w64 (POSIX threads)
    std::thread t_phys(physics_loop);
    std::thread t_comm(comms_loop);

    std::cout << "Press ENTER to abort simulation..." << std::endl;
    std::cin.get();

    running = false;
    t_phys.join();
    t_comm.join();

    return 0;
}