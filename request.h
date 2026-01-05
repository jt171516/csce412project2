#ifndef REQUEST_H
#define REQUEST_H

#include <string>

/**
 * @file request.h
 * @brief Defines the Request struct used to represent incoming jobs.
 */
struct Request {
    /** @brief Source IP address for the incoming request. */
    std::string ip_in;
    /** @brief Destination IP address the request is targeting. */
    std::string ip_out;
    /** @brief Amount of time required for the request to complete. */
    int time_to_process;
    /** @brief Job category: Processing ('P') or Streaming ('S'). */
    char job_type;
};

#endif