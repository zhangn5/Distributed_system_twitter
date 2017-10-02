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

#include "mythread.h"
#include "client.h"
#include <unordered_map>

#define PORT 30666

using namespace std;

class Server {

  private:
    static vector<Client> clients;

    //Socket stuff
    int serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    char buff[256];

  public:
    Server(int siteID, unordered_map<int, pair<string, string> >& address);
    void AcceptAndDispatch();
    static void * HandleClient(void *args);
    static void *echoRequest(void *args);
    static void *echoResponse(char* message, Client* c);

  private:
    int id;
    static void ListClients();
    static void SendToAll(char *message);
    static int FindClientIndex(Client *c);
    void addClient(unordered_map<int, pair<string, string> >& address);
};

#endif
