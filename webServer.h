#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "request.h"

class WebServer {
public:
    WebServer();
    WebServer(int id);

    void startRequest(Request req, int currTime);

    // Checks if the request is finished based on current time
    bool isRequestDone(int currTime);

    // Returns the current request
    Request getRequest();

    // Returns the server ID
    int getID();

private:
    Request currentRequest;
    int requestStartTime;
    bool isBusy;
    int serverID;
};

#endif