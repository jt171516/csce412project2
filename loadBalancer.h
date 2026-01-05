#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include <queue>
#include <vector>
#include <fstream>
#include "webServer.h"
#include "request.h"

class LoadBalancer {
public:
    LoadBalancer(int initial_servers, std::string blocked_ip = "");
    ~LoadBalancer();

    // Firewall logic happens here before pushing to queue
    void addRequest(Request req);

    // The main heartbeat: Harvest -> Scale -> Assign
    void performCycle();

    int getQueueSize();
    int getTime();

    void printStats();
    void logMessage(std::string message);

private:
    std::queue<Request> pQueue; // Processing Queue
    std::queue<Request> sQueue; // Streaming Queue

    std::vector<WebServer> pServers; // Processing Servers
    std::vector<WebServer> sServers; // Streaming Servers
    int systemTime;
    
    // Tracks when we last resized the server pool
    int lastTimeChange; 
    int nextServerID;

    // Statistics
    long long int requestsFinished;
    int blockedCount;

    // Specific counters for Processing Pool
    int scaleUpCountP;
    int scaleDownCountP;

    // Specific counters for Streaming Pool
    int scaleUpCountS;
    int scaleDownCountS;

    // Log File Object
    std::ofstream logFile;

    // Helper functions for scaling
    void incWebServers(char type);
    bool decWebServers(char type);
    
    // Optional: Firewall Data
    std::string blockedIPRange;
};

#endif