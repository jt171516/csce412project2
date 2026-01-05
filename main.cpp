#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "loadBalancer.h"
#include "request.h"

/**
 * @file main.cpp
 * @brief Runs the load balancer simulation by generating requests and driving cycles.
 */

// Constants for task times
const int MIN_TASK_TIME = 40;
const int MAX_TASK_TIME = 99;

/**
 * @brief Generates a random IPv4 address string.
 * @return Randomly generated dotted-quad IP address.
 */
std::string generateRandomIP() {
    return std::to_string(rand() % 255) + "." +
           std::to_string(rand() % 255) + "." +
           std::to_string(rand() % 255) + "." +
           std::to_string(rand() % 255);
}

/**
 * @brief Creates a randomized Request with timing and job type.
 * @return Request populated with random IPs, duration, and job category.
 */
Request createRandomRequest() {
    Request req;
    req.ip_in = generateRandomIP();
    req.ip_out = generateRandomIP();
    
    req.time_to_process = (rand() % (MAX_TASK_TIME - MIN_TASK_TIME + 1)) + MIN_TASK_TIME; 
    
    req.job_type = (rand() % 2 == 0) ? 'P' : 'S'; // Randomly assign Processing or Streaming
    return req;
}

/**
 * @brief Entry point for the load balancer simulation.
 * @return Zero on successful execution.
 */
int main() {
    // Seed the random number generator
    srand(time(0));

    int num_servers, total_time;
    std::string blocked_range;

    // 1. User Input
    std::cout << "--- Load Balancer Simulation ---" << std::endl;
    std::cout << "Enter number of initial servers: ";
    std::cin >> num_servers;
    std::cout << "Enter total simulation time (cycles): ";
    std::cin >> total_time;
    std::cout << "Enter IP range to block (e.g., '10.0.0.'): ";
    std::cin >> blocked_range;

    // 2. Initialize Load Balancer
    LoadBalancer lb(num_servers, blocked_range);

    lb.logMessage("\n=== Simulation Parameters ===");
    lb.logMessage("Initial Servers: " + std::to_string(num_servers));
    lb.logMessage("Starting Queue Size: " + std::to_string(num_servers * 20));
    lb.logMessage("Target Duration: " + std::to_string(total_time) + " cycles");
    lb.logMessage("Task Time Range: " + std::to_string(MIN_TASK_TIME) + " - " + std::to_string(MAX_TASK_TIME) + " cycles");
    lb.logMessage("=============================\n");

    // 3. Pre-fill the Queue (Start with servers * 20 requests)
    std::cout << "Initializing queue with " << num_servers * 20 << " requests..." << std::endl;
    for (int i = 0; i < num_servers * 20; i++) {
        lb.addRequest(createRandomRequest());
    }

    // 4. The Main Simulation Loop
    bool inBurst = false;
    int burstTicksLeft = 0;
    int cooldownTicks = 0;

    // We run until the LoadBalancer's internal time hits the user's limit
    while (lb.getTime() < total_time) {
        
        // A. Run the Load Balancer Logic (Harvest -> Scale -> Assign)
        lb.performCycle();

        // B. Simulate New Traffic
        if (cooldownTicks > 0) {
            cooldownTicks--;
        }

        if (!inBurst && cooldownTicks == 0 && (rand() % 100) < 5) { // % chance to start a burst
            inBurst = true;
            burstTicksLeft = 20 + (rand() % 20); // burst lasts 20–39 cycles
        }

        if (inBurst) {
            int newReq = 1 + (rand() % 3); // add 1-3 new requests per cycle
            for (int i = 0; i < newReq; i++) {
                lb.addRequest(createRandomRequest());
            }
            burstTicksLeft--;
            if (burstTicksLeft == 0) {
                inBurst = false;
                cooldownTicks = 400 + (rand() % 500); // valley duration
            }
        } else {
            // light trickle during valleys
            if ((rand() % 50) == 0) {
                lb.addRequest(createRandomRequest());
            }
        }

        // Print status every 1000 cycles to keep log readable
        if (lb.getTime() % 1000 == 0) {
            lb.logMessage("Cycle " + std::to_string(lb.getTime()) + 
                        ": Queue Size = " + std::to_string(lb.getQueueSize()));
        }
    }

    lb.logMessage("\nSimulation finished at cycle " + std::to_string(lb.getTime()));

    // Restate the simulation parameters explicitly
    lb.logMessage("\n=== Simulation Parameters ===");
    lb.logMessage("Initial Servers: " + std::to_string(num_servers));
    lb.logMessage("Starting Queue Size: " + std::to_string(num_servers * 20));
    lb.logMessage("Target Duration: " + std::to_string(total_time) + " cycles");
    lb.logMessage("Task Time Range: " + std::to_string(MIN_TASK_TIME) + " - " + std::to_string(MAX_TASK_TIME) + " cycles");
    lb.logMessage("=============================\n");

    lb.printStats();

    return 0;
}