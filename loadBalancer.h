#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include <queue>
#include <vector>
#include <fstream>
#include "webServer.h"
#include "request.h"

/**
 * @file loadBalancer.h
 * @brief Declares the LoadBalancer class that orchestrates server pools and request routing.
 */
class LoadBalancer {
public:
    /**
     * @brief Constructs a load balancer with an initial pool of servers.
     * @param initial_servers Number of servers to create across processing and streaming pools.
     * @param blocked_ip Optional IP range prefix to block via the firewall.
     */
    LoadBalancer(int initial_servers, std::string blocked_ip = "");

    /** @brief Cleans up resources and closes any open log streams. */
    ~LoadBalancer();

    /**
     * @brief Applies firewall checks and enqueues a request to the appropriate queue.
     * @param req Incoming request to consider for processing.
     */
    void addRequest(Request req);

    /**
     * @brief Executes a single simulation cycle including harvesting, scaling, and assignment.
     */
    void performCycle();

    /**
     * @brief Retrieves the combined size of processing and streaming queues.
     * @return Total number of queued requests.
     */
    int getQueueSize();

    /**
     * @brief Reports the current simulation time in cycles.
     * @return Elapsed cycles since the simulation began.
     */
    int getTime();

    /** @brief Outputs final simulation statistics to both console and log. */
    void printStats();

    /**
     * @brief Writes a message to standard output and the simulation log.
     * @param message Text to record.
     */
    void logMessage(std::string message);

private:
    /// @brief Queue holding processing requests awaiting assignment.
    std::queue<Request> pQueue;
    /// @brief Queue holding streaming requests awaiting assignment.
    std::queue<Request> sQueue;

    /// @brief Pool of processing servers available for compute-heavy jobs.
    std::vector<WebServer> pServers;
    /// @brief Pool of streaming servers available for streaming-oriented jobs.
    std::vector<WebServer> sServers;
    /// @brief Tracks the simulation time in cycles.
    int systemTime;

    /// @brief Simulation time when the server pool was last resized.
    int lastTimeChange;
    /// @brief Next available identifier to assign to a new server instance.
    int nextServerID;

    /// @brief Total number of requests that have completed processing.
    long long int requestsFinished;
    /// @brief Count of requests blocked by the firewall.
    int blockedCount;

    /// @brief Number of times processing pool scaled up.
    int scaleUpCountP;
    /// @brief Number of times processing pool scaled down.
    int scaleDownCountP;

    /// @brief Number of times streaming pool scaled up.
    int scaleUpCountS;
    /// @brief Number of times streaming pool scaled down.
    int scaleDownCountS;

    /// @brief Output stream for simulation logs.
    std::ofstream logFile;

    /**
     * @brief Adds a new web server to the specified pool.
     * @param type Pool selector ('P' for processing, other values for streaming).
     */
    void incWebServers(char type);

    /**
     * @brief Attempts to remove an idle server from the specified pool.
     * @param type Pool selector ('P' for processing, other values for streaming).
     * @return True if a server was removed; otherwise false.
     */
    bool decWebServers(char type);

    /// @brief Optional IP prefix to block during firewall checks.
    std::string blockedIPRange;
};

#endif