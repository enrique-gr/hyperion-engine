#include "network.h"
#include <cstring>
#include <iostream>

UdpSender::UdpSender(const std::string& ip, int port) {
#ifdef _WIN32
    // Windows: Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[NET] WSAStartup failed" << std::endl;
        return;
    }
    
    // Windows Socket Creation
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        std::cerr << "[NET] Socket creation failed" << std::endl;
        return;
    }
#else
    // Linux/Mac Socket Creation
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "[NET] Socket creation failed" << std::endl;
        return;
    }
#endif

    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    
    // Cross-platform IP address conversion
#ifdef _WIN32
    inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
#else
    servaddr.sin_addr.s_addr = inet_addr(ip.c_str());
#endif
}

UdpSender::~UdpSender() {
#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
}

void UdpSender::send_state(const State& state) {
    TelemetryPacket pkt;
    pkt.timestamp = state.time;
    
    pkt.pos_x = state.pos.x;
    pkt.pos_y = state.pos.y;
    pkt.pos_z = state.pos.z;
    
    pkt.vel_x = state.vel.x;
    pkt.vel_y = state.vel.y;
    pkt.vel_z = state.vel.z;

    pkt.q_w = state.att.w;
    pkt.q_x = state.att.x;
    pkt.q_y = state.att.y;
    pkt.q_z = state.att.z;

    // Cast to (const char*) for Windows compatibility
    sendto(sockfd, (const char*)&pkt, sizeof(pkt), 0, 
           (const struct sockaddr *) &servaddr, sizeof(servaddr));
}