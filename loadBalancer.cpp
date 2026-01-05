#include "loadBalancer.h"
#include <iostream>

/**
 * @file loadBalancer.cpp
 * @brief Implements the LoadBalancer class to manage request queues and server scaling.
 */

/// @brief Number of cycles between consecutive scaling evaluations.
const int CYCLE_WAIT_TIME = 20;

/**
 * @brief Initializes the load balancer, server pools, and logging facilities.
 * @param initial_servers Number of servers to create initially.
 * @param blocked_ip Optional IP range prefix to block.
 */
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

/** @brief Closes the log file and writes termination information. */
LoadBalancer::~LoadBalancer() {
    if (logFile.is_open()) {
        logFile << "--- Simulation Log Ended ---" << std::endl;
        logFile.close();
    }
}

/**
 * @brief Writes a message to both console output and the log file.
 * @param message Message text to record.
 */
void LoadBalancer::logMessage(std::string message) {
    std::cout << message << std::endl;
    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
}

/**
 * @brief Applies firewall rules and enqueues the request in the appropriate queue.
 * @param req Request to evaluate and enqueue.
 */
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

/**
 * @brief Adds a server to the requested pool when scaling up.
 * @param type Pool selector ('P' for processing; otherwise streaming).
 */
void LoadBalancer::incWebServers(char type) {
    // Create a new server with a unique ID (current size + 1 or similar)
    if (type == 'P') {
        pServers.push_back(WebServer(nextServerID++));
    } else {
        sServers.push_back(WebServer(nextServerID++));
    }
}

/**
 * @brief Attempts to remove an idle server from the requested pool.
 * @param type Pool selector ('P' for processing; otherwise streaming).
 * @return True if a server was removed; otherwise false.
 */
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

/**
 * @brief Runs a single cycle of harvesting completed work, scaling pools, and assigning new jobs.
 */
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

/**
 * @brief Computes the total number of queued requests across all pools.
 * @return Combined size of processing and streaming queues.
 */
int LoadBalancer::getQueueSize() {
    return pQueue.size() + sQueue.size();
}

/**
 * @brief Reports the current simulation time.
 * @return Elapsed cycles since the start of the simulation.
 */
int LoadBalancer::getTime() {
    return systemTime;
}

/** @brief Prints final simulation statistics to the log and console. */
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