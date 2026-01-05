#include "webServer.h"

/**
 * @file webServer.cpp
 * @brief Implements the WebServer class responsible for handling individual requests.
 */

/** @brief Default constructor that initializes an idle server with ID zero. */
WebServer::WebServer() {
    serverID = 0;
    requestStartTime = 0;
    isBusy = false;
}

/**
 * @brief Creates an idle server with a specific identifier.
 * @param id Unique identifier for the server instance.
 */
WebServer::WebServer(int id) {
    serverID = id;
    requestStartTime = 0;
    isBusy = false;
}

/**
 * @brief Assigns a new request to the server and records the start time.
 * @param req Request being assigned for processing.
 * @param currTime Current simulation time when the request starts.
 */
void WebServer::startRequest(Request req, int currTime) {
    currentRequest = req;
    requestStartTime = currTime;
    isBusy = true;
}

/**
 * @brief Determines whether the active request has completed processing.
 * @param currTime Current simulation time used for completion check.
 * @return True if the server is idle or the current request has finished; otherwise false.
 */
bool WebServer::isRequestDone(int currTime) {
    if (!isBusy) {
        return true;
    }

    int endTime = requestStartTime + currentRequest.time_to_process;

    if (currTime >= endTime) {
        isBusy = false; // automatically mark self as IDLE
        return true;
    }

    return false;
}

/**
 * @brief Checks whether the server is currently processing a request.
 * @return True if the server is busy; otherwise false.
 */
bool WebServer::isBusyStatus() {
    return isBusy;
}

/**
 * @brief Retrieves the request currently assigned to the server.
 * @return The active Request instance being processed.
 */
Request WebServer::getRequest() {
    return currentRequest;
}

/**
 * @brief Provides the identifier for this server.
 * @return Integer representing the server ID.
 */
int WebServer::getID() {
    return serverID;
}