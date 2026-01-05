#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include <queue>
#include <vector>
#include "webServer.h"
#include "request.h"

class LoadBalancer {
public:
    LoadBalancer(int initial_servers, std::string blocked_ip = "");

    // Firewall logic happens here before pushing to queue
    void addRequest(Request req);

    // The main heartbeat: Harvest -> Scale -> Assign
    void performCycle();

    int getQueueSize();
    int getTime();

    void printStats();

private:
    std::queue<Request> requestQueue;
    std::vector<WebServer> servers;
    int systemTime;
    
    // Tracks when we last resized the server pool
    int lastTimeChange; 

    long long int requestsFinished;
    int scaleUpCount;
    int scaleDownCount;

    // Helper functions for scaling
    void incWebServers();
    bool decWebServers();
    
    // Optional: Firewall Data
    std::string blockedIPRange;
};

#endif