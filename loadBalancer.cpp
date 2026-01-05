#include "loadBalancer.h"
#include <iostream>

// How many cycles to wait
const int CYCLE_WAIT_TIME = 20;

// Constructor
LoadBalancer::LoadBalancer(int initial_servers, std::string blocked_ip) {
    systemTime = 0;
    lastTimeChange = 0;
    blockedIPRange = blocked_ip;
    nextServerID = 0;

    // Initialize Stats
    requestsFinished = 0;
    scaleUpCountP = 0;
    scaleDownCountP = 0;
    scaleUpCountS = 0;
    scaleDownCountS = 0;
    blockedCount = 0;

    // Open the log file
    logFile.open("load_balancer_log.txt"); // <--- Creates/Overwrites file
    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open log file!" << std::endl;
    } else {
        logFile << "--- Simulation Log Started ---" << std::endl;
    }

    // Create the initial set of servers
    for (int i = 0; i < initial_servers; i++) {
        if (i % 2 == 0) {
            pServers.push_back(WebServer(nextServerID++));
        } else {
            sServers.push_back(WebServer(nextServerID++));
        }
    }
}

LoadBalancer::~LoadBalancer() {
    if (logFile.is_open()) {
        logFile << "--- Simulation Log Ended ---" << std::endl;
        logFile.close();
    }
}

// Helper to write to both console and file
void LoadBalancer::logMessage(std::string message) {
    std::cout << message << std::endl;
    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
}

// Firewall + Enqueue Logic
void LoadBalancer::addRequest(Request req) {
    // 1. Firewall Check
    if (!blockedIPRange.empty() && req.ip_in.find(blockedIPRange) == 0) {
        // Log blocked request
        logMessage("Firewall Blocked: " + req.ip_in);
        blockedCount++;
        return; 
    }

    // 2. Split Logic: Route to correct Queue
    if (req.job_type == 'P') {
        pQueue.push(req);
    } else {
        sQueue.push(req);
    }
}

// Helper: Add a server (Scaling Up)
void LoadBalancer::incWebServers(char type) {
    // Create a new server with a unique ID (current size + 1 or similar)
    if (type == 'P') {
        pServers.push_back(WebServer(nextServerID++));
    } else {
        sServers.push_back(WebServer(nextServerID++));
    }
}

// Helper: Remove a server (Scaling Down)
// Returns true if a server was actually removed (found an idle one)
bool LoadBalancer::decWebServers(char type) {
    std::vector<WebServer>* targetPool = (type == 'P') ? &pServers : &sServers;

    // Iterate through vector to find an idle server
    for (auto it = targetPool->begin(); it != targetPool->end(); ++it) {
        if (!it->isBusyStatus()) { 
            targetPool->erase(it);
            return true; 
        }
    }

    // No idle servers found, cannot scale down yet
    return false;
}

// The Main Logic Loop (Harvest -> Scale -> Assign)
void LoadBalancer::performCycle() {
    // 1. HARVEST: Check for finished requests
    auto harvest = [&](std::vector<WebServer>& pool) {
        for (auto& server : pool) {
            if (server.isBusyStatus()) {
                if (server.isRequestDone(systemTime)) {
                    requestsFinished++;
                    // Request is done. You can log it here if you want.
                    Request finishedReq = server.getRequest();
                    logMessage("Server " + std::to_string(server.getID()) + 
                            " finished request from " + finishedReq.ip_in);
                }
            }
        }
    };
    harvest(pServers);
    harvest(sServers);

    // 2. SCALE: Check if we need to resize
    if (systemTime - lastTimeChange > CYCLE_WAIT_TIME) {
        bool scaled = false;

        // Processing Scaling
        if (pQueue.size() > 25 * pServers.size()) {
            incWebServers('P');
            scaleUpCountP++;
            scaled = true;
            logMessage("[Cycle " + std::to_string(systemTime) + "] Scaled UP Processing pool to " + std::to_string(pServers.size()));
        } else if (pQueue.size() < 15 * pServers.size() && pServers.size() > 1) {
            if (decWebServers('P')) {
                scaleDownCountP++;
                scaled = true;
                logMessage("[Cycle " + std::to_string(systemTime) + "] Scaled DOWN Processing pool to " + std::to_string(pServers.size()));
            }
        }

        // Streaming Scaling
        if (sQueue.size() > 25 * sServers.size()) {
            incWebServers('S');
            scaleUpCountS++;
            scaled = true;
            logMessage("[Cycle " + std::to_string(systemTime) + "] Scaled UP Streaming pool to " + std::to_string(sServers.size()));
        } else if (sQueue.size() < 15 * sServers.size() && sServers.size() > 1) {
            if (decWebServers('S')) {
                scaleDownCountS++;
                scaled = true;
                logMessage("[Cycle " + std::to_string(systemTime) + "] Scaled DOWN Streaming pool to " + std::to_string(sServers.size()));
            }
        }

        if (scaled) lastTimeChange = systemTime;
    }

    // 3. ASSIGN: Give jobs to idle servers
    // Loop through servers again to fill any empty spots
    // Assign Processing Jobs
    for (auto& server : pServers) {
        if (server.isRequestDone(systemTime) && !pQueue.empty()) {
            Request nextReq = pQueue.front();
            pQueue.pop();
            server.startRequest(nextReq, systemTime);
        }
    }

    // Assign Streaming Jobs
    for (auto& server : sServers) {
        if (server.isRequestDone(systemTime) && !sQueue.empty()) {
            Request nextReq = sQueue.front();
            sQueue.pop();
            server.startRequest(nextReq, systemTime);
        }
    }

    // 4. Increment Clock
    systemTime++;
}

int LoadBalancer::getQueueSize() {
    return pQueue.size() + sQueue.size();
}

int LoadBalancer::getTime() {
    return systemTime;
}

void LoadBalancer::printStats() {
    logMessage("\n=== Final Simulation Statistics ===");
    logMessage("Total Time Run: " + std::to_string(systemTime));
    logMessage("Total Requests Finished: " + std::to_string(requestsFinished));
    logMessage("Total IPs Blocked: " + std::to_string(blockedCount));
    
    logMessage("\n--- Processing Pool (P) ---");
    logMessage("Servers Added:   " + std::to_string(scaleUpCountP));
    logMessage("Servers Removed: " + std::to_string(scaleDownCountP));
    logMessage("Final Servers:   " + std::to_string(pServers.size()));
    logMessage("Leftover Queue:  " + std::to_string(pQueue.size()));

    logMessage("\n--- Streaming Pool (S) ---");
    logMessage("Servers Added:   " + std::to_string(scaleUpCountS));
    logMessage("Servers Removed: " + std::to_string(scaleDownCountS));
    logMessage("Final Servers:   " + std::to_string(sServers.size()));
    logMessage("Leftover Queue:  " + std::to_string(sQueue.size()));

    logMessage("\n--- Totals ---");
    logMessage("Total Servers Added:   " + std::to_string(scaleUpCountP + scaleUpCountS));
    logMessage("Total Servers Removed: " + std::to_string(scaleDownCountP + scaleDownCountS));
    logMessage("Final Total Servers:   " + std::to_string(pServers.size() + sServers.size()));
    logMessage("Final Total Queue:     " + std::to_string(getQueueSize()));
    logMessage("===================================");
}