#ifndef _server_h_
#define _server_h_

#include <iostream>
#include <vector>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "mythread.h"
#include "client.h"
#include <unordered_map>

#define PORT 30666

using namespace std;

class Server {

  private:
    vector<Client> clients;

    //Socket stuff
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    char buff[256];

  public:
    Server(int siteID, unordered_map<int, pair<string, string> >& address);
    void* AcceptAndDispatch();
    void * HandleClient(void *args);
    void *echoRequest();
    void *echoResponse(char* message, Client* c);
    unordered_map<int, pair<string, string> > addr;
    //vector<int> socketfd;
    //std::vector<std::unique_ptr<std::thread>> threads;
    void* addClient();
    void* acceptRequest();
  private:
    int id;
    void ListClients();
    void SendToAll(char *message);
    int FindClientIndex(int clientId, Client *c);

};

#endif
