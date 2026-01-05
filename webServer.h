#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "request.h"

/**
 * @file webServer.h
 * @brief Declares the WebServer class that handles individual request processing.
 */
class WebServer {
public:
    /** @brief Constructs an idle web server with an ID of zero. */
    WebServer();

    /**
     * @brief Constructs an idle web server with a specific identifier.
     * @param id Unique identifier assigned to the server.
     */
    WebServer(int id);

    /**
     * @brief Assigns a request to the server and marks it as busy.
     * @param req Request to be processed.
     * @param currTime Current simulation time when the request is assigned.
     */
    void startRequest(Request req, int currTime);

    /**
     * @brief Checks whether the active request has finished processing.
     * @param currTime Current simulation time used to evaluate completion.
     * @return True if no request is active or the current request is complete; otherwise false.
     */
    bool isRequestDone(int currTime);

    /**
     * @brief Indicates whether the server is currently processing a request.
     * @return True if busy; otherwise false.
     */
    bool isBusyStatus();

    /**
     * @brief Retrieves the request currently assigned to the server.
     * @return The active Request instance.
     */
    Request getRequest();

    /**
     * @brief Returns the unique identifier for this server.
     * @return Integer representing the server ID.
     */
    int getID();

private:
    /// @brief Request currently being processed by the server.
    Request currentRequest;
    /// @brief Simulation time when the current request started processing.
    int requestStartTime;
    /// @brief Tracks whether the server is busy handling a request.
    bool isBusy;
    /// @brief Unique identifier assigned to this server instance.
    int serverID;
};

#endif