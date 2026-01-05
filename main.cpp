#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "loadBalancer.h"
#include "request.h"

// Helper function to generate a random IP address
std::string generateRandomIP() {
    return std::to_string(rand() % 255) + "." + 
           std::to_string(rand() % 255) + "." + 
           std::to_string(rand() % 255) + "." + 
           std::to_string(rand() % 255);
}

// Helper function to create a random Request
Request createRandomRequest() {
    Request req;
    req.ip_in = generateRandomIP();
    req.ip_out = generateRandomIP();
    req.time_to_process = (rand() % 500) + 10; // Random time between 10 and 510 cycles
    req.job_type = (rand() % 2 == 0) ? 'P' : 'S'; // Randomly assign Processing or Streaming
    return req;
}

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

    // 3. Pre-fill the Queue (Start with servers * 20 requests)
    std::cout << "Initializing queue with " << num_servers * 20 << " requests..." << std::endl;
    for (int i = 0; i < num_servers * 20; i++) {
        lb.addRequest(createRandomRequest());
    }

    // 4. The Main Simulation Loop
    // We run until the LoadBalancer's internal time hits the user's limit
    while (lb.getTime() < total_time) {
        
        // A. Run the Load Balancer Logic (Harvest -> Scale -> Assign)
        lb.performCycle();

        // B. Simulate New Traffic
        // Randomly add new requests during the simulation 
        if (rand() % 35 == 0) {
            lb.addRequest(createRandomRequest());
        }

        // Optional: Print status every 1000 cycles to keep log readable
        if (lb.getTime() % 1000 == 0) {
             std::cout << "Cycle " << lb.getTime() 
                       << ": Queue Size = " << lb.getQueueSize() 
                       << std::endl;
        }
    }

    std::cout << "Simulation finished at cycle " << lb.getTime() << std::endl;
    std::cout << "Final Queue Size: " << lb.getQueueSize() << std::endl;

    return 0;
}