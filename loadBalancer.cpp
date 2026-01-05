#include "loadBalancer.h"
#include <iostream>

// How many cycles to wait
const int CYCLE_WAIT_TIME = 20;

// Constructor
LoadBalancer::LoadBalancer(int initial_servers, std::string blocked_ip) {
    systemTime = 0;
    lastTimeChange = 0;
    blockedIPRange = blocked_ip;

    // Create the initial set of servers
    for (int i = 0; i < initial_servers; i++) {
        servers.push_back(WebServer(i));
    }
}

// Firewall + Enqueue Logic
void LoadBalancer::addRequest(Request req) {
    // FIREWALL: Check if IP starts with the blocked range
    // .find() returns 0 if the substring is found at the very start
    if (!blockedIPRange.empty() && req.ip_in.find(blockedIPRange) == 0) {
        // Log blocked request
        std::cout << "Firewall Blocked: " << req.ip_in << std::endl;
        return; // Discard request
    }

    // If safe, add to queue
    requestQueue.push(req);
}

// Helper: Add a server (Scaling Up)
void LoadBalancer::incWebServers() {
    // Create a new server with a unique ID (current size + 1 or similar)
    int newId = servers.empty() ? 0 : servers.back().getID() + 1;
    servers.push_back(WebServer(newId));
}

// Helper: Remove a server (Scaling Down)
// Returns true if a server was actually removed (found an idle one)
bool LoadBalancer::decWebServers() {
    // Iterate through vector to find an idle server
    for (auto it = servers.begin(); it != servers.end(); ++it) {
        if (it->isRequestDone(systemTime)) { // Double check it is idle
            // Remove this specific server
            servers.erase(it);
            return true; 
        }
    }
    // No idle servers found, cannot scale down yet
    return false;
}

// The Main Logic Loop (Harvest -> Scale -> Assign)
void LoadBalancer::performCycle() {
    // 1. HARVEST: Check for finished requests
    for (auto& server : servers) {
        if (server.isRequestDone(systemTime)) {
             // Request is done. You can log it here if you want.
             Request finishedReq = server.getRequest();
             std::cout << "Server " << server.getID() << " finished request from " 
                       << finishedReq.ip_in << std::endl;
        }
    }

    // 2. SCALE: Check if we need to resize
    if (systemTime - lastTimeChange > CYCLE_WAIT_TIME) {
        
        // Scale Up Logic
        if (getQueueSize() > 25 * servers.size()) {
            incWebServers();
            lastTimeChange = systemTime;
            std::cout << "[Cycle " << systemTime << "] Scaled UP to " << servers.size() << " servers." << std::endl;
        }
        // Scale Down Logic
        else if (getQueueSize() < 15 * servers.size() && servers.size() > 1) {
            if (decWebServers()) { // Try to remove one
                lastTimeChange = systemTime;
                std::cout << "[Cycle " << systemTime << "] Scaled DOWN to " << servers.size() << " servers." << std::endl;
            }
        }
    }

    // 3. ASSIGN: Give jobs to idle servers
    // Loop through servers again to fill any empty spots (including new ones)
    for (auto& server : servers) {
        // If server is free AND we have requests waiting
        if (server.isRequestDone(systemTime) && !requestQueue.empty()) {
            
            // Get next request
            Request nextReq = requestQueue.front();
            requestQueue.pop();

            // Assign it
            server.startRequest(nextReq, systemTime);
        }
    }

    // 4. Increment Clock
    systemTime++;
}

int LoadBalancer::getQueueSize() {
    return requestQueue.size();
}

int LoadBalancer::getTime() {
    return systemTime;
}