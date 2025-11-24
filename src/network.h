#pragma once

// --- WINDOWS COMPATIBILITY FIX ---
#ifdef _WIN32
    // Must define Vista (0x0600) or later to use inet_pton
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0600
    #endif
    
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include "physics.h"
#include <string>

struct TelemetryPacket {
    double timestamp;
    double pos_x, pos_y, pos_z;
    double vel_x, vel_y, vel_z;
    double q_w, q_x, q_y, q_z;
};

class UdpSender {
public:
    UdpSender(const std::string& ip, int port);
    ~UdpSender();
    
    void send_state(const State& state);

private:
#ifdef _WIN32
    SOCKET sockfd;
#else
    int sockfd;
#endif
    struct sockaddr_in servaddr;
};