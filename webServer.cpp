#include "webServer.h"

// Default constructor
WebServer::WebServer() {
    serverID = 0;
    requestStartTime = 0;
    isBusy = false;
}

// Parameterized constructor
WebServer::WebServer(int id) {
    serverID = id;
    requestStartTime = 0;
    isBusy = false;
}

// Assigns a new request to the server
void WebServer::startRequest(Request req, int currTime) {
    currentRequest = req;
    requestStartTime = currTime;
    isBusy = true;
}

// Checks if the current request has finished
bool WebServer::isRequestDone(int currTime) {
    if (isBusy) {
        return false;
    }

    // Calculate when the job should finish
    int endTime = requestStartTime + currentRequest.time_to_process;

    // If current time has reached or passed the end time
    if (currTime >= endTime) {
        isBusy = false; // automatically mark self as IDLE
        return true;
    }

    return false;
}

// Getter for the request
Request WebServer::getRequest() {
    return currentRequest;
}

// Getter for ID
int WebServer::getID() {
    return serverID;
}