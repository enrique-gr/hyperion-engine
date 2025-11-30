import socket
import struct
import matplotlib.pyplot as plt
import numpy as np
from collections import deque

# --- CONFIGURATION ---
UDP_IP = "127.0.0.1"
UDP_PORT = 8080
BUFFER_SIZE = 1024  # Sufficient for our small packet
# Struct format: 11 doubles (timestamp, pos_x,y,z, vel_x,y,z, qw,qx,qy,qz)
# < = little-endian (standard for x86 PC simulation), d = double (8 bytes)
STRUCT_FMT = '<11d' 
PACKET_SIZE = struct.calcsize(STRUCT_FMT)

# Constants
R_EARTH = 6378.137  # km
MU_EARTH = 398600.4418 # km^3/s^2

# Data Storage
history_pos = deque(maxlen=500) # Store last 500 points for trail
history_energy = deque(maxlen=100)
history_time = deque(maxlen=100)

def calculate_energy(pos, vel):
    # Specific Mechanical Energy: E = v^2/2 - mu/r
    r_mag = np.linalg.norm(pos)
    v_mag = np.linalg.norm(vel)
    return (v_mag**2)/2 - MU_EARTH/r_mag

def draw_earth(ax):
    # Wireframe Earth
    u, v = np.mgrid[0:2*np.pi:20j, 0:np.pi:10j]
    x = R_EARTH * np.cos(u) * np.sin(v)
    y = R_EARTH * np.sin(u) * np.sin(v)
    z = R_EARTH * np.cos(v)
    ax.plot_wireframe(x, y, z, color='gray', alpha=0.3)

# --- SETUP PLOT ---
plt.style.use('dark_background')
fig = plt.figure(figsize=(12, 6))

# 3D Orbit View
ax3d = fig.add_subplot(1, 2, 1, projection='3d')
ax3d.set_title("Real-Time Orbit (ECEF)")
ax3d.set_xlabel("X (km)")
ax3d.set_ylabel("Y (km)")
ax3d.set_zlabel("Z (km)")

# Energy Validation Plot
ax_energy = fig.add_subplot(1, 2, 2)
ax_energy.set_title("Integrator Stability (Specific Energy)")
ax_energy.set_xlabel("Time (s)")
ax_energy.set_ylabel("Energy (km^2/s^2)")
ax_energy.grid(True, linestyle='--', alpha=0.3)

# --- NETWORK SETUP ---
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
print(f"Listening for Telemetry on {UDP_IP}:{UDP_PORT}...")

try:
    while True:
        # Blocking receive
        data, addr = sock.recvfrom(BUFFER_SIZE)
        
        if len(data) == PACKET_SIZE:
            # Unpack Binary
            unpacked = struct.unpack(STRUCT_FMT, data)
            t = unpacked[0]
            
            # Convert m to km for plotting
            pos = np.array(unpacked[1:4]) / 1000.0
            vel = np.array(unpacked[4:7]) / 1000.0
            
            # Store Data
            history_pos.append(pos)
            energy = calculate_energy(pos, vel)
            
            history_time.append(t)
            history_energy.append(energy)
            
            # --- UPDATE 3D PLOT ---
            ax3d.cla()
            draw_earth(ax3d)
            
            # Plot Trail
            xs = [p[0] for p in history_pos]
            ys = [p[1] for p in history_pos]
            zs = [p[2] for p in history_pos]
            ax3d.plot(xs, ys, zs, color='cyan', linewidth=1)
            
            # Plot Current Position
            ax3d.scatter([pos[0]], [pos[1]], [pos[2]], color='red', s=50, marker='*')
            
            # Keep view centered roughly on Earth/Sat
            ax3d.set_xlim(-8000, 8000)
            ax3d.set_ylim(-8000, 8000)
            ax3d.set_zlim(-8000, 8000)
            
            # --- UPDATE ENERGY PLOT ---
            ax_energy.cla()
            ax_energy.set_title(f"Integrator Stability (E = {energy:.4f})")
            ax_energy.plot(history_time, history_energy, color='yellow')
            
            # Dynamic scaling to show tiny jitters (if any)
            if len(history_energy) > 1:
                mean_e = np.mean(history_energy)
                ax_energy.set_ylim(mean_e - 0.1, mean_e + 0.1)

            plt.pause(0.001)

except KeyboardInterrupt:
    print("\nClosing Dashboard.")
    sock.close()